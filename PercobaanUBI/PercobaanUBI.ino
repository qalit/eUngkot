#include <SPI.h>
#include <UbidotsESP8266.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char SSID[] = "Darkstar"; //  your network SSID (name)
char PASS[] = "123456789";    // your network password (use for WPA, or use as key for WEP)

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;

#define ID "5829c80d76254214e3f9d3a1"
#define ID2 "5829d051762542481c6f8e33"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

Ubidots client(TOKEN);

void setup() {
  Serial.begin(115200);
  client.wifiConnection(SSID,PASS);
  
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
  delay(10000); //Delay 2 sec.

}
float ambilSuhu() {
  sensorSuhu.requestTemperatures();
  float suhu = sensorSuhu.getTempCByIndex(0);
  return suhu;
}
