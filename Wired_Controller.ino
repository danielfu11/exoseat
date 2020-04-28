const    int btn_pressed_pin = 8;   // Button Pressed (Used for Voice Control Interrupts)
const    int btn1_pin        = 2;   // Button 1 Input Pin
const    int btn1_out        = 12;  // Button 1 Output Pin
const    int btn2_pin        = 3;   // Button 2 Input Pin
const    int btn2_out        = 11;  // Button 2 Output Pin
const    int btn3_pin        = 4;   // Button 3 Input Pin
const    int btn3_out        = 10;  // Button 3 Output Pin
const    int vc1_pin         = 5;   // Voice Control 1 Input Pin
const    int vc2_pin         = 6;   // Voice Control 2 Input Pin
const    int vc3_pin         = 7;   // Voice Control 3 Input Pin

const    int state1_out_byte = 0x53;// Output Byte for Button 1 and Voice Control Command 1
const    int state2_out_byte = 0x42;// Output Byte for Button 2 and Voice Control Command 2
const    int state3_out_byte = 0x50;// Output Byte for Button 3 and Voice Control Command 3

volatile int btn1_state      = 0;   // Button 1 State
volatile int btn2_state      = 0;   // Button 2 State
volatile int btn3_state      = 0;   // Button 3 State
volatile int vc1_state       = 0;   // Voice Control Command 1 State
volatile int vc2_state       = 0;   // Voice Control Command 2 State
volatile int vc3_state       = 0;   // Voice Control Command 3 State


void setup()
{
  // Button 1
  pinMode(btn1_pin, INPUT);
  pinMode(btn1_out, OUTPUT);
  // Button 2
  pinMode(btn2_pin, INPUT);
  pinMode(btn2_out, OUTPUT);
  // Button 3
  pinMode(btn3_pin, INPUT);
  pinMode(btn3_out, OUTPUT);

  // Voice Control Interrupt Signal
  pinMode(btn_pressed_pin, OUTPUT);
  // Voice Control Input 1
  pinMode(vc1_pin, INPUT);
  // Voice Control Input 2
  pinMode(vc2_pin, INPUT);
  // Voice Control Input 3
  pinMode(vc3_pin, INPUT);

  Serial.begin(9600);
}

void loop()
{
  // Read Button 1 State
  btn1_state = digitalRead(btn1_pin);
  // Read Button 2 State
  btn2_state = digitalRead(btn2_pin);
  // Read Button 3 State
  btn3_state = digitalRead(btn3_pin);

  // Read Voice Control Command 1 State
  vc1_state = digitalRead(vc1_pin);
  // Read Voice Control Command 2 State
  vc2_state = digitalRead(vc2_pin);
  // Read Voice Control Command 3 State
  vc3_state = digitalRead(vc3_pin);

  // TODO: Button Debounce
  // TODO: Priorities
  

  // Button 1 Output
  if (btn1_state == HIGH)
  {
    digitalWrite(btn1_out, HIGH);
    digitalWrite(btn_pressed_pin, HIGH);
    Serial.write(state1_out_byte);
    //Serial.println("Button 1: Stop Pressed");
    delay(200);
  }
  else if (vc1_state == HIGH)
  {
    digitalWrite(btn1_out, HIGH);
    Serial.write(state1_out_byte);
    //Serial.println("VC 1: Stop said");
    delay(300);
  }
  else
  {
    digitalWrite(btn1_out, LOW);
    digitalWrite(btn_pressed_pin, LOW);
  }

  // Button 2 Output
  if (btn2_state == HIGH)
  {
    digitalWrite(btn2_out, HIGH);
    digitalWrite(btn_pressed_pin, HIGH);
    Serial.write(state2_out_byte);
    //Serial.println("Button 2: Bring Me Down Pressed");
    delay(200);
  }
  else if (vc2_state == HIGH)
  {
    digitalWrite(btn2_out, HIGH);
    Serial.write(state2_out_byte);
    //Serial.println("VC 2: Bring me Down said");
    delay(300);
  }
  else
  {
    digitalWrite(btn2_out, LOW);
    digitalWrite(btn_pressed_pin, LOW);
  }

  // Button 3 Output
  if (btn3_state == HIGH)
  {
    digitalWrite(btn3_out, HIGH);
    digitalWrite(btn_pressed_pin, HIGH);
    Serial.write(state3_out_byte);
    //Serial.println("Button 3: Pull Me Up Pressed");
    delay(200);
  }
  else if (vc3_state == HIGH)
  {
    digitalWrite(btn3_out, HIGH);
    Serial.write(state3_out_byte);
    //Serial.println("VC 3: Pull Me Up said");
    delay(300);
  }
  else
  {
    digitalWrite(btn3_out, LOW);
    digitalWrite(btn_pressed_pin, LOW);
  }

}
