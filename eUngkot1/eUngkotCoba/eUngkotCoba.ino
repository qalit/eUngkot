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
#define PHSensorPin 0          //pH meter Analog output to Arduino Analog Input 0

// sensor suhu diletakkan di pin 2
#define ONE_WIRE_BUS 2
 
// setup sensor
OneWire oneWire(ONE_WIRE_BUS);

unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

// berikan nama variabel,masukkan ke pustaka Dallas
DallasTemperature sensorSuhu(&oneWire);
 
float suhuSekarang;
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27,16,2);

int WLvlBawah = 6;
int WLvlAtas = 7;
int servoPin1 = 8;
int servoPin2 = 9;
int servoPin3 = 10;
int servoPin4 = 11;
int srvPinKuras = 12;
Servo myservo1;  // servo pakan 1
Servo myservo2;  // servo pakan 2
Servo myservo3; // servo pakan 3
Servo myservo4; // servo pakan 4
Servo srvPasokAir; // servo pasok air
Servo srvKurasAir; // servo kuras air

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
  srvKurasAir.attach(srvPinKuras);
  
  pinMode(WLvlAtas, INPUT_PULLUP);
  pinMode(WLvlBawah, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  
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
  delay(500);
  myservo1.write(2);
  myservo2.write(2);
  myservo3.write(2);
  myservo4.write(2);
  srvKurasAir.write(2);
  delay(2000);
  srvKurasAir.detach();
  
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
    lcd.print(HariKe);
    lcd.print(" Pakan: ");
    lcd.print(RasioPakan*HariKe*JumlahLele); 
    lcd.print(" Gram");
    Serial.print("Hari ke: ");
    Serial.print((HariKe));
    Serial.println(" Sejak 1/12/16(Tebar Pertama)");
    Serial.print("Rasio Pakan Harus Diberi : ");
    Serial.print((RasioPakan*HariKe*JumlahLele)); 
    Serial.println(" Gram");
    Serial.println();
    
    for(int i = 0; i < 16; i++){
      lcd.scrollDisplayLeft();
      delay(500);
    }
    lcd.clear();
    delay(1000);
    
    WLvlAtas = digitalRead(WLvlAtas);
    WLvlBawah = digitalRead(WLvlBawah);
    
    // Sensor pH
    for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
    { 
      buf[i]=analogRead(PHSensorPin);
      delay(10);
    }
    for(int i=0;i<9;i++)        //sort the analog from small to large
    {
      for(int j=i+1;j<10;j++)
      {
        if(buf[i]>buf[j])
        {
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
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("pH : ");   
    lcd.println(phValue); 
    Serial.print("pH:");  
    Serial.println(phValue);
    
    //Ambil data suhu dan tampilkan
    lcd.setCursor(0,1);
    suhuSekarang = ambilSuhu();
    lcd.print("Suhu : ");   
    lcd.println(suhuSekarang); 
    Serial.print("Suhu : ");   
    Serial.println(suhuSekarang); 
    delay(2000);
    lcd.clear();
    
    //Pengaturan waktu 09:00 pemberian pakan 
    if((now.hour() == 13 && now.minute() == 29 ))
    {
     lcd.clear();
     lcd.setCursor(0,0);
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.write(90); //buka katup pakan
        delay(600); //delay sebanyak (HariKe*1s)
        lcd.print("  WAKTU PAKAN 1  ");
        Serial.println("++[ WAKTU PAKAN 1 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(15);
        }
        myservo2.write(90); //buka katup pakan
        delay(600); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(15);
        }
          
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.write(90); //buka katup pakan
        delay(500); //delay sebanyak (HariKe*1s)
        lcd.print("  WAKTU PAKAN 1  ");
        Serial.println("++[ WAKTU PAKAN 1 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(15);
        }
        myservo4.write(90); //buka katup pakan
        delay(500); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(15);
        }
          
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } 
      lcd.clear();
    } 
 
    //Pengaturan waktu 12:00 pemberian pakan 
    if((now.hour() == 13 && now.minute() == 30 ))
    {
      lcd.clear();
      lcd.setCursor(0,0);
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.write(90); //buka katup pakan
        delay(400); //delay sebanyak (HariKe*1s)
        lcd.print("  WAKTU PAKAN 2 ");
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(15);
        }
        myservo2.write(90); //buka katup pakan
        delay(400); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(15);
        }
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.write(90); //buka katup pakan
        delay(300); //delay sebanyak (HariKe*1s)
        lcd.print("  WAKTU PAKAN 2  ");
        Serial.println("++[ WAKTU PAKAN 2 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(15);
        }
        myservo4.write(90); //buka katup pakan
        delay(300); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(15);
        }
          
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      }
      lcd.clear();
    }

     //Pengaturan waktu 17:00 pemberian pakan 
     if((now.hour() == 13 && now.minute() == 31))
     {
      lcd.clear();
      lcd.setCursor(0,0);
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.write(90); //buka katup pakan
        delay(200); //delay sebanyak (HariKe*1s)
        lcd.print("  WAKTU PAKAN 3  ");
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(15);
        }
        myservo2.write(90); //buka katup pakan
        delay(200); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(15);
        }
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        lcd.setCursor(0,0);
        myservo3.write(90); //buka katup pakan
        delay(100); //delay sebanyak (HariKe*1s)
        lcd.print("  WAKTU PAKAN 3  ");
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(15);
        }
        myservo4.write(90); //buka katup pakan
        delay(100); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(15);
        }
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      }
      lcd.clear();
    }

    delay(2000);
    //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
 //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
    if(suhuSekarang > 30 || phValue > 9 || phValue < 5){
      Serial.println("++[ AIR DIKURAS ]++");
      srvKurasAir.attach(srvPinKuras);
      delay(1000);
      srvKurasAir.write(90); //buka katup pakan
      delay(1000);
      srvKurasAir.detach();
      
    } else if (NilaiWLvlAtas == HIGH && NilaiWLvlBawah == HIGH){
        Serial.println("++[ AIR DIPASOK]++");
        srvKurasAir.write(0); //buka katup pakan
        delay(2000);
        srvKurasAir.detach();
        digitalWrite(LED_BUILTIN, HIGH);  
     }
     else if (NilaiWLvlAtas == LOW && NilaiWLvlBawah == LOW){
          digitalWrite(LED_BUILTIN, LOW);     
     }
    

}

float ambilSuhu()
{
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0)+1;
   return suhu;   
}
