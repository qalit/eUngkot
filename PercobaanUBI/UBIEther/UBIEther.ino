#include <SPI.h>
#include "RTClib.h"
#include <Servo.h>
#include <Ethernet.h>
#include <UbidotsEthernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
#define IDSuhu "5829c80d76254214e3f9d3a1"
#define IDPakan "5829c8a476254218b9f109e7"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

int servoPin1 = 8;
int servoPin2 = 9;
int servoPin3 = 10;
int srvPinPasok = 11;
int srvPinKuras = 12;
Servo myservo1;  // servo pakan 1
Servo myservo2;  // servo pakan 2
Servo myservo3; // servo pakan 3
Servo srvPasokAir; // servo pasok air
Servo srvKurasAir; // servo kuras air

int WLvlAtas = A2;
int WLvlBawah = A3;

int servoAngle = 0;
int srvAngle1 = 0;
int srvAngle2 = 0;

int NilaiWLvlAtas = LOW;
int NilaiWLvlBawah = LOW;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

Ubidots client(TOKEN);

void setup() {
  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("RTC Mati");
    while (1);
  }
  if (rtc.lostPower()) {
    //rtc.adjust(DateTime(__DATE__,__TIME__));
    Serial.println("RTC lowbat !!");
  
  }
  if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
  }
  suhuSekarang = ambilSuhu();
  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);
  myservo3.attach(servoPin3);
  srvPasokAir.attach(srvPinPasok);
  srvKurasAir.attach(srvPinKuras);
  pinMode(WLvlAtas, INPUT);
  pinMode(WLvlBawah, INPUT);
  sensorSuhu.begin();
  delay(5000);
}

void loop() {
    NilaiWLvlAtas = analogRead(WLvlAtas);
    NilaiWLvlBawah = analogRead(WLvlBawah);

   //Bagian deklarasi waktu sekarang dan waktu sejak tebar pertama
    DateTime now = rtc.now();
    DateTime HariPertama = DateTime(__DATE__,__TIME__);
    
  //TGL hari pertama & jumlah lele
    DateTime now1 (2016, 11, 1, 0, 0, 0);
    int JumlahLele = 100;
    int HariKe = ((HariPertama.secondstime() - now1.secondstime())/86400);
    float RasioPakan = 0.0039;
    float RasioHarian = RasioPakan*HariKe*JumlahLele;
    
  //Tampilkan Waktu sekarang
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print("| ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    //hitung hari sejak program di upload
    Serial.print("Hari ke: ");
    Serial.print((HariKe));
    Serial.println(" Sejak 1/11/16(Tebar Pertama)");
    Serial.print("Rasio Pakan Harus Diberi : ");
    Serial.print(RasioHarian); 
    Serial.println(" Gram");
    Serial.println();
    
    //Ambil data suhu dan tampilkan
    suhuSekarang = ambilSuhu();
    Serial.print("Suhu : ");   
    Serial.println(suhuSekarang); 
    myservo1.write(2);
    myservo2.write(2);
    myservo3.write(2);
    srvPasokAir.write(2);
    srvKurasAir.write(2);
    
    delay(1000);
    
    //Pengaturan waktu 12:00 pemberian pakan 
    if((now.hour() == 12 && now.minute() == 28 ))
    {
      Serial.println("++[ WAKTU PAKAN 2 ]++");
      myservo2.write(180); //buka katup pakan
      delay(RasioPakan*1000*HariKe); //delay sebanyak (HariKe*1s)
      
      for (servoAngle = 180; servoAngle > 0; servoAngle--) 
      { // servo berputar 180 derajat membuka katup pakan
  
        myservo2.write(servoAngle);          
        delay(10);
      }
      myservo2.detach(); //Matikan Servo
      
      Serial.print("Pakan diberi sebanyak : ");
      Serial.print(RasioHarian);
      Serial.println("Gr");
      Serial.println(" ");
      client.add(IDPakan, RasioHarian);
      client.sendAll();
    } 
    
    if(ambilSuhu() > 30){
      Serial.println("++[ AIR DIKURAS ]++");
      srvKurasAir.write(180); //buka katup pakan
      srvPasokAir.write(1);
      delay(5000);
    }
    
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

