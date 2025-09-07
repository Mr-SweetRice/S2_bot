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
LineSensor sensorLinha(PINS, true, 80, false);// Pinos, inverter linha branco-preto, uso do sensor 0-100%, debug
BLEConnection ble; 
Motor motorL(PWMB,BIN1,BIN2,BENCL_A,BENCL_B );
Motor motorR(PWMA,AIN1,AIN2,AENCL_A,AENCL_B );
uint16_t target=500;
uint64_t stop_pulses=0;
TimerMicros changeControl(8000);
TimerMicros stop(100000000);
TimerMicros tempo(30000000);
TimerMicros curvat(300000);
unsigned long lt=0;
int mtL,mtR;
int u =0;
bool lastf=0;
bool curva=0;

void setup() {
    Serial.begin(115200);
    sensorLinha.begin();
    sensorLinha.calibrate();
    ble.begin();


    motorR.begin();
    motorL.begin();
    ble.handleClientRequests();
    gP =2;
    ble.notifyPID();
}

void loop() {
    // Serial.print(gVelBase);
    // Serial.print(" | ");
    // Serial.println(gVelCurva);
    
    // stop_pulses= contarParada(motorL.getPulse(),motorR.getPulse());
    // while(!controlFlag){
    //   motorL.stop();
    //   motorR.stop();
    //   // int pos = sensorLinha.linePosition();
    //   ble.setPosition(0, pos,0);
    //   // lt = millis();
    //   // if(lastf){
    //   //   motorL.stop();
    //   //   motorR.stop();
    //   //   ble.setLineData(0, 0, stop_pulses);
    //   //   // while(1);
    //   // }
    //   stop.reiniciar();
    // }
    // // 0.7,0.00000001,0.3
    // lastf=1;
    // int pos = sensorLinha.linePosition();
    // if(labs(pos)>40){
    //   vel_base = 200;
    //    curvat.reiniciar();
    // }else if(curvat.pronto()){
    //   vel_base = 400;
    // }
    // if(changeControl.pronto()){
    //   u =control(pos,0,0.5,0.0000001,0.5,&curva);
    //   // if(curva){
    //   //   vel_base =200;
    //   //   curvat.reiniciar();
    //   //   Serial.println("Curva");
    //   // }else if(curvat.pronto()){
    //   //   vel_base =500;
    //   //   Serial.println("Reta");
    //   // }
      
    // mtL = constrain(mtL,0,820); mtR = constrain(mtR,0,820);
    // mtL= vel_base- u; mtR= vel_base  + u;
    // motorL.controlPwm(mtL, 1);
    // motorR.controlPwm(mtR, 1);
    // }
    // ble.setPosition(0, pos, 0);

    // // Serial.println(pos);
    // // Serial.print(" | ");
    // // Serial.print(mtL);
    // // Serial.print(" | ");
    // // Serial.print(mtR);
    // // Serial.print(" | ");
    // // Serial.println(u);
    // if(stop_pulses>69000){
    //     motorL.stop();
    //     motorR.stop();
    //     while(1);
    // }
    //   if(stop.pronto()){
    //     motorL.stop();
    //     motorR.stop();
    //     while(1);
    //   }

}
