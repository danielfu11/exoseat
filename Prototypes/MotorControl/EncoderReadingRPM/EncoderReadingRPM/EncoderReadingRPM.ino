#include <PID_v1.h>

int encoderPin1 = 2;
int encoderPin2 = 3;

volatile int lastEncoded = 0;
volatile long oldencodervalue=0, encoderValue = 0;
volatile float numberofdpulses;
long lastencoderValue = 0;
int time;
float rev=0, lastmillis=0;
float degree=0, olddegree=0;
int MSB = 0;
int LSB = 0;
float rpm = 0;

// CONTROLS PARAMETERS
double rpm_normalized = 0;
double max_rpm = 20.2;
double pwm = 0;
double setPointRpm;
double setPoint_normalized;
double Kp = 0.006;
double Ki = .046;
double Kd = 0;

//Specify the links and initial tuning parameters

PID PidController(&rpm_normalized, &pwm, &setPoint_normalized, Kp, Ki, Kd, DIRECT);

void setup() {
  Serial.begin (9600);
  pinMode(encoderPin1, INPUT_PULLUP); 
  pinMode(encoderPin2, INPUT_PULLUP);
  pinMode(6,OUTPUT);
  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);

  setPointRpm = 15; 
  setPoint_normalized = (setPointRpm/max_rpm)*255;
  
  PidController.SetMode(AUTOMATIC);
}

void loop(){ 
  //Do stuff here
  if(encoderValue!=oldencodervalue){
    delay(1000);
    detachInterrupt(0);
    detachInterrupt(1); 
    time = millis() - lastmillis;
    numberofdpulses=encoderValue/4;  
    degree=numberofdpulses*(0.17); //360/2096
    rev = (degree-olddegree)/360;
    rpm = (rev/time)*60000; 
    rpm_normalized = (rpm/max_rpm) * 255;    
    Serial.print("Degree ");
    Serial.println(numberofdpulses); 
    Serial.print("RPM ");
    Serial.println(rpm); 
    Serial.print("NORMALIZED RPM ");
    Serial.println(rpm_normalized);
    oldencodervalue=encoderValue;
    lastmillis = millis();
    olddegree=degree;
    rev=0;
    attachInterrupt(0, updateEncoder, CHANGE); 
    attachInterrupt(1, updateEncoder, CHANGE);    
    //delay(1000); //just here to slow down the output, and show it will work  even during a delay

    Serial.print("CONTROLLER OUTPUT ");
    Serial.println(pwm);
  }
  //Serial.println(pwm); 
  PidController.Compute();
  analogWrite(6,pwm);        //set wheel speeds
  //analogWrite(6,128);
}


void updateEncoder(){
  MSB = digitalRead(encoderPin1); //MSB = most significant bit
  LSB = digitalRead(encoderPin2); //LSB = least significant bit
//  Serial.print("MSB: ");Serial.print(MSB); Serial.print("LSB: ");Serial.print(LSB);
//  Serial.println();
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
  lastEncoded = encoded; //store this value for next time
}
