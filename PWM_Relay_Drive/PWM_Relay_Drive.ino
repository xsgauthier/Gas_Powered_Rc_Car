#include <Wire.h>
#include <Servo.h>

const int pwm_pin = 3;
const int A_pin = 4;
const int B_pin = 5;

Servo myservo; 
const int gaz_servo_pin = 9;
const int vmotor_analog = A0;

int drive_direction = 0; // 0 = break, 1 = Reverse, 2 = Forward
int pwm_command = 0; // 0-255
int servo_command = 0; // 0-180

void setup()
{
  pinMode(pwm_pin, OUTPUT);
  pinMode(A_pin, OUTPUT);
  pinMode(B_pin, OUTPUT);
  myservo.attach(gaz_servo_pin);
  
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  Serial.begin(9600);           // start serial for output
}

void loop()
{
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(2 < Wire.available()) // loop through all but the last
  {
    int x = Wire.read();    // receive byte as an integer
    drive_direction = x;
    Serial.print(x);         // print the character
    Serial.print(" ");         // print the character
  }
  if (drive_direction & 1)
    digitalWrite(A_pin, HIGH);
  else
    digitalWrite(A_pin, LOW);
  if (drive_direction & 2)
    digitalWrite(B_pin, HIGH);
  else
    digitalWrite(B_pin, LOW);
    
  servo_command = Wire.read();    // receive byte as an integer
  Serial.println(servo_command);         // print the integer
  myservo.write(servo_command);
  
  pwm_command = Wire.read();    // receive byte as an integer
  Serial.println(pwm_command);         // print the integer
  analogWrite(pwm_pin, pwm_command);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write(analogRead(vmotor_analog)); // respond with message of 2 bytes
}
