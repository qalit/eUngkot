#include <Ethernet.h>
#include <UbidotsEthernet.h>

#define IDRelay "58a348107625421325decb58"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"
#define RELAY 12

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

Ubidots client(TOKEN);

void setup(){
  Serial.begin(9600);
  
  if (Ethernet.begin(mac) == 0) {
      Serial.println("ETHERNET MATI");
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
  }
  
  delay(10);
  pinMode(RELAY, OUTPUT);
}
void loop(){
  float value = client.getValue(IDRelay);
  Serial.println(value);
  if (value == 1.00) {
      digitalWrite(RELAY, HIGH); //On relay
  }
  if (value == 0.00) {
      digitalWrite(RELAY, LOW); //Off relay
  }
}
