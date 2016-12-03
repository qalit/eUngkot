#include <UbidotsESP8266.h>
#include <SoftwareSerial.h> 

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;

#define ssid "Darkstar"
#define pass "poss1234"

#define ID "5829d046762542467bb8066d"
#define ID2 "5829d051762542481c6f8e33"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

Ubidots client(TOKEN);

void setup() {
  Serial.begin(9600);
  client.wifiConnection(ssid,pass);
  suhuSekarang = ambilSuhu();
  delay(1000);
}

void loop() {
  suhuSekarang = ambilSuhu();
  Serial.print("Suhu : ");   
  Serial.println(sensorSuhu.getTempCByIndex(0));
  client.add(ID, suhuSekarang);
  client.sendAll();
  Serial.println("----------");
  delay(3000); //Delay 2 sec.
    
}
float ambilSuhu(){
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   
}

