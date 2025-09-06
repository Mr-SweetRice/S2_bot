#include <LineSensor.h>
#include <BLEConnection.h>
#include <TimerMicros.h>
#include <Motor.h>
#include <SHControl.h>
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

const uint8_t PINS[8] = {36,39,34,35,32,33,25,26};
// int rpms[8] = {100, 200, 300, 400, 500, 600, 700, 800};
// uint16_t sensors[8] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};
// uint8_t digital_sensors[8] = {1, 0, 1, 0, 1, 0, 1, 0};
LineSensor sensorLinha(PINS, true, 80, true);// Pinos, inverter linha branco-preto, uso do sensor 0-100%, debug
BLEConnection ble; 
Motor motorL(PWMB,BIN1,BIN2,BENCL_A,BENCL_B );
Motor motorR(PWMA,AIN1,AIN2,AENCL_A,AENCL_B );
uint16_t target=500;
TimerMicros changeControl(8000);
TimerMicros stop(45000000);
unsigned long lt=0;
int mtL,mtR;
int u =0;
bool lastf=0;

void setup() {
    Serial.begin(115200);
    sensorLinha.begin();
    sensorLinha.calibrate();
    ble.begin();

    motorR.begin();
    motorL.begin();
}

void loop() {
    ble.handleClientRequests();
    while(!controlFlag){
      motorL.stop();
      motorR.stop();
      uint64_t stop_pulses= contarParada(motorL.getPulse(),motorR.getPulse());
      int pos = sensorLinha.linePosition();
      ble.setPosition(0, pos,u );
      lt = millis();
      if(lastf){
        motorL.stop();
        motorR.stop();
        ble.setLineData(0, 0, stop_pulses);
        while(1);
      }
      // stop.reiniciar();
    }
    // 0.7,0.00000001,0.3
    int pos = sensorLinha.linePosition();
    if(changeControl.pronto()){
      u =control(pos,0,gP,gI,gD);
      Serial.println(u);
      mtL= 500 - u;mtR= 500 + u;
      mtL = constrain(mtL,0,820); mtR = constrain(mtR,0,820);

    };
    // ble.setPosition(0, pos, u);
    motorL.controlPwm(mtL, 1);
    motorR.controlPwm(mtR, 1);
    // Serial.println(pos);
    // Serial.print(" | ");
    // Serial.print(mtL);
    // Serial.print(" | ");
    // Serial.print(mtR);
    // Serial.print(" | ");
    // Serial.println(u);
    

    // if(changeRpm.pronto()){
    //   int i = random(0,7);
    //   target = rpms[i];
    // }
    // // ble.setTelemetryData(sensors, motorL.getRpm(), 200,digital_sensors );
    // if(stop.pronto()){
    //   motorL.stop();
    //   motorR.stop();
    //   while(1);
    // }
    }
