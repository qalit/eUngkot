#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;
 
void setup(void){
  Serial.begin(115200);
  pinMode(1, OUTPUT);
  sensorSuhu.begin();
}
 
void loop(void){
  suhuSekarang = ambilSuhu();
  Serial.print("Suhu : ");   
  Serial.println(sensorSuhu.getTempCByIndex(0));         
  
  delay(1000);
  if(suhuSekarang >= 30 ){
    Serial.println("++[ AIR DIPASOK ]++");
    digitalWrite(1, HIGH);
  }
}
 
float ambilSuhu(){
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   
}
