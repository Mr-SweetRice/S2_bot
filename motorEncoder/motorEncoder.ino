#include <ESP32Encoder.h>

// --- ENCODERS ---
ESP32Encoder encL, encR;
// ajuste conforme sua roda: perímetro(mm)/contagens
float pulseDist = 0.350104;
long alvoCounts;

// --- TB6612FNG ---
const int PWMA = 13;
const int AIN1 = 14;
const int AIN2 = 27;

const int PWMB = 23;
const int BIN1 = 21;
const int BIN2 = 22;

const int STBY = 2;

// --- MOTOR CONTROL ---
void motorA(int pwm, bool fwd) {
  digitalWrite(AIN1, fwd);
  digitalWrite(AIN2, !fwd);
  analogWrite(PWMA, pwm);
}
void motorB(int pwm, bool fwd) {
  digitalWrite(BIN1, fwd);
  digitalWrite(BIN2, !fwd);
  analogWrite(PWMB, pwm);
}
void motorsStop() {
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
}

void setup() {
  Serial.begin(115200);

  // encoders
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  encL.attachHalfQuad(16, 17);
  encR.attachHalfQuad(18, 5);
  encL.clearCount();
  encR.clearCount();

  // driver
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);

  // PWM setup (20 kHz, 10 bits = 0–1023)
  analogWriteFrequency(PWMA, 20000);
  analogWriteResolution(PWMA, 10);
  analogWriteFrequency(PWMB, 20000);
  analogWriteResolution(PWMB, 10);

  // alvo em contagens (1 m = 1000 mm)
  alvoCounts = (long)(2000.0 / pulseDist);
  Serial.printf("Alvo: %ld counts\n", alvoCounts);
}

void loop() {
  // zera contagem
  encL.clearCount();
  encR.clearCount();

  // inicia frente
  motorA(700, true); // duty 600/1023
  motorB(700, false);
  
  // roda até alcançar alvo
  while (false) {
    long cL = abs((long)encL.getCount());
    long cR = abs((long)encR.getCount());
    long media = (cL + cR) / 2;

    if (millis() % 200 < 10) {
      Serial.printf("cL=%ld cR=%ld media=%ld alvo=%ld\n", cL, cR, media, alvoCounts);
    }

    if (media >= alvoCounts) break;
    delay(5);
  }
  delay(3000);
  // parar
  motorsStop();
  Serial.println("Percorreu 1m, parado.");
  delay(3000);
}
