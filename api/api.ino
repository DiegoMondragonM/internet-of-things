#include <esp_wifi.h>
#include <webServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


const char* ssid ="ALUMNOS TEC";
const char* password ="";

webServer server(80);

const char* htmlpage = R"rawliteral()rawliteral";

void handleWeather(){
  if(WiFi.status() == WL_CONNECTTED){
   HTTPClient http;
   String url = "https://api.open-mateo.com-/v1/forecast?latitude=19.04?longitude=-98.20&currrent-weather=true";
   http.begin(url);
   int httpCode = http.GET();

    if(httpCode > 0){
      String payload = http.getString();
      
    }
  }
  server.send(200,"text/plain","ERROR");
}
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
