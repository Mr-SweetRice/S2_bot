#define VELOCIDADE_BASE 200
#define ERRO_CURV 10

int p =0;
int i =0;
int d =0;
unsigned long last_time =0;
int error,last_error=0;
float integral=0;
float derivative=0;
uint16_t pos =0;

int control(int16_t linePosition, uint16_t target,float p, float i,float d,bool* curva){
    unsigned long now = millis();
    error =target-linePosition; 
    unsigned long dt = (now - last_time);
    integral = integral+ error*dt; 
    derivative = (abs(error)-abs(last_error));
    if( labs(derivative)>ERRO_CURV){*curva=1;}else{*curva=0;}
    integral = constrain(integral,-20000000, 20000000);
    int u =round((p*error*dt + i*integral + d*derivative));
    last_time = now;
    last_error = error;
    // Serial.print(p);
    // Serial.print(" | ");
    // Serial.print(linePosition);
    // Serial.print(" | ");
    // Serial.print(dt);
    // Serial.print(" | ");
    // Serial.println(error);
    // Serial.print(" | ");
    // Serial.print((p*error*dt + i*integral + d*derivative));
    // Serial.print(" | ");
    // Serial.println(u);
    
    
    return u;
}

long contarParada(long pulseL,long pulseR){
  long media = (labs(pulseL) +labs(pulseR))/2;
  
  return media;

}
