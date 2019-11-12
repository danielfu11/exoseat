 #define SPEED_ZERO 0
 #define SPEED_FOURTH 63.75
 #define SPEED_HALF 127.5
 #define SPEED_FULL 255
 #define SHAFT_RADIUS 0.0209//inches
 
// Left Wheel
const int pinON = 6;         // ON/OFF switch, active HIGH
//left wheel
const int pinCW_Left = 7;    // clock-wise PMOS gate
const int pinCC_Left = 8;    // counter-clock-wise PMOS gate
const int pinSpeed_Left = 9; // speed reference

//Right wheel
const int pinCW_Right = 11;    // clock-wise PMOS gate
const int pinCC_Right = 12;    // counter-clock-wise PMOS gate
const int pinSpeed_Right = 10; // speed reference

// global counter variables
volatile int enc_count_Left = 0; 
volatile int enc_count_Right = 0; 
float encoderPulses = 0;


// setup pins and initial values
void setup() {
  pinMode(pinON,INPUT);
  pinMode(13,OUTPUT);             // on-board LED
  digitalWrite(13,LOW);           // turn LED off
    
  //left wheel
  pinMode(pinCW_Left,OUTPUT);
  pinMode(pinCC_Left,OUTPUT);
  pinMode(pinSpeed_Left,OUTPUT);
  digitalWrite(pinCW_Left,LOW);   // stop clockwise
  digitalWrite(pinCC_Left,LOW);   // stop counter-clockwise
  analogWrite(pinSpeed_Left,0); // set speed reference
  
  //right wheel
  pinMode(pinCW_Right,OUTPUT);
  pinMode(pinCC_Right,OUTPUT);
  pinMode(pinSpeed_Right,OUTPUT);
  digitalWrite(pinCW_Right,LOW);   // stop clockwise
  digitalWrite(pinCC_Right,LOW);   // stop counter-clockwise
  analogWrite(pinSpeed_Right,0); // set speed reference
  
  //Connect encoder outputs to external interrupt pins via a 1k resistor
  attachInterrupt(0, count_Left, RISING); //ISR: count_Left
  attachInterrupt(1, count_Right, RISING); //ISR: count_Right

  Serial.begin(9600);
}

///ISRs///
void count_Left(){
  enc_count_Left++;
}

void count_Right(){
  enc_count_Right++;
}

///FUNCTION DEFINITIONS///
void forward(float Dist_inch, uint8_t Speed){
  
  encoderPulses = Dist_inch/SHAFT_RADIUS;  //(.53mm = .0209in) Find total number of encoder pulses to travel the distance, Dist_inch 
  
  analogWrite(pinSpeed_Left,Speed);        //set wheel speeds
  analogWrite(pinSpeed_Right,Speed);
  digitalWrite(pinCW_Left, LOW);           //START: turn on wheels so that they rotate in the same direction
  digitalWrite(pinCC_Left, HIGH);
  digitalWrite(pinCC_Right, LOW);
  digitalWrite(pinCW_Right, HIGH);

  while (enc_count_Left < encoderPulses);  //wait for the encoder count to be >= the caluclated encoder pulse count

  digitalWrite(pinCW_Left, LOW);           //STOP: turn off all wheels and directions
  digitalWrite(pinCC_Left, LOW);
  digitalWrite(pinCC_Right, LOW);
  digitalWrite(pinCW_Right, LOW);

  enc_count_Left = 0;                      //reset encoder count to 0
}

void reverse(float Dist_inch, uint8_t Speed){
  
  encoderPulses = Dist_inch/SHAFT_RADIUS; //(.53mm = .0209in) 
    
  analogWrite(pinSpeed_Left,Speed);
  analogWrite(pinSpeed_Right,Speed);
  digitalWrite(pinCC_Left, LOW);
  digitalWrite(pinCW_Left, HIGH);
  digitalWrite(pinCW_Right, LOW);
  digitalWrite(pinCC_Right, HIGH);

  while (enc_count_Left < encoderPulses);
  
  digitalWrite(pinCW_Left, LOW);
  digitalWrite(pinCC_Left, LOW);
  digitalWrite(pinCC_Right, LOW);
  digitalWrite(pinCW_Right, LOW);

  enc_count_Left = 0;
}

void turn_left(float Dist_inch, uint8_t Speed){

  encoderPulses = Dist_inch/SHAFT_RADIUS; //.53mm = .0209in
  
  analogWrite(pinSpeed_Left,Speed);
  analogWrite(pinSpeed_Right,Speed);
  digitalWrite(pinCW_Left, LOW);
  digitalWrite(pinCC_Left, HIGH);
  digitalWrite(pinCW_Right, LOW);
  digitalWrite(pinCC_Right, HIGH);

  while (enc_count_Left < encoderPulses);
  
  digitalWrite(pinCW_Left, LOW);
  digitalWrite(pinCC_Left, LOW);
  digitalWrite(pinCC_Right, LOW);
  digitalWrite(pinCW_Right, LOW);

  enc_count_Left = 0;
}

void turn_right(float Dist_inch, uint8_t Speed){

  encoderPulses = Dist_inch/SHAFT_RADIUS; //.53mm = .0209in
 
  analogWrite(pinSpeed_Left,Speed);
  analogWrite(pinSpeed_Right,Speed);
  digitalWrite(pinCC_Left, LOW);
  digitalWrite(pinCW_Left, HIGH);
  digitalWrite(pinCC_Right, LOW);
  digitalWrite(pinCW_Right, HIGH);

  while (enc_count_Left < encoderPulses);
  
  digitalWrite(pinCW_Left, LOW);
  digitalWrite(pinCC_Left, LOW);
  digitalWrite(pinCC_Right, LOW);
  digitalWrite(pinCW_Right, LOW);

  enc_count_Left = 0;
}

uint8_t loop_count = 0;

void loop() {
  Serial.print("timer 6: ");
  Serial.println(digitalPinToTimer(6));
  Serial.print("timer 5: ");
  Serial.println(digitalPinToTimer(5));
  Serial.println(TIMER1A);
  delay(5000);
  if(digitalRead(pinON) == HIGH){
    delay(1000);
    while(loop_count < 1){
   
      reverse (22, SPEED_FOURTH); //2 feet forward
      turn_right(15.6, SPEED_FOURTH);//CW turn
      reverse (22, SPEED_FOURTH); //2 feet forward
     // delay(500);
      turn_left(15.7, SPEED_FOURTH);//CCW turn
      delay(500);
      //turn_right(15.7, SPEED_FOURTH);//CCW turn
      

      /* Figure 8 motion
      reverse(36, SPEED_FOURTH);
      turn_left(6.5, SPEED_FOURTH);
      reverse(72, SPEED_FOURTH);
      turn_right(7, SPEED_FOURTH);
      reverse(34.5, SPEED_FOURTH);
      turn_right(9, SPEED_FOURTH);
      reverse(36.5, SPEED_FOURTH);
      turn_right(7, SPEED_FOURTH);
      reverse(72, SPEED_FOURTH);
      turn_left(7, SPEED_FOURTH);
      reverse(36, SPEED_FOURTH);
      turn_left(8, SPEED_FOURTH);
      */
  
      //print encoder values to serial terminal
      Serial.print("LEFT: ");
      Serial.println(enc_count_Left);
      Serial.print("RIGHT: ");
      Serial.println(enc_count_Right);
      Serial.print("DIFF: ");
      Serial.println(enc_count_Left-enc_count_Right);
      Serial.println(" ");  
      
      loop_count++;
    }
  }
}
