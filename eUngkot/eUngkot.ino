/* eUngkot - Clarias Gariepinus Feeder

Time RTC set servo to move feed door 

 The circuit:
 *base on fritzing sketch
 *available : qalit.io/eungkot
 
 created 2016
 by Al Qalit
 Base on code jeelabs rtc-libs

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
Servo myservo;  // create servo object to control a servo
Servo myservo1;  // create servo object to control a servo

char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};
int pos = 0;

void setup () {

  Serial.begin(9600);
  
  delay(3000); 

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

  //Bagian deklarasi waktu sekarang dan waktu sejak tebar pertama
    DateTime now = rtc.now();
    DateTime HariPertama = DateTime(__DATE__,__TIME__);
    DateTime now1 (2016, 11, 1, 0, 0, 0);
    int HariKe = ((HariPertama.secondstime() - now1.secondstime())/86400);
    int RasioDelay = HariKe * 1000;
    
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

    //hitung hari sejak program di compile
    Serial.print("Hari ke: ");
    Serial.print((HariKe));
    Serial.print(" Sejak 1/11/16(Tebar Pertama)");
    Serial.println();

    //Ambil data suhu dan tampilkan
    suhuSekarang = ambilSuhu();
    Serial.print("Suhu : ");   
    Serial.println(suhuSekarang); 
    delay(3000);
    
    //Pengaturan waktu 09:00 pemberian pakan 
    if((now.hour() == 9 && now.minute() == 00 )){
    delay(3000); 
    myservo.attach(9); 
    myservo.write(pos);
    delay(2000);
    for (pos = 0; pos <= 180;) { // servo berputar 180 derajat membuka katup pakan
      
      delay(RasioDelay); //delay sebanyak (HariKe*1s)
      Serial.print("Pakan diberi sebanyak : ");
      Serial.print(HariKe);
      Serial.println("Gr");
      
      myservo.write(pos);   // servo menutup kembali katup pakan
      
      }
      myservo.detach(); //Matikan Servo
    } 
    
    //Pengaturan waktu 12:00 pemberian pakan 
    if((now.hour() == 12 && now.minute() == 00 )){
    delay(3000); 
    myservo.attach(9); 
    myservo.write(pos);
    delay(2000);
    for (pos = 0; pos <= 180;) { // servo berputar 180 derajat membuka katup pakan
      
      delay(RasioDelay); //delay sebanyak (HariKe*1s)
      Serial.print("Pakan diberi sebanyak : ");
      Serial.print(HariKe);
      Serial.println("Gr");
      
      myservo.write(pos);   // servo menutup kembali katup pakan
      
      }
      myservo.detach(); //Matikan Servo
    } 

    //Pengaturan waktu 17:00 pemberian pakan 
    if((now.hour() == 18 && now.minute() == 6 )){
    myservo.attach(9);
    for (pos = 0; pos <= 180;) { // servo berputar 180 derajat membuka katup pakan
      
      delay(RasioDelay); //delay sebanyak (HariKe*1s)
      Serial.print("Pakan diberi sebanyak : ");
      Serial.print(HariKe);
      Serial.println("Gr");
      
      myservo.write(0);   // servo menutup kembali katup pakan
      
      }
      myservo.detach(); //Matikan Servo
      
    } 

    //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
    if((suhuSekarang > 30)){ 
    myservo1.attach(10);
    myservo1.write(pos);
    delay(2000);
      for (pos = 0; pos <= 180;) { // servo berputar 180 derajat membuka katup pakan

        //Matikan Ketika LevelAirAtas==HIGH
        
        } 
    }

}

float ambilSuhu()
{
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   
}
