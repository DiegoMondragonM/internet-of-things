const int LED_PIN = 14; // Pin donde está conectado el LED

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hola, ESP32"); // Configurar el pin como salida
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // Encender el LED
  delay(500);                 // Esperar 1 segundo
  digitalWrite(LED_PIN, LOW);  // Apagar el LED
  delay(500);                 // Esperar 1 segundo
}
