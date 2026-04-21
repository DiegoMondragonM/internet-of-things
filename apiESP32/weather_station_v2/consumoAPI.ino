#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ── Configuración WiFi ──────────────────────────────────────────────────────
const char* ssid     = "INFINITUMC200";
const char* password = "devtonics";

// ── Servidor web en puerto 80 ───────────────────────────────────────────────
WebServer server(80);

// ── Variables globales ──────────────────────────────────────────────────────
float latitude  = 18.37;
float longitude = -97.37;
const int ledPin = 14;
int ledState = LOW;

// ── Página HTML embebida ────────────────────────────────────────────────────
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Clima ESP32</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 20px;
      line-height: 1.5;
    }

    h1, h2 {
      margin-bottom: 10px;
    }

    .seccion {
      margin-bottom: 25px;
      padding-bottom: 10px;
      border-bottom: 1px solid #ccc;
    }

    input {
      padding: 6px;
      margin-top: 4px;
      margin-bottom: 10px;
      width: 180px;
    }

    button {
      padding: 8px 14px;
      border: 1px solid #999;
      background: #f2f2f2;
      cursor: pointer;
    }

    button:hover {
      background: #e6e6e6;
    }

    pre {
      background: #f7f7f7;
      padding: 10px;
      border: 1px solid #ccc;
      overflow-x: auto;
    }
  </style>
</head>
<body>

  <h1>Clima con ESP32</h1>

  <div class="seccion">
    <h2>Temperatura actual</h2>
    <p id="tempValue">-- °C</p>
    <p id="statusMsg">Cargando...</p>
  </div>

  <div class="seccion">
    <h2>Cambiar coordenadas</h2>

    <label for="latInput">Latitud</label><br>
    <input type="text" id="latInput" value="18.37"><br>

    <label for="lonInput">Longitud</label><br>
    <input type="text" id="lonInput" value="-97.37"><br>

    <button onclick="setCoords()">Consultar</button>
  </div>

  <div class="seccion">
    <h2>Estado del LED</h2>
    <p id="ledLabel">--</p>
  </div>

  <div class="seccion">
    <h2>Respuesta JSON</h2>
    <pre id="jsonData">Esperando datos...</pre>
  </div>

  <script>
    function updateWeather() {
      fetch('/weather')
        .then(response => response.json())
        .then(data => {
          document.getElementById('tempValue').innerText = data.temperatura + ' °C';
          document.getElementById('statusMsg').innerText =
            'Actualizado: ' + new Date().toLocaleTimeString();

          const esNoche = data.is_day === 0;
          document.getElementById('ledLabel').innerText =
            esNoche ? 'Encendido (noche)' : 'Apagado (día)';

          document.getElementById('jsonData').innerText =
            JSON.stringify(data.raw, null, 2);
        })
        .catch(err => {
          document.getElementById('statusMsg').innerText = 'Error al obtener datos';
          console.error(err);
        });
    }

    function setCoords() {
      const lat = document.getElementById('latInput').value;
      const lon = document.getElementById('lonInput').value;

      fetch('/setcoords?lat=' + lat + '&lon=' + lon)
        .then(() => updateWeather())
        .catch(err => console.error('Error setcoords:', err));
    }

    updateWeather();
    setInterval(updateWeather, 10000);
  </script>

</body>
</html>
)rawliteral";
// ── Handler GET / ────────────────────────────────────────────────────────────
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// ── Handler GET /weather ─────────────────────────────────────────────────────
// Regresa JSON: { "temperatura": 22.5, "is_day": 1, "raw": { ...open-meteo... } }
void handleWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    server.send(503, "text/plain", "WiFi desconectado");
    return;
  }

  HTTPClient http;
  String url = "http://api.open-meteo.com/v1/forecast"
               "?latitude="  + String(latitude,  4) +
               "&longitude=" + String(longitude, 4) +
               "&current_weather=true";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float temperatura = doc["current_weather"]["temperature"];
      int   is_day      = doc["current_weather"]["is_day"];

      // Controlar LED físico: noche → encendido
      ledState = (is_day == 0) ? HIGH : LOW;
      digitalWrite(ledPin, ledState);

      // Armar respuesta JSON para el browser
      StaticJsonDocument<1024> resp;
      resp["temperatura"] = temperatura;
      resp["is_day"]      = is_day;
      resp["raw"]         = doc["current_weather"];

      String respStr;
      serializeJson(resp, respStr);

      http.end();
      server.send(200, "application/json", respStr);
      return;
    }
  }

  http.end();
  server.send(500, "text/plain", "ERROR al obtener clima");
}

// ── Handler GET /setcoords ───────────────────────────────────────────────────
// Recibe ?lat=XX.XX&lon=YY.YY y actualiza las variables globales
void handleSetCoords() {
  if (server.hasArg("lat") && server.hasArg("lon")) {
    latitude  = server.arg("lat").toFloat();
    longitude = server.arg("lon").toFloat();
    Serial.printf("Coordenadas actualizadas: lat=%.4f, lon=%.4f\n", latitude, longitude);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Faltan parametros lat y lon");
  }
}

// ── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.printf("Conectando a %s", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  server.on("/",          handleRoot);
  server.on("/weather",   handleWeather);
  server.on("/setcoords", handleSetCoords);

  server.begin();
  Serial.println("Servidor HTTP iniciado en puerto 80");
}

// ── Loop ─────────────────────────────────────────────────────────────────────
void loop() {
  server.handleClient();
}
