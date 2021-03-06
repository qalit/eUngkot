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

//sensor pH pin 0
#define PHSensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

// sensor suhu diletakkan di pin 2
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

int WLvlAtas = A1;
int WLvlBawah = A2;
int servoPin1 = 5;
int servoPin2 = 6;
int servoPin3 = 7;
int servoPin4 = 8;
int srvPinKuras = 9;
Servo myservo1;  // servo pakan 1
Servo myservo2;  // servo pakan 2
Servo myservo3; // servo pakan 3
Servo myservo4; // servo pakan 4
Servo srvPasokAir; // servo pasok air
Servo srvKurasAir; // servo kuras air

int servoAngle = 0;

void setup () {
  Serial.begin(9600);
  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);
  myservo3.attach(servoPin3);
  myservo4.attach(servoPin4);
  srvKurasAir.attach(srvPinKuras);
  delay(1000);
  myservo1.write(0);
  myservo2.write(0);
  myservo3.write(0);
  myservo4.write(0);
  srvKurasAir.write(0);
  delay(1000);
  myservo1.detach();
  myservo2.detach();
  myservo3.detach();
  myservo4.detach();
  srvKurasAir.detach();
  
  pinMode(WLvlAtas, INPUT_PULLUP);
  pinMode(WLvlBawah, INPUT_PULLUP);
  
  rtc.begin();
 

  sensorSuhu.begin();
  
}

