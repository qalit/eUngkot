#include <SPI.h>
#include <Ethernet.h>
#include <UbidotsEthernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;

#define IDSuhu "5829c80d76254214e3f9d3a1"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
//DHCP IP untuk arduino
Ubidots client(TOKEN);

void setup() {
  Serial.begin(9600);
  if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
  }
  suhuSekarang = ambilSuhu();
  delay(5000);
}

void loop() {
  suhuSekarang = ambilSuhu();
  Serial.print("Suhu : ");   
  Serial.println(sensorSuhu.getTempCByIndex(0));
  client.add(IDSuhu, suhuSekarang);
  client.sendAll();
  Serial.println("----------");
  delay(5000); //Delay 2 sec.
    
}
float ambilSuhu(){
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   
}

