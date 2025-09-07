#define MAX_SEG 100
TimerMicros SEG(300000);
enum Tipo { RETA,
            CURVA };

Tipo trajeto[MAX_SEG];
int segCount = 0;

long lastR = 0, lastL = 0;

// --- Função para detectar segmento e salvar ---
void detectaSegmento(long encA, long encB) {
  if (SEG.pronto()) {
    encA = labs(encA);
    encB = labs(encB);
    long diffA = encA - lastR;
    long diffB = encB - lastL;
    lastR = encA;
    lastL = encB;
    long diff = diffA - diffB;
    Tipo atual;
    if (labs(diff) < LIMIAR) {
      atual = RETA;
      Serial.println("Reta");
    } else {
      atual = CURVA;
      Serial.println("Curva");
    }
    // Salva se mudou de tipo ou se é o primeiro
    if (segCount == 0 || trajeto[segCount - 1] != atual) {
      if (segCount < MAX_SEG) {
        trajeto[segCount++] = atual;
      }
    }
  }
}

// --- Para visualizar ---
void imprimeTrajeto() {
  Tipo atual;
  for (int i = 0; i < segCount; i++) {
    switch (trajeto[i]) {
      case RETA: Serial.println("RETA "); break;
      case CURVA: Serial.println("CURVA"); break;
    }
  }
}