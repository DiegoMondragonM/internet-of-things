#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "INFINITUMC300";
const char* password = "4bChf4aFcP";

WebServer server(80);
const int LED_PIN = 2;

void handleRoot() {
  bool ledState = digitalRead(LED_PIN);

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>ESP32 LED</title>";
  html += "<style>";
  html += "body { font-family: sans-serif; text-align: center; padding: 40px; }";
  html += "a { display: inline-block; padding: 16px 32px; margin: 10px;";
  html += "border-radius: 8px; text-decoration: none; font-size: 20px; color: white; }";
  html += ".on { background: #1D9E75; }";
  html += ".off { background: #E24B4A; }";
  html += "</style></head><body>";
  html += "<h2>Control LED - ESP32</h2>";
  html += "<p>Estado: <strong>";
  html += ledState ? "ENCENDIDO" : "APAGADO";
  html += "</strong></p>";
  html += "<a href='/on' class='on'>Encender</a>";
  html += "<a href='/off' class='off'>Apagar</a>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ---- Ruta /on ----
void handleOn() {
  digitalWrite(LED_PIN, HIGH);
  // Redirige de vuelta al inicio para ver el estado actualizado
  //server.sendHeader("Location", "/");
  server.send(200, "text/html", "Led encendido<br><a href=\"/\">Volver</a>");
}

// ---- Ruta /off ----
void handleOff() {
  digitalWrite(LED_PIN, LOW);
  //server.sendHeader("Location", "/");
  server.send(200, "text/html", "Led encendido<br><a href=\"/\">Volver</a>");
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(115200);
  delay(400);

  WiFi.begin(ssid, password);
  
  Serial.print("#");
  Serial.print("##");
  Serial.print("#####");
  Serial.print("######.Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  Serial.print("IP del server: ");
  Serial.println(WiFi.localIP()); 

  // Registrar cada ruta con su función
  server.on("/",    handleRoot);
  server.on("/on",  handleOn);
  server.on("/off", handleOff);

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();
}