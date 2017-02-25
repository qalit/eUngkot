#include <Ethernet.h>
#include <SPI.h>
#include <UbidotsEthernet.h>
#define IDSuhu "5829c80d76254214e3f9d3a1"
#define IDpH "58a9dd0b762542238e7cf8ae"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);

#define PHSensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

byte mac[] = { 0x33, 0x33, 0x00, 0x00, 0x00, 0xFB };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

Ubidots client(TOKEN);

unsigned long interval=1000; // the time we need to wait
unsigned long interval1=60000;
unsigned long previousMillis=0; // millis() returns an unsigned long.

void setup(){
  if (Ethernet.begin(mac) == 0) {
   //Serial.println("Failed to configure Ethernet using DHCP");
  // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();
  
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
  for(int i=2;i<8;i++){
    avgValue+=buf[i];
  }
  
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;
  
  sensorSuhu.requestTemperatures();
  float suhu = sensorSuhu.getTempCByIndex(0);
  
  //if ((unsigned long)(currentMillis - previousMillis) >= interval1) {
    client.add(IDSuhu, suhu);
    client.add(IDpH, phValue);
    client.sendAll();
    delay(60000);
    //client.freeRam();
  //}
}
