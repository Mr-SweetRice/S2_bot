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

const uint8_t PINS[8] = {39,36,34,35,32,33,25,26};
// int rpms[8] = {100, 200, 300, 400, 500, 600, 700, 800};
// uint16_t sensors[8] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};
// uint8_t digital_sensors[8] = {1, 0, 1, 0, 1, 0, 1, 0};
LineSensor sensorLinha(PINS, true, 80, true);// Pinos, inverter linha branco-preto, uso do sensor 0-100%, debug
BLEConnection ble; 
Motor motorL(PWMB,BIN1,BIN2,BENCL_A,BENCL_B );
Motor motorR(PWMA,AIN1,AIN2,AENCL_A,AENCL_B );
uint16_t target=500;
TimerMicros changeControl(15000);
unsigned long tl=0;
int mtL,mtR;

void setup() {
    Serial.begin(115200);
    sensorLinha.begin();
    sensorLinha.calibrate();
    ble.begin();

    motorR.begin();
    motorL.begin();
}

void loop() {
    // ble.handleClientRequests();
    int pos = sensorLinha.linePosition();
    if(changeControl.pronto()){
      int u =control(pos,0,gP,gI,gD);
      mtL= 200 - u;mtR= 200 + u;
      mtL = constrain(mtL,0,800); mtR = constrain(mtR,0,800);

    };
    ble.setPosition(0, pos, 0);
    motorL.rpmMotor(mtL, 1);
    motorR.rpmMotor(mtR, 1);

    // ble.setPosition(0, pos, 0);
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
    }
