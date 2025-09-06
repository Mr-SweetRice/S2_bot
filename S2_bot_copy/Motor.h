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
    long _lastMs,_lastMs2 =0;
    float _rpm =0;
    float _kpm = 0.005;  
    float _kim = 0.000003;             
    float _eintegral = 0;                
    int   _upwm = 0;   
    float _vFilt =0;
    float _vPrev=0;
    unsigned long _now,_now2 = 0;
    long _c,_dCounts=0;
    float _dt,_rpmRaw =0;
    float _rpm2,_e2,_dt2=0;
    long _pulses;


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
    pwm = constrain(pwm,0,820);
    digitalWrite(_IN1, dir);
    digitalWrite(_IN2, !dir);
    ledcWrite(_PWMPin, pwm);
  }
  void stop() {
    ledcWrite(_PWMPin, 0);
  }

  
  float getRpm(){
    _now = millis();
    // if (abs(_enc.getCount()) > 8) {
    //   long c =abs( _enc.getCount());
    //   long currT = micros();
    //   long deltaT = ((float)(currT - _lastMs ) * 1.0e6);
    //   deltaT = deltaT / c;
    //   _rpm = (1 / (deltaT * pulseC)) * 60;
    //   _lastMs  = currT;
    //   _enc.clearCount();
    // }
    if (_now - _lastMs >= amostragem_ms) {
      _c = _enc.getCount();
      //Serial.println(c);
      _dCounts = _c - _lastCount;
      _lastCount = _c;
      _dt = (_now - _lastMs) / 1000.0f;  // tempo real em segundos
      _lastMs = _now;
      _rpmRaw = ((float)llabs(_dCounts) / pulseC) * (60.0f / _dt);

      _vFilt = 0.854* _vFilt + 0.0728* _rpmRaw + 0.0728* _vPrev; //a+b ≈ 1
      _vPrev = _rpmRaw;
      _rpm = _vFilt;  // guarda RPM filtrado
    }
    return _rpm;
  }


  
  void rpmMotor(int vel, bool dir){ 
    _now2 = millis();
    _kpm =gP;_kim=gI;
    _rpm2 = getRpm();
    _e2 = vel-_vFilt; 
    _dt2 = (_now2 - _lastMs2);
    // Serial.print(_dt2);
    _eintegral = _eintegral + _e2*_dt2; 
    _upwm = (_kpm*_e2*_dt2 + _kim*_eintegral)+_upwm;
    _lastMs2 = _now2;
    _eintegral = constrain(_eintegral,-200000, 200000);
    if(_upwm>800)_upwm=800;if(_upwm<0)_upwm=0;
    controlPwm(_upwm ,dir);

    // Serial.print(" | ");
    // Serial.print((_kpm*e*dt + _kim*_eintegral));
    // Serial.print(" | ");
    // Serial.print(e);
    // Serial.print(" | ");
    // Serial.print(_upwm);
    // Serial.print(" | ");
    // Serial.print(_eintegral);
    // Serial.print(" | ");
    // Serial.println(rpm);


  }
 long getPulse(){
   _pulses= _enc.getCount();
   return _pulses;
  }

  void printPwm(){
    Serial.println(_upwm);
  }
};