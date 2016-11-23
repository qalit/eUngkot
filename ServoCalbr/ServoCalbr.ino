/*
Into Robotics
*/
 
#include <Servo.h>
 
int servoPin = 9;
 
Servo servo;  
 
int servoAngle = 0;   // servo position in degrees
 
void setup()
{
  Serial.begin(9600);  
  servo.attach(servoPin);
}
 
 
void loop()
{
//control the servo's direction and the position of the motor


   //servo.write(2);      // Turn SG90 servo back to 90 degrees (center position)
   servo.write(180);
   delay(5000); 

//end control the servo's direction and the position of the motor


//control the servo's speed  

//if you change the delay value (from example change 50 to 10), the speed of the servo changes

  for(servoAngle = 180; servoAngle > 0; servoAngle--)  //now move back the micro servo from 0 degrees to 180 degrees
  {                                
    servo.write(servoAngle);          
    delay(10);
  }
  servo.detach();

  //end control the servo's speed  
}
