#include <Servo.h>
//Servo
Servo servo;
int servoPin = 11;
int servoAngle;
int oldServoAngle = 0;
int stepsize = 20;
//Solenoid and Pressure Sensor
int S1 = 10; //Digital pint solenoid valve is connected to
int P1 = A1; //Absoulte Pressure Sensor Measuring P1
int P2 = A0; // "" "" "" "" P2
float atm = 101.325; //Atm pressure
float Pressure1;
float Pressure2;
float diffP;
float bits;
float Vout;
//Encoder Interrupt
static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

void setup() {
  pinMode(S1, OUTPUT);
  servo.attach(servoPin);

  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)

  Serial.begin(500000);
}
void PinA() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB() {
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}
//Functions

void ON(int SOLENOID) {
  digitalWrite(SOLENOID, HIGH);
}
void OFF(int SOLENOID) {
  digitalWrite(SOLENOID, LOW);
}
void loop() {

  if (oldEncPos != encoderPos) {
    Serial.print(encoderPos);
    Serial.print(" ,");
    Serial.println(oldEncPos);
    oldEncPos = encoderPos;

    servoAngle = map(encoderPos, 0, 255, 0 , 180);
    servo.write(servoAngle);
    Serial.println(servoAngle);
  }

  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == '0') {
      ON(S1);
    }
    else if (ch == '1') {
      OFF(S1);
    }
    else if (ch == '4') {
      unsigned long current_time = millis();
      OFF(S1);
      while (millis() - current_time < 1500) {
        if (millis() - current_time > 200) {
          Serial.print(float(millis() - current_time));
          Serial.print(" , ");
          bits = analogRead(P2);
          Vout = bits * 5 / 1023;
          diffP = ((Vout / 5.0) - 0.04) / 0.0018 + 2.13 - 0.54;
          Serial.print(diffP);
          Serial.print(" , ");
          Serial.println(servoAngle);
        }
      }
      OFF(S1);

    }

  }

}
