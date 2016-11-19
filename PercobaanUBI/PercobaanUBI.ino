#include "UbidotsESP8266.h"
#include <SoftwareSerial.h> 


#define SSID "@aneuk.kupi 2"
#define PASS "aneukkupi"


#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"
#define IDSuhu "5829d046762542467bb8066d" // Replace it with your Ubidots' variable ID
#define IDKelembaban "5829d051762542481c6f8e33" 

Ubidots client(TOKEN);

void setup() {
  Serial.begin(9600);
  client.wifiConnection(SSID,PASS);
}
void loop() {
  float value = analogRead(A0);
  float value2 = analogRead(A1);
  float value3 = analogRead(A2);
  float value4 = analogRead(A3);
  client.add(ID,value);
  client.add(ID2,value2);
  client.add(ID3,value3);
  client.add(ID4,value4);
  client.sendAll();
}
