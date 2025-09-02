#include <LineSensor.h>
const uint8_t PINS[8] = {36,39,34,35,32,33,25,26};
LineSensor sensorLinha(PINS, false, 80, true);// Pinos, inverter linha branco-preto, uso do sensor 0-100%, debug
void setup(){
  Serial.begin(115200);
  sensorLinha.begin();
  sensorLinha.calibrate();
}

void loop(){
  //Serial.println(sensorLinha.position());
  sensorLinha.linePosition();
}