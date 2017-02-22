#include <Servo.h>
 
int servoPin1 = 12;
int servoPin2 = 10;
Servo servo1;  
Servo servo2;
int servoAngle1 = 0;   // servo position in degrees
int servoAngle2 = 0;

void setup(){
  Serial.begin(9600);  
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
}

void loop(){
  
  servo1.attach(servoPin1);
   servo1.write(90);
   delay(2000); 

  for(servoAngle1 = 90; servoAngle1 > 0; servoAngle1--){                                
    servo1.write(servoAngle1);          
    delay(10);
  }
  servo1.detach();

  servo2.write(90);
   delay(2000); 

  for(servoAngle2 = 90; servoAngle2 > 0; servoAngle2--){                                
    servo2.write(servoAngle2);          
    delay(10);
  }

}
