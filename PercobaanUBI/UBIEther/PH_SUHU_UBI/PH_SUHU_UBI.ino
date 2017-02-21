#include <SPI.h>
#include <Ethernet.h>
#include <UbidotsEthernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
#define PHSensorPin 0  
#define IDSuhu "5829c80d76254214e3f9d3a1"
#define IDpH "58a9dd0b762542238e7cf8ae"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;
unsigned long int avgValue; 
float b;
int buf[10],temp;

Ubidots client(TOKEN);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 177);

void setup(){
  Serial.begin(9600);
  
  if (Ethernet.begin(mac) == 0) {
      Serial.println("Ether Failed");
      Ethernet.begin(mac, ip);
  }
  
  suhuSekarang = ambilSuhu();
  delay(5000);
}

void loop() {
  suhuSekarang = ambilSuhu();
  Serial.print("Suhu : ");   
  Serial.println(sensorSuhu.getTempCByIndex(0));
   // Sensor pH
  for(int i=0;i<10;i++){ 
      buf[i]=analogRead(PHSensorPin);
      delay(10);
  }
  for(int i=0;i<9;i++){
    for(int j=i+1;j<10;j++){
      if(buf[i]>buf[j]){
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
  avgValue+=buf[i];
  
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;
  delay(1000);
 
  client.add(IDSuhu, suhuSekarang);
  client.add(IDpH, phValue);
  client.sendAll();
  delay(20000); //Delay 20 sec to send and get respon to eungkot  
}
float ambilSuhu(){
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;  

}
