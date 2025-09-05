#include <LineSensor.h>
#include <BLEConnection.h>
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
LineSensor sensorLinha(PINS, false, 60, false);// Pinos, inverter linha branco-preto, uso do sensor 0-100%, debug
BLEConnection ble; 
Motor motorL(PWMB,BIN1,BIN2,BENCL_A,BENCL_B );
Motor motorR(PWMA,AIN1,AIN2,AENCL_A,AENCL_B );

void setup() {
    Serial.begin(115200);
    ble.begin();
    // sensorLinha.begin();
    // sensorLinha.calibrate();
    motorL.begin();
}

void loop() {
    // int pos = sensorLinha.linePosition();
    ble.handleClientRequests();
    uint16_t sensors[8] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};
    uint8_t digital_sensors[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    motorL.rpmMotor(520,1);
    ble.setTelemetryData(sensors, motorL.getRpm(), 200,digital_sensors );
    ble.setPosition(520, motorL.getRpm());
}
