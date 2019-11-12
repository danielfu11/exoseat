// modified from https://wiki.dfrobot.com/12V_DC_Motor_251rpm_w_Encoder__SKU__FIT0186_
//The sample code for driving one way motor encoder
const byte encoder0pinA = 2;//A pin -> the interrupt pin 0
const byte encoder0pinB = 4;//B pin -> the digital pin 4
byte encoderALast;
int duration;//the number of the pulses
boolean Direction;//the rotation direction


void setup()
{
  Serial.begin(57600);//Initialize the serial port
  EncoderInit();//Initialize the module
}

void loop()
{
  Serial.print("Pulse:");
  if (!Direction)
  {
    Serial.print("-");
  }
  Serial.println(duration);
  duration = 0; // reset duration every time the loop iterates (the higher the FREQUENCY of the square wave, the higher the number of pulses per loop iteration) -> change to timer for higher accuracy
  delay(100);
}

void EncoderInit()
{
  Direction = true;//default -> Forward (when going forward, encoder A should be 90 degrees BEHIND encoder B)
  pinMode(encoder0pinB,INPUT);
  attachInterrupt(0, wheelSpeed, CHANGE); // set interrupt to trigger at every edge of encoder0pinA
}

void wheelSpeed() // only need one encoder to determine speed, 2 encoders to determine direction
{
  int encoderAState = digitalRead(encoder0pinA); // read encoder A
  int encoderBState = digitalRead(encoder0pinB); // read encoder B

  // determine direction:
  if((encoderALast == LOW) && encoderAState==HIGH) // if encoder A was low before and is high now (aka if there was a rising edge of encoder A)
  {
    if(encoderBState == LOW && Direction) // if encoder B is low and the direction is set as forward
    {
      Direction = false; //Reverse
    }
    else if(encoderBState == HIGH && !Direction) // if encoder B is high and direction is set as reverse
    {
      Direction = true;  //Forward
    }
  }
  encoderALast = encoderAState;

  duration++;
}
