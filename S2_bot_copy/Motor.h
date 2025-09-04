#pragma once
#include <ESP32Encoder.h>

// --- ENCODERS ---


class Motor{
  public: 
    int pulseC = 287;          // pulsos por volta
    uint16_t amostragem_ms =10;
    uint8_t STBY = 2;

  private:
    uint8_t _PWMPin, _IN1, _IN2, _STBY, _encA, _encB;
    ESP32Encoder _enc;
    long long _lastCount =0;
    long _lastMs=0;
    float _rpm =0;
    float _kpm = 2.0f;  
    float _kim = 1.0f;             
    float _eintegral = 0;                
    int   _upwm = 0;   
    float _vFilt =0;
    float _vPrev=0;


  public:
  Motor(uint8_t pwmPin,
        uint8_t in1Pin,
        uint8_t in2Pin,
        uint8_t encA,
        uint8_t encB)
  : _PWMPin(pwmPin), _IN1(in1Pin), _IN2(in2Pin), _encA(encA), _encB(encB) {}

  void begin() {
    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    _enc.attachHalfQuad(_encA, _encB);
    _enc.clearCount();

    pinMode(_IN1, OUTPUT);
    pinMode(_IN2, OUTPUT);
    pinMode(STBY, OUTPUT);
    digitalWrite(STBY, HIGH);
    ledcAttach(_PWMPin, 20000, 10);
    stop();
  }
  void controlPwm(int pwm, bool dir) {
    digitalWrite( _IN1, dir);
    digitalWrite(_IN2, !dir);
    ledcWrite(_PWMPin, pwm);
  }
  void stop() {
    analogWrite(_PWMPin, 0);
  }

  float getRpm(){
    // if (abs(_enc.getCount()) > 8) {
    //   long c =abs( _enc.getCount());
    //   long currT = micros();
    //   long deltaT = ((float)(currT - _lastMs ) * 1.0e6);
    //   deltaT = deltaT / c;
    //   _rpm = (1 / (deltaT * pulseC)) * 60;
    //   _lastMs  = currT;
    //   _enc.clearCount();
    // }
    unsigned long now = millis();
    if (now - _lastMs >= amostragem_ms) {
      long c = _enc.getCount();
      //Serial.println(c);
      long dCounts = c - _lastCount;
      _lastCount = c;

      float dt = (now - _lastMs) / 1000.0f;  // tempo real em segundos
      _lastMs = now;

      float rpmRaw = ((float)llabs(dCounts) / pulseC) * (60.0f / dt);

      _vFilt = 0.854* _vFilt + 0.0728* rpmRaw + 0.0728* _vPrev; //a+b ≈ 1
      _vPrev = rpmRaw;

      _rpm = _vFilt;  // guarda RPM filtrado
      Serial.print(_rpm); Serial.print("|"); Serial.println((float)llabs(c) / pulseC);
    }
    return _rpm;
  }

  void rpmMotor(int vel, bool dir){
    float rpm = getRpm();
    float e = vel-_vFilt; 
    //Serial.print(vel);
    //Serial.print(" | ");
    float dt = (float)amostragem_ms / 1000.0f;
    _eintegral = _eintegral + e*dt; 
    _upwm = (_kpm*e + _kim*_eintegral);
    if(_upwm>700)_upwm=700;if(_upwm<0)_upwm=0;
    //Serial.print(rpm);
    //Serial.print(" | ");
    ///Serial.println(_upwm);
    controlPwm(_upwm,dir);

  }
};