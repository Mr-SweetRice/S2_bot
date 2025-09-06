#pragma once
#include <Arduino.h>
#include "TimerMicros.h"

class LineSensor {
public:
  static constexpr uint8_t  kCount   = 8;
  static constexpr uint16_t kAdcMax  = 3000;
  static constexpr uint32_t kCalibUs = 4000000; // 4 s

private:
  uint8_t  _pins[kCount];
  uint16_t _threshold;
  bool     _invert;
  bool     _debug;
  TimerMicros _calibT;
  uint16_t _min[kCount];
  uint16_t _raw[kCount];
  bool     _dig[kCount];
  int _normalized[kCount];

public:
  LineSensor(const uint8_t pins[kCount], bool invert=false, uint16_t threshold=60, bool debug=false)
  : _threshold(threshold), _invert(invert), _debug(debug), _calibT(kCalibUs) {
    for (uint8_t i = 0; i < kCount; i++) _pins[i] = pins[i];
    for (uint8_t i = 0; i < kCount; i++) { _min[i] = kAdcMax; _raw[i] = 0; _dig[i] = false; }
  }

  void begin() {
    for (uint8_t i = 0; i < kCount; i++) pinMode(_pins[i], INPUT);
  }

  void setThreshold(uint16_t t) { _threshold = t; }
  void setInvert(bool inv)      { _invert = inv;  }
  void enableDebug(bool on)     { _debug = on;    }

  void calibrate(uint32_t dur_us = kCalibUs) {
    _calibT.setIntervalo(dur_us);
    if (_debug) { Serial.println("Iniciando calibracao..."); delay(1000); }
    for (uint8_t i = 0; i < kCount; i++) _min[i] = kAdcMax;
    _calibT.reiniciar();
    while (!_calibT.pronto()) {
      for (uint8_t i = 0; i < kCount; i++) {
        uint16_t v = analogRead(_pins[i]);
        if (v < _min[i]) _min[i] = v;
        if (_debug) { Serial.print(" | "); Serial.print(v); }
      }
      if (_debug) Serial.println();
      yield(); // útil em ESP
    }
    if (_debug) {
      Serial.println("Minimos calibrados:");
      for (uint8_t i = 0; i < kCount; i++) { Serial.print(" | "); Serial.print(_min[i]); }
      Serial.println();
    }
  }

  void read() {
    for (uint8_t i = 0; i < kCount; i++) {
      _raw[i] = analogRead(_pins[i]);
      bool d = (_raw[i] > _threshold);
      if (_invert) d = !d;
      _dig[i] = d;
      if (_debug) { Serial.print(" | "); Serial.print(_dig[i]); }
    }
    //if (_debug) { Serial.print(" |  Posicao: "); Serial.println(position()); }
  }

float linePosition() {
  LineSensor::normalized();
  float num = 0, den = 0;
  bool seguir =0;
  for (int i = 0; i < kCount; i++) {
    num += _normalized[i] * i;
    den += _normalized[i];
  }
  // for (int i = 0; i < kCount; i++) {
  //   if(_normalized[i] >90){
  //     seguir =1;
  //   }else{break;}
  //   if(_normalized[i] <10){
  //     seguir =1;
  //   }else{break;}
  // }
  // if(seguir)return 0;
  if (den < 1e-3) return 0; // nenhum sensor ativo

  // posição média no índice (0 à esquerda, 7 à direita)
  float idx = num / den; // 0..7

  // centraliza em -100..+100
  // idx=0 => -100 , idx=3.5 => 0 , idx=7 => +100
  float pos = ((idx - 3.5f) / 3.5f) * 100.0f;
  if (pos < -100) pos = -100;
  if (pos >  100) pos =  100;
  return pos;
}


 void normalized() {
    for (int i = 0; i < 8; i++) {
        uint16_t minU = _min[i]+((kAdcMax-_min[i])*(100-_threshold)/100);
        uint16_t v = analogRead(_pins[i]);
        _normalized[i] = map(v, minU, kAdcMax, 100, 0);
        _normalized[i] = constrain(_normalized[i], 0, 100);
       if (_debug){Serial.print(" | "); Serial.print(_normalized[i]);}

    }
    if (_debug)Serial.println("");
  }

  uint16_t raw(uint8_t i) const    { return (i < kCount) ? _raw[i] : 0; }
  bool     digital(uint8_t i) const{ return (i < kCount) ? _dig[i] : false; }
  uint16_t minCalib(uint8_t i) const{ return (i < kCount) ? _min[i] : 0; }
};
