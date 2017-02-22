#include <SPI.h>
#include <Ethernet.h>
#include <UbidotsEthernet.h>
#include <RTClib.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

unsigned long interval=1000; // the time we need to wait
unsigned long interval1=20000;
unsigned long previousMillis=0; // millis() returns an unsigned long.

#define ONE_WIRE_BUS 2
#define PHSensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;

#define IDSuhu "5829c80d76254214e3f9d3a1"
#define IDpH "58a9dd0b762542238e7cf8ae"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

Ubidots client(TOKEN);

void setup(){
  //Serial.begin(9600);
//  pinMode(4,OUTPUT);
//  digitalWrite(4,HIGH);

  if (Ethernet.begin(mac) == 0) {
      //Serial.println("Failed to configure Ethernet using DHCP");
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
  }
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {
     previousMillis = millis()+4000;
  }

}

void loop() {
  unsigned long currentMillis = millis();
  
  for(int i=0;i<10;i++){ 
      buf[i]=analogRead(PHSensorPin);
      if ((unsigned long)(currentMillis - previousMillis) >= interval) {
        previousMillis = millis();
      }
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
  sensorSuhu.requestTemperatures();
  float suhu = sensorSuhu.getTempCByIndex(0);
  client.add(IDSuhu, suhu);
  client.add(IDpH, phValue);
  //Serial.println(suhu);
  //Serial.println(phValue);
  if ((unsigned long)(currentMillis - previousMillis) >= interval1) {
     previousMillis = millis();     
     client.sendAll();
  }
}