void loop () {
  sensorSuhu.requestTemperatures();
  float suhu = sensorSuhu.getTempCByIndex(0);
  delay(100);
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
  Serial.print("pH:");  
  Serial.println(phValue);
  
  //Ambil data suhu dan tampilkan
  Serial.print("Suhu : ");   
  Serial.println(suhu); 
  delay(2000);
  int NilaiWLvlAtas = analogRead(WLvlAtas);
  int NilaiWLvlBawah = analogRead(WLvlBawah);

 //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
  if(suhu >= 30 || phValue >= 9 || phValue <= 5){
    Serial.println("++[ AIR DIKURAS ]++");
    srvKurasAir.attach(srvPinKuras);
    srvKurasAir.write(90); //buka katup pakan
    delay(2000);
    srvKurasAir.detach();
  }
  if (NilaiWLvlAtas >= 1000 && NilaiWLvlBawah >= 1000){
      Serial.println("++[ AIR DIPASOK]++");
      srvKurasAir.attach(srvPinKuras);
      srvKurasAir.write(0); //buka katup pakan
      delay(2000);
      srvKurasAir.detach();
      analogWrite(A3, 255);  
  }
  if (NilaiWLvlAtas <= 1000 && NilaiWLvlBawah <= 1000){
      analogWrite(A3, 0);      
  }
    
  //Bagian deklarasi waktu sekarang dan waktu sejak tebar pertama
  DateTime now = rtc.now();
  DateTime HariPertama = DateTime(__DATE__,__TIME__);
  
  //TGL hari pertama & jumlah lele
  DateTime HariTebar (2017, 2, 1, 0, 0, 0);
  int JumlahLele = 100;
  int HariKe = ((HariPertama.secondstime() - HariTebar.secondstime())/86400);
  float RasioPakan = 0.0039;
   
  //hitung hari sejak program di upload
  Serial.print("Hari ke: ");
  Serial.print((HariKe));
  Serial.println(" Sejak 1/12/16(Tebar Pertama)");
  Serial.print("Rasio Pakan Harus Diberi : ");
  Serial.print((RasioPakan*HariKe*JumlahLele)); 
  Serial.println(" Gram");
  Serial.println();
  
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
   
  Serial.println("<========================>");
    
  //Pengaturan waktu 09:00 pemberian pakan 
  if((now.hour() == 12 && now.minute() == 10 ))
  {
    if(HariKe >= 1 && HariKe <= 28){
      myservo1.attach(servoPin1);
      myservo1.write(90); //buka katup pakan
      delay(600); //delay sebanyak (HariKe*1s)
      Serial.println("++[ WAKTU PAKAN 1 ]++");
      
      for (servoAngle = 90; servoAngle > 0; servoAngle--) 
      { // servo kembali ke posisi 0 menutup kotak pakan
        myservo1.write(servoAngle);          
        delay(15);
      }
      myservo2.attach(servoPin2);
      myservo2.write(90); //buka katup pakan
      delay(600); //delay sebanyak (HariKe*1s)
      
      for (servoAngle = 90; servoAngle > 0; servoAngle--) 
      { // servo kembali ke posisi 0 menutup kotak pakan
        myservo2.write(servoAngle);          
        delay(15);
      }
      myservo1.detach();
      myservo2.detach();
      Serial.print("Pakan diberi sebanyak : ");
      Serial.print(RasioPakan*HariKe*JumlahLele);
      Serial.println("Gr");
      Serial.println(" ");
      } 
      else if(HariKe >= 29 && HariKe <= 50){
        myservo3.attach(servoPin3);
        myservo3.write(90); //buka katup pakan
        delay(500); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 1 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(15);
        }
        myservo4.attach(servoPin4);
        myservo4.write(90); //buka katup pakan
        delay(500); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(15);
        }
          myservo3.detach();
          myservo4.detach();
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } 
    } 
 
    //Pengaturan waktu 12:00 pemberian pakan 
    if((now.hour() == 14 && now.minute() == 59 ))
    {
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.attach(servoPin1);
        myservo1.write(90); //buka katup pakan
        delay(400); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 2 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(15);
        }
        myservo2.attach(servoPin2);
        myservo2.write(90); //buka katup pakan
        delay(400); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(15);
        }
        myservo1.detach();
        myservo2.detach();
        Serial.print("Pakan diberi sebanyak : ");
        Serial.print(RasioPakan*HariKe*JumlahLele);
        Serial.println("Gr");
        Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.attach(servoPin3);
        myservo3.write(90); //buka katup pakan
        delay(300); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 2 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(15);
        }
        myservo4.attach(servoPin4);
        myservo4.write(90); //buka katup pakan
        delay(300); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(15);
        }
        myservo3.detach();
        myservo4.detach();
        Serial.print("Pakan diberi sebanyak : ");
        Serial.print(RasioPakan*HariKe*JumlahLele);
        Serial.println("Gr");
        Serial.println(" ");
        }
      }

     //Pengaturan waktu 17:00 pemberian pakan 
     if((now.hour() == 22 && now.minute() == 47))
     {
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.attach(servoPin1);
        myservo1.write(90); //buka katup pakan
        delay(200); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo1.write(servoAngle);          
          delay(15);
        }
        myservo2.attach(servoPin2);
        myservo2.write(90); //buka katup pakan
        delay(200); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo2.write(servoAngle);          
          delay(15);
        }
        myservo3.detach();
        myservo4.detach();
        Serial.print("Pakan diberi sebanyak : ");
        Serial.print(RasioPakan*HariKe*JumlahLele);
        Serial.println("Gr");
        Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.attach(servoPin3);
        myservo3.write(90); //buka katup pakan
        delay(100); //delay sebanyak (HariKe*1s)
        Serial.println("++[ WAKTU PAKAN 3 ]++");
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo3.write(servoAngle);          
          delay(15);
        }
        myservo4.attach(servoPin4);
        myservo4.write(90); //buka katup pakan
        delay(100); //delay sebanyak (HariKe*1s)
        
        for (servoAngle = 90; servoAngle > 0; servoAngle--) 
        { // servo kembali ke posisi 0 menutup kotak pakan
          myservo4.write(servoAngle);          
          delay(15);
        }
        myservo3.detach();
        myservo4.detach();
        Serial.print("Pakan diberi sebanyak : ");
        Serial.print(RasioPakan*HariKe*JumlahLele);
        Serial.println("Gr");
        Serial.println(" ");
      }
    }
    delay(2000);

}
