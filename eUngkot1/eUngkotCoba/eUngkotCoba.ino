/* eUngkot - Clarias Gariepinus Feeder
Time RTC set servo to move feed 

 circuit:
 *base on fritzing sketch & proteus simulator
 *available : qalit.io/eungkot
 by Al Qalit @ 2016

 This example code is in the public domain.
 http://www.github.com/qalit/eungkot
 */
 
#include <Wire.h>
#include "RTClib.h"
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
// sensor diletakkan di pin 2
#define ONE_WIRE_BUS 2
 
// setup sensor
OneWire oneWire(ONE_WIRE_BUS);
 
// berikan nama variabel,masukkan ke pustaka Dallas
DallasTemperature sensorSuhu(&oneWire);
 
float suhuSekarang;
RTC_DS3231 rtc;

int servoPin1 = 9;
int servoPin2 = 10;
int servoPin3 = 11;
int srvPinPasok = 13;
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

char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

int NilaiWLvlAtas = LOW;
int NilaiWLvlBawah = LOW;

void setup () {
  Serial.begin(9600);
  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);
  myservo3.attach(servoPin3);
  srvPasokAir.attach(srvPinPasok);
  srvKurasAir.attach(srvPinKuras);
  pinMode(WLvlAtas, INPUT);
  pinMode(WLvlBawah, INPUT);
  
  if (! rtc.begin()) {
    Serial.println("RTC Mati");
    while (1);
  }

  if (rtc.lostPower()) {
    //rtc.adjust(DateTime(__DATE__,__TIME__));
    Serial.println("RTC lowbat !!");
  
  }
  sensorSuhu.begin();
}

void loop () {
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
    Serial.print((RasioPakan*HariKe*JumlahLele)); 
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

    
    //Pengaturan waktu 09:00 pemberian pakan 
    if((now.hour() == 10 && now.minute() == 13 ))
    {
 
      myservo1.write(180); //buka katup pakan
      delay(RasioPakan*100000*HariKe); //delay sebanyak (HariKe*1s)
      Serial.println("++[ WAKTU PAKAN 1 ]++");
      
      for (servoAngle = 180; servoAngle > 0; servoAngle--) 
      { // servo kembali ke posisi 0 menutup kotak pakan
        myservo1.write(servoAngle);          
        delay(10);
      }
        myservo1.detach(); //Matikan Servo
        
        Serial.print("Pakan diberi sebanyak : ");
        Serial.print(RasioPakan);
        Serial.println("Gr");
        Serial.println(" ");
    } 
 
    //Pengaturan waktu 12:00 pemberian pakan 
    if((now.hour() == 10 && now.minute() == 15 ))
    {
      Serial.println("++[ WAKTU PAKAN 2 ]++");
      myservo2.write(180); //buka katup pakan
      delay(RasioPakan*100000*HariKe); //delay sebanyak (HariKe*1s)
      
      
      for (servoAngle = 180; servoAngle > 0; servoAngle--) 
      { // servo berputar 180 derajat membuka katup pakan
  
        myservo2.write(servoAngle);          
        delay(10);
      }
        myservo2.detach(); //Matikan Servo
        
        Serial.print("Pakan diberi sebanyak : ");
        Serial.print(HariKe);
        Serial.println("Gr");
        Serial.println(" ");
    } 

     //Pengaturan waktu 09:00 pemberian pakan 
    if((now.hour() == 18 && now.minute() == 15 ))
    {
      Serial.println("++[ WAKTU PAKAN 3 ]++");
      myservo3.write(180); //buka katup pakan
      delay(RasioPakan*100000*HariKe); //delay sebanyak (HariKe*1s)
      
      
      for (servoAngle = 180; servoAngle > 0; servoAngle--) 
      { // servo berputar 180 derajat membuka katup pakan
  
        myservo3.write(servoAngle);          
        delay(10);
      }
        myservo3.detach(); //Matikan Servo
        
        Serial.print("Pakan diberi sebanyak : ");
        Serial.print(HariKe);
        Serial.println("Gr");
        Serial.println(" ");
    } 

    //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
    if(ambilSuhu() > 30){
      Serial.println("++[ AIR DIKURAS ]++");
      srvKurasAir.write(180); //buka katup pakan
      srvPasokAir.write(1);
      delay(5000);
      }
//      if (NilaiWLvlBawah == HIGH && NilaiWLvlAtas == LOW){
//        Serial.println("++[ AIR DIPASOK]++");
//        srvKurasAir.write(1);
//        srvPasokAir.write(180);
//        }
//        if (NilaiWLvlAtas == HIGH && NilaiWLvlBawah == HIGH){
//          srvKurasAir.write(0);
//          srvPasokAir.write(0);
//          } else{}

}

float ambilSuhu()
{
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   
}
