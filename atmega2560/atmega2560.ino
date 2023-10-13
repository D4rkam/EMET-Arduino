                // ------ IMPORTACIONES ------ //
#include "config.h"

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <SFE_BMP180.h>
#include <NewPing.h>
#include <DHT.h>

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

// Sensor Temperatura/Humedad (DHT22)
struct SensorDht22
{
    float humidity;
    float temp;
};

// Sensor Ultrasonico (HC-SR04)
struct SensorHcSr04
{
    float rainedWater;
    float HIGH_SENSOR = 14;
};

// Sensor Presion Atmosferica (BMP180)
struct SensorBmp180
{
    double presionAtmos;
    double altitude;
    double PRESION_NIVEL_MAR = 1013.25;
};

struct Sensores
{
    SensorDht22 Dht22;
    SensorHcSr04 HcSr04;
    SensorBmp180 Bmp180;
};

struct Json
{
    StaticJsonDocument<10> jsonDocument;
    String jsonString;
};

void setup() {
  Serial.begin(115200);
  Dht.begin();
  BMP180.begin();  

  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  digitalWrite(TriggerPin, LOW);

  pinMode(ButtonPin, INPUT_PULLUP);
}

                // ------ MAIN ------ //
void loop() {
  unsigned long currentMillis = millis();
  Sensores sensores;

  if (Serial.available() > 0){
    //String data = Serial.readStringUntil("[JSON SD]");
    //saveData(data);
  } 

  if (currentMillis - previousMillis >= INTERVAL_HOUR) {
    previousMillis = currentMillis;
    sensores = initSensores();
    Json jsonData = createJson(sensores);
    
    if (jsonData.jsonString.length() == 0) {
      return;
    } 
    else if (Serial.availableWrite()) {
      Serial.print(jsonData.jsonString);
      Serial.write("[JSON]")
      }
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
  sensorDht22.temp = isnan(sensorDht22.temp) ? -1 : Dht.readTemperature();
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
          sensorBmp180.altitude = BMP180.altitude(sensorBmp180.presionAtmos, sensorBmp180.PRESION_NIVEL_MAR); //Calcular altura    
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

Sensores initSensores(){
  Sensores sensores;

  sensores.Dht22 = readDHT22();
  sensores.HcSr04 = getRainedWater();
  sensores.Bmp180 = getAltitudeAndPresionAtmos();
  return sensores;
}