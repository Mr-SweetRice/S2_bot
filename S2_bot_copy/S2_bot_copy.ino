#include <LineSensor.h>
#include <BLEConnection.h>
#include <TimerMicros.h>
#include <Motor.h>
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
float rpmL,rpmR;
const uint8_t PINS[8] = {39,36,34,35,32,33,25,26};
int rpms[8] = {100, 200, 300, 400, 500, 600, 700, 800};
LineSensor sensorLinha(PINS, false, 60, false);// Pinos, inverter linha branco-preto, uso do sensor 0-100%, debug
BLEConnection ble; 
Motor motorL(PWMB,BIN1,BIN2,BENCL_A,BENCL_B );
Motor motorR(PWMA,AIN1,AIN2,AENCL_A,AENCL_B );
uint16_t target=500;
TimerMicros changeRpm(6000000);
unsigned long tl=0;

void setup() {
    Serial.begin(115200);
    ble.begin();
    // sensorLinha.begin();
    // sensorLinha.calibrate();
    motorR.begin();
    motorL.begin();
}

void loop() {
    // int pos = sensorLinha.linePosition();
    ble.handleClientRequests();
    // uint16_t sensors[8] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};
    // uint8_t digital_sensors[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    if(changeRpm.pronto()){
      int i = random(0,7);
      target = rpms[i];
    }
    motorR.rpmMotor(target,1);
    motorL.rpmMotor(target,1);
    // // ble.setTelemetryData(sensors, motorL.getRpm(), 200,digital_sensors );
    ble.setPosition(target,motorL.getRpm(),motorR.getRpm());
    }
