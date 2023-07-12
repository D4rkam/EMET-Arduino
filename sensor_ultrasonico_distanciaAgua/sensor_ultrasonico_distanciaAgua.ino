// Pines utilizados
const int EchoPin = 5;
const int TriggerPin = 6;

void setup() {
  Serial.begin(9600); //Inicializacion del monitor
  // Inicializacion de los pines
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);
}

void loop() {
  int distanciaCm = ping(TriggerPin, EchoPin); // Funcion encargada de obtener la distancia
  Serial.print("Distancia: ");
  Serial.println(distanciaCm);
  delay(1000);
}

int ping(int TriggerPin, int EchoPin) {
  long tiempo, distanciaCm;
  
  digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  
  tiempo = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
  
  distanciaCm = tiempo * 10 / 292 / 2;   //convertimos a distancia, en cm
  return distanciaCm;
}