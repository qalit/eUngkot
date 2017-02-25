#include <Ethernet.h>
#include <UbidotsEthernet.h>
#include <RTClib.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

//sensor pH pin 0
#define PHSensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

#define ONE_WIRE_BUS 2
#define IDpH "5829db3e76254207ecb42195"
#define IDSuhu "5829c80d76254214e3f9d3a1"
#define IDPakan "5829c8a476254218b9f109e7"
#define TOKEN "iGPgAzahoarBndo3RvDDUfln0z1aDK"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

int WLvlAtas = 6;
int WLvlBawah = 7;
int servoPin1 = 3;
int servoPin2 = 9;
int servoPin3 = 5;
int servoPin4 = 11;
int srvPinKuras = 12;
Servo myservo1;  // servo pakan 1
Servo myservo2;  // servo pakan 2
Servo myservo3; // servo pakan 3
Servo myservo4; // servo pakan 4
Servo srvPasokAir; // servo pasok air
Servo srvKurasAir; // servo kuras air

int servoAngle = 0;

Ubidots client(TOKEN);
unsigned long interval=1000; // the time we need to wait
unsigned long previousMillis=0; // millis() returns an unsigned long.

void setup() {
  //Serial.begin(9600);
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
  
  if (Ethernet.begin(mac) == 0) {
      //Serial.println("Failed to configure Ethernet using DHCP");
      // try to congifure using IP address instead of DHCP:
      Ethernet.begin(mac, ip);
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
  sensorSuhu.begin();
  
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis) >= interval) {
     previousMillis = millis()+4000;
  }
}

void loop() {
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
  //Serial.print("pH:");  
  //Serial.println(phValue);
  
  //Ambil data suhu dan tampilkan
  //Serial.print("Suhu : ");   
  //Serial.println(suhu); 
  delay(2000);
  int NilaiWLvlAtas = digitalRead(WLvlAtas);
  int NilaiWLvlBawah = digitalRead(WLvlBawah);

 //Jika suhu diatas 30 & pH diatas 9 servo kuras hidup
  if(suhu >= 30 || phValue >= 9 || phValue <= 5){
    //Serial.println("++[ AIR DIKURAS ]++");
    srvKurasAir.attach(srvPinKuras);
    srvKurasAir.write(90); //buka katup pakan
    delay(2000);
    srvKurasAir.detach();
  }
  if (NilaiWLvlAtas == LOW && NilaiWLvlBawah == LOW){
      //Serial.println("++[ AIR DIPASOK]++");
      srvKurasAir.attach(srvPinKuras);
      srvKurasAir.write(0); //buka katup pakan
      delay(2000);
      srvKurasAir.detach();
      digitalWrite(LED_BUILTIN, HIGH);  
  }
  if (NilaiWLvlAtas == HIGH && NilaiWLvlBawah == HIGH){
      digitalWrite(LED_BUILTIN, LOW);     
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
  //Serial.print("Hari ke: ");
  //Serial.print((HariKe));
  //Serial.println(" Sejak 1/12/16(Tebar Pertama)");
  //Serial.print("Rasio Pakan Harus Diberi : ");
  //Serial.print((RasioPakan*HariKe*JumlahLele)); 
  //Serial.println(" Gram");
  //Serial.println();
  
  //Tampilkan Waktu sekarang
  //Serial.print(now.year(), DEC);
  //Serial.print('/');
  //Serial.print(now.month(), DEC);
  //Serial.print('/');
  //Serial.print(now.day(), DEC);
  //Serial.print(" (");
  //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  //Serial.print(") ");
  //Serial.print("| ");
  //Serial.print(now.hour(), DEC);
  //Serial.print(':');
  //Serial.print(now.minute(), DEC);
  //Serial.print(':');
  //Serial.print(now.second(), DEC);
  //Serial.println();
   
  //Serial.println("<========================>");
    
  //Pengaturan waktu 09:00 pemberian pakan 
  if((now.hour() == 14 && now.minute() == 39 ))
  {
    if(HariKe >= 1 && HariKe <= 28){
      myservo1.attach(servoPin1);
      myservo1.write(90); //buka katup pakan
      delay(600); //delay sebanyak (HariKe*1s)
      //Serial.println("++[ WAKTU PAKAN 1 ]++");
      
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
      //Serial.print("Pakan diberi sebanyak : ");
      //Serial.print(RasioPakan*HariKe*JumlahLele);
      //Serial.println("Gr");
      //Serial.println(" ");
      } 
      else if(HariKe >= 29 && HariKe <= 50){
        myservo3.attach(servoPin3);
        myservo3.write(90); //buka katup pakan
        delay(500); //delay sebanyak (HariKe*1s)
        //Serial.println("++[ WAKTU PAKAN 1 ]++");
        
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
          //Serial.print("Pakan diberi sebanyak : ");
          //Serial.print(RasioPakan*HariKe*JumlahLele);
          //Serial.println("Gr");
          //Serial.println(" ");
      } 
    } 
 
    //Pengaturan waktu 12:00 pemberian pakan 
    if((now.hour() == 14 && now.minute() == 59 ))
    {
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.attach(servoPin1);
        myservo1.write(90); //buka katup pakan
        delay(400); //delay sebanyak (HariKe*1s)
        //Serial.println("++[ WAKTU PAKAN 2 ]++");
        
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
        //Serial.print("Pakan diberi sebanyak : ");
        //Serial.print(RasioPakan*HariKe*JumlahLele);
        //Serial.println("Gr");
        //Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.attach(servoPin3);
        myservo3.write(90); //buka katup pakan
        delay(300); //delay sebanyak (HariKe*1s)
        //Serial.println("++[ WAKTU PAKAN 2 ]++");
        
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
        //Serial.print("Pakan diberi sebanyak : ");
        //Serial.print(RasioPakan*HariKe*JumlahLele);
        //Serial.println("Gr");
        //Serial.println(" ");
        }
      }

     //Pengaturan waktu 17:00 pemberian pakan 
     if((now.hour() == 22 && now.minute() == 47))
     {
      if(HariKe >= 1 && HariKe <= 28){
        myservo1.attach(servoPin1);
        myservo1.write(90); //buka katup pakan
        delay(200); //delay sebanyak (HariKe*1s)
        //Serial.println("++[ WAKTU PAKAN 3 ]++");
        
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
        //Serial.print("Pakan diberi sebanyak : ");
        //Serial.print(RasioPakan*HariKe*JumlahLele);
        //Serial.println("Gr");
        //Serial.println(" ");
      } else if(HariKe >= 29 && HariKe <= 50){
        myservo3.attach(servoPin3);
        myservo3.write(90); //buka katup pakan
        delay(100); //delay sebanyak (HariKe*1s)
        //Serial.println("++[ WAKTU PAKAN 3 ]++");
        
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
        //Serial.print("Pakan diberi sebanyak : ");
        //Serial.print(RasioPakan*HariKe*JumlahLele);
        //Serial.println("Gr");
        //Serial.println(" ");
      }
    }
 
  client.add(IDSuhu, suhuSekarang);
  client.add(IDpH, phValue);
  client.sendAll();
  delay(55000);
  return;
}


