  /* eUngkot - Clarias Gariepinus Feeder
Time RTC set servo to move feed 

 circuit:
 *base on fritzing sketch & proteus simulator
 *available : qalit.io/eungkot
 by Al Qalit @ 2016

 This example code is in the public domain.
 http://www.github.com/qalit/eungkot
 */
#include <Ethernet.h>
#include <SPI.h>
#include <UbidotsEthernet.h>
#define IDSuhu "5829c80d76254214e3f9d3a1"
#define IDpH "58a9dd0b762542238e7cf8ae"
#define IDPakan "5829c8a476254218b9f109e7"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

#include <RTClib.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);

#define PHSensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

byte mac[] = { 0x33, 0x33, 0x00, 0x00, 0x00, 0xFB };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

Ubidots client(TOKEN);

unsigned long interval=1000; // the time we need to wait
unsigned long interval1=60000;
unsigned long previousMillis=0; // millis() returns an unsigned long.

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

int WLvlAtas = 3;
int WLvlBawah = 4;
int servoPin1 = 5;
int servoPin2 = 6;
int servoPin3 = 7;
int servoPin4 = 8;
int srvPinKuras = 9;
Servo myservo1;  // servo pakan 1
Servo myservo2;  // servo pakan 2
Servo myservo3; // servo pakan 3
Servo myservo4; // servo pakan 4
Servo srvKurasAir; // servo kuras air

int servoAngle = 0;

void setup () {
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
   
  if (Ethernet.begin(mac) == 0) {
      //Serial.println("Failed to configure Ethernet using DHCP");
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
  }
  pinMode(WLvlAtas, INPUT_PULLUP);
  pinMode(WLvlBawah, INPUT_PULLUP);
  
  if (! rtc.begin()) {
    Serial.println("RTC Mati");
    while (1);
  }

  if (rtc.lostPower()) {
    //rtc.adjust(DateTime(__DATE__,__TIME__));
    Serial.println("RTC lowbat !!");
  
  }
  delay(2000);
}

void loop () {
    unsigned long currentMillis = millis();
  
  for(int i=0;i<10;i++){ 
    buf[i]=analogRead(PHSensorPin);
    delay(10);
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
  for(int i=2;i<8;i++){
    avgValue+=buf[i];
  }
  
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;
  
  sensorSuhu.requestTemperatures();
  float suhu = sensorSuhu.getTempCByIndex(0);
  
  //if ((unsigned long)(currentMillis - previousMillis) >= interval1) {


    int NilaiWLvlAtas = digitalRead(WLvlAtas);
    int NilaiWLvlBawah = digitalRead(WLvlBawah);
        //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
    if(suhu >= 30 || phValue >= 9 || phValue <= 5){
      Serial.println("++[ AIR DIKURAS ]++");
      srvKurasAir.attach(srvPinKuras);
      srvKurasAir.write(90); //buka katup pakan
      delay(2000);
      srvKurasAir.detach();
    }
    if (NilaiWLvlAtas == HIGH && NilaiWLvlBawah == HIGH){
        Serial.println("++[ AIR DIPASOK]++");
        srvKurasAir.attach(srvPinKuras);
        srvKurasAir.write(0); //buka katup pakan
        delay(2000);
        srvKurasAir.detach();
        digitalWrite(LED_BUILTIN, HIGH);  
     }
     if (NilaiWLvlAtas == LOW && NilaiWLvlBawah == LOW){
          digitalWrite(LED_BUILTIN, LOW);     
     }
    Serial.println("<========================>");
    
    //Bagian deklarasi waktu sekarang dan waktu sejak tebar pertama
    DateTime now = rtc.now();
    DateTime HariPertama = DateTime(__DATE__,__TIME__);
    
  //TGL hari pertama & jumlah lele
    DateTime HariTebar (2017, 2, 1, 0, 0, 0);
    int JumlahLele = 100;
    int HariKe = ((HariPertama.secondstime() - HariTebar.secondstime())/86400);
    float RasioPakan = 0.0039;
    float RasioHarian = RasioPakan*HariKe*JumlahLele;
    
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
    
    //Pengaturan waktu 09:00 pemberian pakan 
    if((now.hour() == 12 && now.minute() == 10))
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
      } else if(HariKe >= 29 && HariKe <= 50){
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
          Serial.print("Pakan diberi sebanyak : ");
          Serial.print(RasioPakan*HariKe*JumlahLele);
          Serial.println("Gr");
          Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.write(90); //buka katup pakan
        delay(300); //delay sebanyak (HariKe*1s)
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
    }

     //Pengaturan waktu 17:00 pemberian pakan 
     if((now.hour() == 12 && now.minute() == 58))
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
          myservo1.detach();
          myservo2.detach();
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
      client.add(IDPakan, RasioHarian);
     }
    client.add(IDSuhu, suhu);
    client.add(IDpH, phValue);
    client.sendAll();
    delay(30000);
}    


float ambilSuhu(){
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0)+1;
   return suhu;   
}
