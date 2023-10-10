                // ------ IMPORTACIONES ------ //
#include "config.h"

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <SFE_BMP180.h>
#include <NewPing.h>
#include <DHT.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

                // ------ INSTANCIAS ------ //
LiquidCrystal_I2C LCD(0x27, 20, 4);
NewPing SONAR(TriggerPin, EchoPin, 200);
SFE_BMP180 BMP180;
DHT Dht(DHTPin, DHTTYPE);

                // ---------- VARIABLES ---------- //

unsigned long previousMillis = 0;
bool lcdActive = false;
unsigned long lcdActivationTime = 0;

void setup() {
  Serial.begin(115200);
  Dht.begin();
  BMP180.begin();  

  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  digitalWrite(TriggerPin, LOW);

  pinMode(ButtonPin, INPUT_PULLUP);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(ESP.getChipId()); //El ESP.getChipId() retorna la identificacion unica del chip

}

                // ------ MAIN ------ //
void loop() {
  unsigned long currentMillis = millis();
  Sensores sensores;

  if (currentMillis - previousMillis >= INTERVAL_HOUR) {
    previousMillis = currentMillis;
    sensores = initSensores();
    Json jsonData = createJson(sensores);
    
    if (jsonData.jsonString.length() == 0) {
      Serial.println("Error al crear JSON. Datos no válidos.");
    } 
    else {
      sendData(jsonData.jsonString);
    }
  }

  bool valueButton = digitalRead(ButtonPin);
  if (valueButton == HIGH) {
    lcdActive = true;
    lcdActivationTime = currentMillis;
    // Muestra el LCD
    showDisplayLcd(sensores);
    valueButton = false;
  }

  // Verifica si se debe apagar el LCD después de cierto tiempo
  if (lcdActive && (currentMillis - lcdActivationTime >= LCD_DISPLAY_DURATION)) {
    LCD.clear();
    lcdActive = false;
  }
}

                // ------ FUNCIONES PERSONALIZADAS ------ //

//Obtiene la humedad y temperatura. Estos valores son obtenidos atraves del sensor DHT22
SensorDht22 readDHT22(){
  SensorDht22 sensorDht22;

  sensorDht22.humidity = isnan(sensorDht22.humidity) ? -1 : Dht.readHumidity();
  sensorDht22.temp = isnan(sensorDht22.temp) ? -1 : dht.readTemperature();
  return sensorDht22;
}

//Calcula la cantidad de agua llovida. Esto se logra utilizando el sensor HC-SR04
SensorHcSr04 getRainedWater() {
  SensorHcSr04 sensorHcSr04;
  int dist = SONAR.ping_cm();
  sensorHcSr04.rainedWater = sensorHcSr04.HIGH_SENSOR - dist;

  return sensorHcSr04;
  
}
//Obtiene la Presion Atmosferica y calcula la altitud
SensorBmp180 getAltitudeAndPresionAtmos(){
  SensorBmp180 sensorBmp180;
  char status;
  double T;
  
  status = BMP180.startTemperature(); //Inicio de lectura de temperatura
  if (status != 0)
  {   
    delay(status); //Pausa para que finalice la lectura
    status = BMP180.getTemperature(T); //Obtener la temperatura
    if (status != 0)
    {
      status = BMP180.startPressure(3); //Inicio lectura de presión
      if (status != 0)
      {        
        delay(status); //Pausa para que finalice la lectura        
        status = BMP180.getPressure(sensorBmp180.presionAtmos, T); //Obtener la presión
        if (status != 0)
        {                  
          sensorBmp180.altitude = BMP180.altitude(sensorBmp180.presionAtmos, SensorBmp180.PRESION_NIVEL_MAR); //Calcular altura    
        }      
      }      
    }   
  }
  return sensorBmp180;
}

//Muestra la informacion procesada en el display LCD
void showDisplayLcd(Sensores sensores){
  LCD.init();
  LCD.clear();
  LCD.backlight();
  
  // ------ TEMPERATURA - HUMEDAD ------ //
  LCD.setCursor(0, 0);
  LCD.print("Temp: ");
  LCD.print(sensores.Dht22.temp);
  LCD.print("C");

  LCD.setCursor(0, 1);
  LCD.print("Hum: ");
  LCD.print(sensores.Dht22.humidity);
  LCD.print("%");

  LCD.setCursor(0, 2);
  LCD.print("Presion: ");
  LCD.print(sensores.Bmp180.presionAtmos);
  LCD.print(" Mbar");

  LCD.setCursor(0, 3);
  LCD.print("Agua: ");
  LCD.print(sensores.HcSr04.rainedWater);
  LCD.print(" cm");
}

//Esta funcion sera utilizada para obtener la fecha
void getDate(){
}

Json createJson(Sensores sensores){
  Json jsonData;
  jsonData.jsonDocument["temperature"] = sensores.Dht22.temp;
  jsonData.jsonDocument["humidity"] = sensores.Dht22.humidity;
  jsonData.jsonDocument["precipitacion"] = sensores.HcSr04.rainedWater;
  jsonData.jsonDocument["presion"] = sensores.Bmp180.presionAtmos;
  jsonData.jsonDocument["altitud"] = sensores.Bmp180.altitude;
  jsonData.jsonDocument["date"] = "2023-10-12T12:00:00";
  //jsonData.jsonDocument["locacion"] = ESP.getChipId();

  serializeJson(jsonData.jsonDocument, jsonData.jsonString);
  return jsonData;
}

//En esta funcion se utilizaria el ESP8266 para enviar los datos al servidor
void sendData(String jsonData){
  
  if (WiFi.status() != WL_CONNECTED) {
    //saveData(); -> Esta funcion guardaria los datos en una SD
    Serial.println("No esta conectado a WiFi");
  }

  HTTPClient http;
  http.begin(SERVER_URL);

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

Sensores initSensores(){
  Sensores sensores;

  SensorDht22 dht22 = readDHT22();
  SensorHcSr04 hc_sr04 = getRainedWater();
  SensorBmp180 bmp180 = getAltitudeAndPresionAtmos();
  sensores.Dht22 = dht22;
  sensores.HcSr04 = hc_sr04;
  sensores.Bmp180 = bmp180;
  return sensores;
}