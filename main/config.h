//Config WiFi
#define WIFI_SSID "TuSSID"
#define WIFI_PASSWORD "TuClave"
#define SERVER_URL "http://localhost:8080/api/upload"


//Credenciales
#define API_USERNAME "UsuarioAPI"
#define API_PASSWORD "ClaveAPI"

                // ------ CONSTANTES ------ //

#define DHTTYPE DHT22
const int DHTPin = 5;
const int EchoPin = 3;
const int TriggerPin = 4;
const int ButtonPin = 9;
const unsigned long INTERVAL_HOUR = 3600000;
const unsigned long LCD_DISPLAY_DURATION = 5000;



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
    const float HIGH_SENSOR = 14;
};

// Sensor Presion Atmosferica (BMP180)
struct SensorBmp180
{
    double presionAtmos;
    double altitude;
    const double PRESION_NIVEL_MAR = 1013.25;
};

struct Sensores
{
    SensorDht22 Dht22;
    SensorHcSr04 HcSr04;
    SensorBmp180 Bmp180;
}

// Json
struct Json
{
    StaticJsonDocument<50> jsonDocument;
    String jsonString;
};