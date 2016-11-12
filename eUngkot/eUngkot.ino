/* DJ Pakan - Clarias Gariepinus Feeder

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

char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};
int pos = 0;

void setup () {

  Serial.begin(9600);
  
  delay(3000); // wait for console opening

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
    Serial.print((HariPertama.secondstime()-now1.secondstime())/86400);
    Serial.print(" Sejak 1/11/16(Tebar Pertama)");
    Serial.println();
    
    suhuSekarang = ambilSuhu();
    Serial.print("Suhu : ");   
    Serial.println(suhuSekarang); 
    delay(3000);

   if((now.hour() == 10 && now.minute() == 27 )){
    delay(3000);
    myservo.attach(9);
    myservo.write(pos);
    delay(2000);
      for (pos = 0; pos <= 180;) { // servo berputar 180 derajat membuka katup pakan
        // in steps of 1 degree
        delay(3000);                       // delay selama 1detik
        myservo.write(pos);   // servo menutup kembali katup pakan
        
        } 
        myservo.detach();
    } 


}

float ambilSuhu()
{
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   
}
int ratioDelay()
{
      
}
