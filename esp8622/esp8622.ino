#include "config.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(ESP.getChipId()); //El ESP.getChipId() retorna la identificacion unica del chip
}

void loop(){
  if(Serial.available() > 0){
    String data = Serial.readStringUntil("[JSON]");
    sendData(data);
  }
}

//En esta funcion se utilizaria el ESP8266 para enviar los datos al servidor
void sendData(String jsonData){
  
  if (WiFi.status() != WL_CONNECTED) {
    //saveData(); -> Esta funcion guardaria los datos en una SD
    Serial.print(jsonData);
    Serial.write("[JSON SD]");
    return;
  }

  HTTPClient http;
  WiFiClient client;
  http.begin(client, SERVER_URL);

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0) {
    Serial.print("Respuesta del servidor: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error en la solicitud. Código de error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}