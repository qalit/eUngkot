#include "Servo.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
 
// setup sensor
OneWire oneWire(ONE_WIRE_BUS);
 
// berikan nama variabel,masukkan ke pustaka Dallas
DallasTemperature sensorSuhu(&oneWire);
 
float suhuSekarang;

int WLvlAtas = A2;
int WLvlBawah = A3;
int srvPinPasok = 13;
int srvPinKuras = 12;
Servo srvPasokAir;
Servo srvKurasAir;
int srvAngle1 = 0;
int srvAngle2 = 0;

int NilaiWLvlAtas = LOW;
int NilaiWLvlBawah = LOW;

void setup(){
  Serial.begin(9600);
  pinMode(WLvlAtas, INPUT);
  pinMode(WLvlBawah, INPUT);
  srvPasokAir.attach(srvPinPasok);
  srvKurasAir.attach(srvPinKuras);
  sensorSuhu.begin();
}

void loop(){
  
  NilaiWLvlAtas = analogRead(WLvlAtas);
  NilaiWLvlBawah = analogRead(WLvlBawah);
  delay(2000);
 
  if(ambilSuhu() >= 30){
    Serial.println("++[ AIR DIKURAS ]++");
    srvKurasAir.write(180); //buka katup pakan
    srvPasokAir.write(1);
  }
  
  if (NilaiWLvlBawah == HIGH && NilaiWLvlAtas == LOW){
      Serial.println("++[ AIR DIPASOK]++");
      srvKurasAir.write(1);
      srvPasokAir.write(180);
  }
  if (NilaiWLvlAtas == HIGH && NilaiWLvlBawah == HIGH){
    srvKurasAir.write(0);
    srvPasokAir.write(0);
  }

  Serial.println(srvAngle1);
  Serial.println(srvAngle2);
  Serial.println(NilaiWLvlAtas);
  Serial.println(NilaiWLvlBawah);
  suhuSekarang = ambilSuhu();
  Serial.print("Suhu : ");   
  Serial.println(suhuSekarang); 
  srvKurasAir.write(1);
  srvPasokAir.write(1);
  Serial.println("--------");
 }

 float ambilSuhu()
{
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   
   delay(7000);
}
