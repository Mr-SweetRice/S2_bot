#include <LineSensor.h>
#include <BLEConnection.h>
#include <TimerMicros.h>
#include <Motor.h>
#include <SHControl.h>
#include <Preferences.h>
#include <Armazenamento.h>

Armazenamento mem;
const int PWMA = 13;
const int AIN1 = 14;
const int AIN2 = 27;
const int PWMB = 23;
const int BIN1 = 21;
const int BIN2 = 22;
const int BENCL_A = 18;
const int BENCL_B = 5;
const int AENCL_A = 17;
const int AENCL_B = 16;
const int LIMIAR = 100;  // limaiara de curva
const uint8_t PINS[8] = { 36, 39, 34, 35, 32, 33, 25, 26 };

LineSensor sensorLinha(PINS, true, 80, false);  // Pinos, inverter linha branco-preto, uso do sensor 0-100%, debug
BLEConnection ble;
Motor motorL(PWMB, BIN1, BIN2, BENCL_A, BENCL_B);
Motor motorR(PWMA, AIN1, AIN2, AENCL_A, AENCL_B);
uint16_t target = 500;
uint64_t stop_pulses = 0;
TimerMicros changeControl(8000);
TimerMicros stop(100000000);
TimerMicros tempo(30000000);
TimerMicros curvat(300000);
unsigned long lt = 0;
int mtL, mtR;
int u = 0;
bool lastf = 0;
int vel_base = 200;
bool curva = 0;
float kp, kd, ki;
float old_kp, old_ki, old_kd;

void setup() {
  Serial.begin(115200); //Inicia Serial
  ble.begin(); //Inicia ble
  
  //Le os dados na memoria de KP, KI, KD
  kp = mem.lerValor("kp"); gP = kp;
  ki = mem.lerValor("ki"); gI = ki;
  kd = mem.lerValor("kd"); gD = kd;
  //Inicia Sensor de linha e calibra por 4s
  sensorLinha.begin();
  sensorLinha.calibrate();
  motorR.begin();
  motorL.begin();
}

void loop() {
  ble.handleClientRequests();
  kp = gP; if(kp != old_kp){mem.salvarValor("kp", kp)old_kp = kp;}
  ki = gI; if(ki != old_ki){mem.salvarValor("ki", ki)old_ki = kd;}
  kd = gD; if(kd != old_kd){mem.salvarValor("kd", kd)old_kd = ki;}
  int pos = sensorLinha.linePosition();
  if (changeControl.pronto()) {
    u = control(pos, 0, kp, ki, kd);
    mtL = constrain(mtL, 0, 820);
    mtR = constrain(mtR, 0, 820);
    mtL = vel_base - u;
    mtR = vel_base + u;
    motorL.controlPwm(mtL, 1);
    motorR.controlPwm(mtR, 1);
  }
  ble.setPosition(0, pos, 0);
  //Paradas
  while (!controlFlag) {
    motorL.stop();
    motorR.stop();
    int pos = sensorLinha.linePosition();
    ble.setPosition(0, pos, 0);
    stop.reiniciar();
    ble.handleClientRequests();
    kp = gP;
    ki = gI;
    kd = gD;
  }
  stop_pulses = contarParada(motorL.getPulse(), motorR.getPulse());
  if (stop_pulses > 69000) {
    motorL.stop();
    motorR.stop();
    while (!controlFlag)ble.handleClientRequests();
  }
  if (stop.pronto()) {
    motorL.stop();
    motorR.stop();
    while (!controlFlag)ble.handleClientRequests();
  }
}
