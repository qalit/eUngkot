#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
RTC_DS1307 RTC;

LiquidCrystal_I2C lcd(0x27,16,2);

void setup () {
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();
  if (!RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  lcd.init();
  lcd.backlight();
}
void loop () {
    DateTime now = RTC.now(); 
    lcd.setCursor(0,0);
    lcd.print(now.year(), DEC);
    Serial.print(now.year(), DEC);
    lcd.print("/");
    Serial.print('/');
    lcd.print(now.month(), DEC);
    Serial.print(now.month(), DEC);
    lcd.print('/');
    Serial.print('/');
    lcd.print(now.day(), DEC);
    Serial.print(now.day(), DEC);
    lcd.print(' ');
    Serial.print(' ');
    lcd.print(now.hour(), DEC);
    Serial.print(now.hour(), DEC);
    lcd.print(':');
    Serial.print(':');
    lcd.print(now.minute(), DEC);
    Serial.print(now.minute(), DEC);
    lcd.print(':');
    Serial.print(':');
    lcd.print(now.second(), DEC);
    Serial.print(now.second(), DEC);
    Serial.println();
    delay(1000);
}
