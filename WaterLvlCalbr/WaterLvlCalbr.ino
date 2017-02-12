int floatswitch1 = 6;
int floatswitch2 = 7;

void setup() {
  // put your setup code here, to run once:
  pinMode(floatswitch1, INPUT_PULLUP);
  pinMode(floatswitch2, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int x = digitalRead(floatswitch1);
  int y = digitalRead(floatswitch2);

  delay(2000);
  
  if (x == LOW && y == LOW){
    Serial.println("++[ AIR 1]++");
  }
  if (x == HIGH && y == LOW){
    Serial.println("++[ AIR 2]++");
  }  
  if (x == LOW && y == HIGH){
    Serial.println("++[ AIR 3]++");
  }  
  if (x == HIGH && y == HIGH){
    Serial.println("++[ AIR 4]++");
  }  

  Serial.println(x);
  Serial.println(y);
  Serial.println("===============");
}
