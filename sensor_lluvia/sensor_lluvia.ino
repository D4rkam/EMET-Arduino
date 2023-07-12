void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
}

void loop(){
  Serial.println(analogRead(A0)); //Se lee la entrada analogica A0
  delay(100);
}

