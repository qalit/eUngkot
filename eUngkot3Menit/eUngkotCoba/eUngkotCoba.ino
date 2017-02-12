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
#include <LiquidCrystal_I2C.h>
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
LiquidCrystal_I2C lcd(0x27,16,2);

int servoPin1 = 8;
int servoPin2 = 9;
int servoPin3 = 10;
int servoPin4 = 11;
int srvPinKuras = 12;
int srvPinPasok = 13;
Servo myservo1;  // servo pakan 1
Servo myservo2;  // servo pakan 2
Servo myservo3; // servo pakan 3
Servo myservo4; // servo pakan 4
Servo srvPasokAir; // servo pasok air
Servo srvKurasAir; // servo kuras air

int WLvlAtas = 7;
int WLvlBawah = 6;

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
  myservo4.attach(servoPin4);
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
  lcd.init();
  lcd.backlight();
  sensorSuhu.begin();
}

void loop () {
  NilaiWLvlAtas = analogRead(WLvlAtas);
  NilaiWLvlBawah = analogRead(WLvlBawah);
  
  //Bagian deklarasi waktu sekarang dan waktu sejak tebar pertama
    DateTime now = rtc.now();
    DateTime HariPertama = DateTime(__DATE__,__TIME__);
    
  //TGL hari pertama & jumlah lele
    DateTime HariTebar (2017, 2, 1, 0, 0, 0);
    int JumlahLele = 100;
    int HariKe = ((HariPertama.secondstime() - HariTebar.secondstime())/86400);
    float RasioPakan = 0.0039;
    
  //Tampilkan Waktu sekarang
    lcd.setCursor(0,0);
    lcd.print(now.year(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.day(), DEC);
    lcd.print(" (");
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(") ");
    lcd.print("| ");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print(now.second(), DEC);
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
    lcd.setCursor(0,1);
    lcd.print("Hari: ");
    lcd.print((HariKe));
    lcd.print(" Pakan: ");
    lcd.print((RasioPakan*HariKe*JumlahLele)); 
    lcd.println(" Gram");
    Serial.print("Hari ke: ");
    Serial.print((HariKe));
    Serial.println(" Sejak 1/12/16(Tebar Pertama)");
    Serial.print("Rasio Pakan Harus Diberi : ");
    Serial.print((RasioPakan*HariKe*JumlahLele)); 
    Serial.println(" Gram");
    Serial.println();

    //Ambil data suhu dan tampilkan
    suhuSekarang = ambilSuhu();
    lcd.print("Suhu : ");   
    lcd.println(suhuSekarang); 
    Serial.print("Suhu : ");   
    Serial.println(suhuSekarang); 
    for(int i=0; i < 16; i++){
      lcd.scrollDisplayLeft();
      delay(500);
    }
    myservo1.write(2);
    myservo2.write(2);
    myservo3.write(2);
    myservo4.write(2);
    srvPasokAir.write(2);
    srvKurasAir.write(2);
    delay(2000);
    srvKurasAir.detach();
    
    //Pengaturan waktu 09:00 pemberian pakan 
    if((now.hour() == 9 && now.minute() == 00 ))
    {
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 1 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(10);
        }
        myservo2.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(10);
        }
          
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 1 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(10);
        }
        myservo4.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(10);
        }
          
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } 
    } 
 
    //Pengaturan waktu 12:00 pemberian pakan 
    if((now.hour() == 15 && now.minute() == 54 ))
    {
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(10);
        }
        myservo2.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(10);
        }
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 2 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(10);
        }
        myservo4.write(90); //buka katup pakan
        delay(2000); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(10);
        }
          
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      }
    }

     //Pengaturan waktu 17:00 pemberian pakan 
     if((now.hour() == 19 && now.minute() == 25 ))
     {
      if(HariKe >= 1 && HariKe <= 28){
        lcd.clear();
        lcd.setCursor(0,0);
        myservo1.write(90); //buka katup pakan
        delay(1000); //delay sebanyak (HariKe*1s)
        lcd.print("  WAKTU PAKAN 3  ");
        lcd.print(1000*10*(RasioPakan*HariKe*10));
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(10);
        }
        myservo2.write(90); //buka katup pakan
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(10);
        }
          lcd.setCursor(0,1);
          lcd.print("Diberi : ");
          lcd.print(RasioPakan*HariKe*JumlahLele);
          lcd.println("Gr");
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo2.write(90); //buka katup pakan
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(10);
        }
          myservo2.detach(); //Matikan Servo
          
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } 
    }

    //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
    if(ambilSuhu() >= 30 ){
      lcd.clear();
      lcd.setCursor(0,0);
      srvKurasAir.attach(srvPinKuras);
      lcd.print("  Air Dikuras ");
      Serial.println("++[ AIR DIKURAS ]++");
      srvKurasAir.write(90); //buka katup pakan
      delay(10000);
      }
      if (NilaiWLvlBawah == HIGH && NilaiWLvlAtas == HIGH){
        Serial.println("++[ AIR DIPASOK]++");
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { 
          srvKurasAir.write(servoAngle);          
          delay(10);
          srvKurasAir.detach();
        }
        srvPasokAir.attach(srvPinPasok);
        srvPasokAir.write(90);
       }
        if (NilaiWLvlAtas == LOW && NilaiWLvlBawah == LOW){
          srvPasokAir.write(0);
          delay(10);
          srvPasokAir.detach();          
        }

}

float ambilSuhu()
{
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0)+1;
   return suhu;   
}
