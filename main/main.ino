                // ------ IMPORTACIONES ------ //
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <NewPing.h>
#include <DHT.h>

                // ------ CONSTANTES/INSTANCIAS ------ //
const int DHTPin = 5;
const int EchoPin = 3;
const int TriggerPin = 4;
const int ButtonPin = 9;

                // ---------- VARIABLES ---------- //

//Sensor Temperatura/Humedad (DHT22)
struct SensorDht22{
  float humidity;
  float temp;
};

//Sensor Ultrasonico (HC-SR04)
struct SensorHcSr04{
  float rainedWater;
  const float HIGH_SENSOR = 14;
};

//Sensor Presion Atmosferica (BMP180) 
struct SensorBmp180{
  double presionAtmos;
  double altitude;
  const double PRESION_NIVEL_MAR = 1013.25;
};

struct Sensores{
  SensorDht22 Dht22;
  SensorHcSr04 HcSr04;
  SensorBmp180 Bmp180;
}

//Json
struct Json{
  StaticJsonDocument<200> jsonDocument;
  String jsonString;
};

//Objetos
#define DHTTYPE DHT22
LiquidCrystal_I2C LCD(0x27, 20, 4);
NewPing SONAR(TriggerPin, EchoPin, 200);
SFE_BMP180 BMP180;
DHT Dht(DHTPin, DHTTYPE);


void setup() {
  Serial.begin(9600);
  Dht.begin();
  BMP180.begin();  
  
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  digitalWrite(TriggerPin, LOW);

  pinMode(ButtonPin, INPUT_PULLUP);
}

                // ------ MAIN ------ //
void loop() {
  Sensores sensores;
  bool valueButton = digitalRead(ButtonPin);

  SensorDht22 dht22 = readDHT22();
  SensorHcSr04 hc_sr04 = getRainedWater();
  SensorBmp180 bmp180 = getAltitudeAndPresionAtmos();
  sensores.Dht22 = dht22;
  sensores.HcSr04 = hc_sr04;
  sensores.Bmp180 = bmp180;
  
  Json jsonData = createJson(sensores);

  if(valueButton == HIGH){
    showDisplayLcd(sensores);
    valueButton = false;
    delay(5000);
    LCD.clear();
  }
  LCD.noBacklight();
}

                // ------ FUNCIONES PERSONALIZADAS ------ //

//Obtiene la humedad y temperatura. Estos valores son obtenidos atraves del sensor DHT22
SensorDht22 readDHT22(){
  SensorDht22 sensorDht22;

  sensorDht22.humidity = Dht.readHumidity();
  sensorDht22.temp = dht.readTemperature();

  if (isnan(sensorDht22.humidity) || isnan( sensorDht22.temp)) {
    sensorDht22.humidity = -1;
     sensorDht22.temp = -1;
  }
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
  // jsonDocument["date"] = date;

  serializeJson(jsonData.jsonDocument, jsonData.jsonString);
  return jsonData;
}

//En esta funcion se utilizaria el ESP8266 para enviar los datos al servidor
void sendData(){
}
