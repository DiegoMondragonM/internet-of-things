#define LED_PIN  14// Pin donde está conectado el LED
#define boton 32
bool estado;
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hola, ESP32"); // Configurar el pin como salida
  pinMode(LED_PIN, OUTPUT);
  pinMode(boton, INPUT_PULLUP);
  estado=LOW;
}

void loop() {
  bool estadoboton=digitalRead(boton);
  if(estadoboton==LOW)
  {
    estado=!estado;
    digitalWrite(LED_PIN, estado);
    delay(400);
  }                 // Esperar 1 segundo
}

