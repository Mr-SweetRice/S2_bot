int control(uint16_t linePosition, uint16_t target,int p, int i,int d){
    unsigned long now = millis();
    error =target - linePosition; 
    unsigned long dt = (now - last_time);
    integral = integral + error*dt; 
    derivative = (error-last_error)/dt;
    u = (p*error*dt + i*integral + d*derivative)+u;
    last_time = now;
    last_error = error;
    integral = constrain(integral,-200000, 200000);
    
    return u;
}
