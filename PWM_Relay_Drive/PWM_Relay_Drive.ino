#include <Wire.h>
#include <Servo.h>
#include <TimerEvent.h>

const int pwm_pin = 3;
const int A_pin = 4;
const int B_pin = 5;

Servo myservo; 
const int gaz_servo_pin = 9;
const int vmotor_analog = A0;

int drive_command = 0; // 0 = break, 1 = Reverse, 2 = Forward
int drive_output = 0;
int pwm_command = 0; // 0-255
int pwm_output = 0; // 0-255
int servo_command = 0; // 0-180
int servo_output = 0;

TimerEvent servoTimer;
TimerEvent motorTimer;

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

  servoTimer.set(5, servoTimerFunc);
  motorTimer.set(50, motorTimerFunc);
}

void loop()
{
  servoTimer.update();
  motorTimer.update();
}

// Generate a ramp output for servo to move not too fast
void servoTimerFunc() {
  if (servo_output < servo_command)
  {
    servo_output++;
  }
  else if (servo_output > servo_command)
  {
    servo_output--;
  }
  myservo.write(servo_output);
}

// Generate a ramp output for servo to move not too fast
void motorTimerFunc() {
  if ((drive_command & 3) == 0) {
    // want to brake
    if (pwm_output > 0) {
      pwm_output--;
    } else {
      drive_output = drive_command;
    }
  } else if ((drive_output & 3) != (drive_command & 3)) {
    // Direction change
    if (pwm_output > 0) {
      pwm_output--; // must pass by 0
    } else {
      drive_output = drive_command;
    }    
  } else {
    if (pwm_output < pwm_command) {
      pwm_output++;
    } else if (pwm_output > pwm_command) {
      pwm_output--;
    }
  }
  
  if (drive_output & 1)
    digitalWrite(A_pin, HIGH);
  else
    digitalWrite(A_pin, LOW);
  if (drive_output & 2)
    digitalWrite(B_pin, HIGH);
  else
    digitalWrite(B_pin, LOW);
  analogWrite(pwm_pin, pwm_output);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while(2 < Wire.available()) // loop through all but the last
  {
    int x = Wire.read();    // receive byte as an integer
    drive_command = x;
    Serial.print(x);         // print the character
    Serial.print(" ");         // print the character
  }
    
  servo_command = Wire.read();    // receive byte as an integer
  Serial.println(servo_command);         // print the integer
  
  pwm_command = Wire.read();    // receive byte as an integer
  Serial.println(pwm_command);         // print the integer
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write(analogRead(vmotor_analog)); // respond with message of 2 bytes
}
