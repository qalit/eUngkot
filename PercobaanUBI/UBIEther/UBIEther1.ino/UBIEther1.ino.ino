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
unsigned long interval1=60000;
unsigned long previousMillis=0; // millis() returns an unsigned long.

#define ONE_WIRE_BUS 2
#define PHSensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

int WLvlAtas = 6;
int WLvlBawah = 7;
int servoPin1 = 8;
int servoPin2 = 9;
int servoPin3 = 3;
int servoPin4 = 5;
int srvPinKuras = 12;
Servo myservo1;  // servo pakan 1
Servo myservo2;  // servo pakan 2
Servo myservo3; // servo pakan 3
Servo myservo4; // servo pakan 4
Servo srvKurasAir; // servo kuras air

int servoAngle = 0;

#define IDSuhu "5829c80d76254214e3f9d3a1"
#define IDpH "58a9dd0b762542238e7cf8ae"
#define IDPakan "5829c8a476254218b9f109e7"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

Ubidots client(TOKEN);

void setup(){
  unsigned long currentMillis = millis();
    //Serial.begin(9600);
  if (Ethernet.begin(mac) == 0) {
      //Serial.println("Failed to configure Ethernet using DHCP");
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
  }
  if ((unsigned long)(currentMillis - previousMillis) >= 5000) {
     delay(5000);
  }
  if (! rtc.begin()) {
    ////Serial.println("RTC Mati");
    while (1);
  }
  if (rtc.lostPower()) {
    //rtc.adjust(DateTime(__DATE__,__TIME__));
    ////Serial.println("RTC lowbat !!");
  
  }
  pinMode(WLvlAtas, INPUT_PULLUP);
  pinMode(WLvlBawah, INPUT_PULLUP);

  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);
  myservo3.attach(servoPin3);
  myservo4.attach(servoPin4);
  srvKurasAir.attach(srvPinKuras);
  delay(1000);
  myservo1.write(2);
  myservo2.write(2);
  myservo3.write(2);
  myservo4.write(2);
  srvKurasAir.write(2);
  delay(2000);
  srvKurasAir.detach();
    
}

void loop() {
  unsigned long currentMillis = millis();
//  if ((unsigned long)(currentMillis - previousMillis) >= interval) {
//     delay(5000);
//  }
  delay(1000);
  for(int i=0;i<10;i++){ 
      buf[i]=analogRead(PHSensorPin);
      if ((unsigned long)(currentMillis - previousMillis) >= interval) {
        delay(10);
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
  
  if ((unsigned long)(currentMillis - previousMillis) >= interval1) {     
     client.add(IDSuhu, suhu);
     client.add(IDpH, phValue);
     previousMillis = millis();
     client.sendAll();
     delay(60000);

  }
}
