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

const int ALTURA_SENSOR_AGUA = 14;
const double PRESION_NIVEL_MAR = 1013.25;

#define DHTTYPE DHT22
LiquidCrystal_I2C lcd(0x27, 20, 4);
NewPing sonar(TriggerPin, EchoPin, 200);
SFE_BMP180 bmp180;
DHT dht(DHTPin, DHTTYPE);

                // ------ DEFINICION ------ //
void setup() {
  Serial.begin(9600);
  
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  digitalWrite(TriggerPin, LOW);

  pinMode(ButtonPin, INPUT_PULLUP);

  dht.begin();

  bmp180.begin();  
}

                // ---------- VARIABLES ---------- //

//Sensor Temperatura/Humedad (DHT22)
float humidity;
float temp;

//Sensor Ultrasonico (HC-SR04)
float distanceCm;
float highSensor;

//Sensor Presion Atmosferica (BMP180) 
double presionAtmos;
double altitude;

//Json
StaticJsonDocument<200> jsonDocument;
String jsonString;

                // ------ MAIN ------ //
void loop() {
  bool valueButton = digitalRead(ButtonPin);

  if(valueButton == HIGH){
    
    readDHT22(humidity, temp);
    getRainedWater(distanceCm);
    getAltitudeAndPresionAtmos(presionAtmos, altitude);
    createJson(jsonDocument, jsonString, humidity, temp, presionAtmos, distanceCm, altitude);
    showDisplayLcd(lcd, distanceCm, presionAtmos);
    
    valueButton = false;
    delay(5000);
    lcd.clear();
  }
  lcd.noBacklight();
}

                // ------ FUNCIONES PERSONALIZADAS ------ //

//Obtiene la humedad y temperatura. Estos valores son obtenidos atraves del sensor DHT22
void readDHT22(float& humidity, float& temp){
  humidity = dht.readHumidity();
  temp = dht.readTemperature();

  if (isnan(humidity) || isnan(temp)) {
    humidity = -1;
    temp = -1;
  }
}

//Calcula la cantidad de agua llovida. Esto se logra utilizando el sensor HC-SR04
void getRainedWater(float& distanceCm) {

  int dist = sonar.ping_cm();
  distanceCm = ALTURA_SENSOR_AGUA - dist;
  
}
//Obtiene la Presion Atmosferica y calcula la altitud
void getAltitudeAndPresionAtmos(double& presion, double& altitude){

  char status;
  double T;
  
  status = bmp180.startTemperature(); //Inicio de lectura de temperatura
  if (status != 0)
  {   
    delay(status); //Pausa para que finalice la lectura
    status = bmp180.getTemperature(T); //Obtener la temperatura
    if (status != 0)
    {
      status = bmp180.startPressure(3); //Inicio lectura de presión
      if (status != 0)
      {        
        delay(status); //Pausa para que finalice la lectura        
        status = bmp180.getPressure(presion, T); //Obtener la presión
        if (status != 0)
        {                  
          altitude = bmp180.altitude(presion, PRESION_NIVEL_MAR); //Calcular altura    
        }      
      }      
    }   
  } 
}

//Muestra la informacion procesada en el display LCD
void showDisplayLcd(LiquidCrystal_I2C lcd, float distanceCm, double presion){
  lcd.init();
  lcd.clear();
  lcd.backlight();
  
  // ------ TEMPERATURA - HUMEDAD ------ //
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print("%");

  lcd.setCursor(0, 2);
  lcd.print("Presion: ");
  lcd.print(presion);
  lcd.print(" Mbar");

  lcd.setCursor(0, 3);
  lcd.print("Agua: ");
  lcd.print(distanceCm);
  lcd.print(" cm");
}

//Esta funcion sera utilizada para obtener la fecha
void getDate(){
}

void createJson(StaticJsonDocument<200>& jsonDocument, String& jsonString, float hum, float temp, double pre_atmos, float rained_water, float altitude){
  
  jsonDocument["temperature"] = temp;
  jsonDocument["humidity"] = hum;
  jsonDocument["presion"] = pre_atmos;
  jsonDocument["waterRained"] = rained_water;
  jsonDocument["altitud"] = altitude;
  // jsonDocument["date"] = date;

  serializeJson(jsonDocument, jsonString);
}

//En esta funcion se utilizaria el ESP8266 para enviar los datos al servidor
void sendData(){
}
