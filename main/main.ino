                // ------ IMPORTACIONES ------ //
#include<LiquidCrystal_I2C.h>
#include<DHT.h>

                // ------ CONSTANTES/INSTANCIAS ------ //
const int DHTPin = 5;
const int BTNPin = 2;
const int EchoPin = 3;
const int TriggerPin = 4;

#define DHTTYPE DHT22
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPin, DHTTYPE);


                // ------ DEFINICION ------ //
void setup() {
  pinMode(BTNPin, INPUT_PULLUP);
  
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  digitalWrite(TriggerPin, LOW);

  lcd.init();
  lcd.backlight();

  dht.begin();

  Serial.begin(9600);
}

                // ---------- VARIABLES ---------- //

//Display LCD
int displayMode = 0;
int btnState = HIGH;
int lastBtnState = HIGH;

//Sensor DHT22
float humidity;
float temp;

//Sensor Ultrasonico (HC-SR04)
float distanceCm;
float highSensor;

                // ------ MAIN ------ //
void loop() {
  readDHT22(humidity, temp);
  getRainedWater(TriggerPin, EchoPin, distanceCm, highSensor);
  showDisplayLcd(displayMode, btnState, lastBtnState, lcd);
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
void getRainedWater(int TriggerPin, int EchoPin, float& distanceCm, float& highSensor) {
  long time;
  digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  
  time = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
  
  distanceCm = (time * 0.000001 / 34300.0 / 2.0) - highSensor;   //convertimos a distancia, en cm
}

//Muestra la informacion procesada en el display LCD
void showDisplayLcd(int& displayMode, int& btnState, int& lastBtnState, LiquidCrystal_I2C lcd){
  btnState = digitalRead(BTNPin);
  if (btnState == LOW && lastBtnState == HIGH){
    displayMode = (displayMode + 1) % 3;
    
    lcd.clear();
    switch (displayMode){

      // ------ TEMPERATURA - HUMEDAD ------ //
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temp);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("Hum: ");
        lcd.print(humidity);
        lcd.print("%");
        break;
      
      // ------ ¿PLUVIOMETRO? ------ //
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Distan: ");
        lcd.print(distanceCm);
        lcd.print(" cm");
        lcd.setCursor(0, 1);
        lcd.print("Altura: ");
        lcd.print(highSensor);
        lcd.print(" cm");
        break;
    }
  }
  Serial.print("Display Mode: ");
  Serial.println(displayMode);
  lastBtnState = btnState;
}
