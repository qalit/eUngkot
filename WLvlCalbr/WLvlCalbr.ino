int floatswitch1 = A1;
int floatswitch2 = A2;

void setup() {
  // put your setup code here, to run once:
  pinMode(floatswitch1, INPUT_PULLUP);
  pinMode(floatswitch2, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int x = analogRead(floatswitch1);
  int y = analogRead(floatswitch2);

  delay(500);
  
  if (x <= 1000 && y <= 1000){
    Serial.println("++[ AIR 1]++");
    analogWrite(A3, 0); 
  }
  if (x >= 1000 && y <= 1000){
    Serial.println("++[ AIR 2]++");
  }  
  if (x <= 1000 && y >= 1000){
    Serial.println("++[ AIR 3]++");
  }  
  if (x >= 1000 && y >= 1000){
    Serial.println("++[ AIR 4]++");
    analogWrite(A3, 255); 
  }  

  Serial.println(x);
  Serial.println(y);
  Serial.println("===============");
}

