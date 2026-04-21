#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ── Configuración WiFi ──────────────────────────────────────────────────────
const char* ssid     = "INFINITUMC200";  
const char* password = "devtonics";            

// ── Servidor web en puerto 80 ───────────────────────────────────────────────
WebServer server(80);

float latitude=18.37;
float longitude=-97.37;
String Json="";
const int ledPin=14;
int ledstate=LOW;
// ── Página HTML embebida ────────────────────────────────────────────────────
// El contenido viene del archivo index.html convertido a string raw
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Clima con ESP32</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: Arial, sans-serif;
      background: #1a1a2e;
      color: #eee;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .card {
      background: #16213e;
      border-radius: 16px;
      padding: 40px 48px;
      text-align: center;
      box-shadow: 0 8px 32px rgba(0,0,0,0.4);
      min-width: 300px;
    }
    h1 {
      font-size: 18px;
      color: #a8b2d8;
      margin-bottom: 8px;
      letter-spacing: 1px;
      text-transform: uppercase;
    }
    #tempValue {
      font-size: 72px;
      font-weight: bold;
      color: #e94560;
      margin: 16px 0 4px;
    }
    #statusMsg {
      font-size: 13px;
      color: #555;
      margin-top: 12px;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>Temperatura actual en Puebla</h1>
    <p id="tempValue">--</p>
    <p id="statusMsg">Cargando...</p>
  </div>

  <script>
    function updateWeather() {
      fetch('/weather')
        .then(response => response.text())
        .then(data => {
          document.getElementById('tempValue').innerText = data + ' °C';
          document.getElementById('statusMsg').innerText =
            'Actualizado: ' + new Date().toLocaleTimeString();
        })
        .catch(err => {
          document.getElementById('statusMsg').innerText = 'Error al obtener datos';
          console.error(err);
        });
    }

    // Llamada inicial y luego cada 10 segundos
    updateWeather();
    setInterval(updateWeather, 10000);
  </script>
</body>
</html>
)rawliteral";

// ── Handler GET / ───────────────────────────────────────────────────────────
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}


// ── Handler GET /weather ────────────────────────────────────────────────────
void handleWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    server.send(503, "text/plain", "WiFi desconectado");
    return;
  }

  HTTPClient http;

  // API open-meteo: temperatura actual en Puebla (lat 19.04, lon -98.20)
  String url = "http://api.open-meteo.com/v1/forecast"
               "?latitude="+String(latitude,2)+
               "&longitude="+String(longitude,2)+
               "&current_weather=true";
               //String(current_weather ? "true":"false");


  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    // Parsear JSON con ArduinoJson
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float temperatura = doc["current_weather"]["temperature"];
      int is_Day=doc["current_weather"]["is_day"];
      http.end();
      server.send(200, "text/plain", String(temperatura, 1));
      return;
    }
  }

  http.end();
  server.send(500, "text/plain", "ERROR");
}

// ── Setup ───────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Conectar al WiFi
  Serial.printf("Conectando a WIFI", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  // Registrar rutas del servidor
  server.on("/",        handleRoot);
  server.on("/weather", handleWeather);

  server.begin();
  Serial.println("Servidor HTTP iniciado en puerto 80");
}

// ── Loop ────────────────────────────────────────────────────────────────────
void loop() {
  server.handleClient();
}
