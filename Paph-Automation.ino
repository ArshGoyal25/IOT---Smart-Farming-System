//Paph and Phal automation
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <dht.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

dht DHT;
#define DHT22_PIN 13
#define fogger_Relay  11
#define fan_Relay 10
DateTime dt;

//Variables
int current_Temp = 0;
int current_Humi = 0;
bool isFanON = false;
bool isHighTemp = false;

//Settings
int display_count = 0;
int temp_sampling_interval = 30; //temp and humidity sampling interval Secs
int fan_Time = 15 ; //minutes for every 30 mins
int fogging_Time = 20; //secs for predefined slots
int temperature_limit = 37;
//int onTime[] = {8, 9, 10, 16, 17, 18};
int fan_Start = 8; // 8AM morning
int fan_End = 18; //6PM evening

void setup() {
  Serial.begin(9600);
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower())
  {
    //Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


  //setTime(dt.hour(), dt.minute(), dt.second(), dt.day(), dt.month(), tmYearToCalendar(dt.year()));

  pinMode(fogger_Relay, OUTPUT);
  digitalWrite(fogger_Relay, HIGH); // reversing based on relay

  pinMode(fan_Relay, OUTPUT);
  digitalWrite(fan_Relay, HIGH); // reversing based on relay

  // initialize the LCD
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.noBlink();
  lcd.print("Welcome Nagesh!");
  lcd.setCursor(0, 1);
  lcd.print("Automation V1.0");
  delay(3000);
  lcd.clear();
}

int displayLoop = 0;

void loop() {

  dt = rtc.now();
  ReadDHT22();
  RunFogger();
  RunFan();
  DisplayTempHumi();
  DisplayTimeStamp();
  delay(1000);
}

//Reading DHT22 sensor
void ReadDHT22()
{
  if (display_count > temp_sampling_interval || display_count == 0)
  {
    int chk = DHT.read22(DHT22_PIN);
    //    Serial.print("DHT reading -");
    //    Serial.println(chk);
    if (chk >= 0)
    {
      current_Temp = DHT.temperature;
      current_Humi = DHT.humidity;
      display_count = 1;
      MonitorTempHumi();
    }
  }
  display_count++;
}

void DisplayTempHumi()
{
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" T~");
  lcd.print(current_Temp);
  lcd.print("c * ");

  //lcd.setCursor(0, 1);
  lcd.print("H~");
  lcd.print(current_Humi);
  lcd.print("%");
}

void DisplayTimeStamp()
{
  char buf1[20];
  char buf2[20];
  //DateTime now = rtc.now();
  char mon[4];
  switch (dt.month())
  {
    case 1:
      strcpy(mon, "Jan");
      break;
    case 2:
      strcpy(mon, "Feb");
      break;
    case 3:
      strcpy(mon, "Mar");
      break;
    case 4:
      strcpy(mon, "Apr");
      break;
    case 5:
      strcpy(mon, "May");
      break;
    case 6:
      strcpy(mon, "Jun");
      break;
    case 7:
      strcpy(mon, "Jly");
      break;
    case 8:
      strcpy(mon, "Aug");
      break;
    case 9:
      strcpy(mon, "Sep");
      break;
    case 10:
      strcpy(mon, "Oct");
      break;
    case 11:
      strcpy(mon, "Nov");
      break;
    case 12:
      strcpy(mon, "Dec");
      break;
  }
  sprintf(buf1, "%02d:%02d:%02d%02s%02d%01s%03s%",  dt.hour(), dt.minute(), dt.second(), "  ", dt.day(), "-", mon);
  //sprintf(buf2, "%02d/%02d",  );
  //lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(buf1);
  //lcd.setCursor(0, 1);
  //lcd.print(buf2);
}

void RunFogger()
{

  switch (dt.hour())
  {
    case 8:
    case 9:
    case 10:
    case 12:
    case 14:
    case 16:
    case 17:
      if (dt.minute() == 0 && dt.second() >= 0 &&  dt.second() <= fogging_Time)
      {
        //Serial.println("Fogger on");
        FoggerONOFF(true);
      }
      else
      {
        //Serial.println("Fogger on else case");
        FoggerONOFF(false);
      }
      break;
  }
}

void RunFan()
{
  if (dt.hour() >= fan_Start && dt.hour() < fan_End)
  {
    if ((dt.minute() >= 0 && dt.minute() < fan_Time) || (dt.minute() >= 30 && dt.minute() < 30 + fan_Time))
    {
      //Serial.println("Fan on");
      isFanON = true;
      FanONOFF(true);
    }
    else
    {
      if (!isHighTemp)
      {
        //Serial.println("Fan on else");
        isFanON = false;
        FanONOFF(false);
      }
    }
  }
  else
  {
    FanONOFF(false);
  }
}

void FoggerONOFF(bool state)
{
  if (state)
    digitalWrite(fogger_Relay, LOW);
  else
    digitalWrite(fogger_Relay, HIGH);
}

void FanONOFF(bool state)
{
  if (state)
    digitalWrite(fan_Relay, LOW);
  else
    digitalWrite(fan_Relay, HIGH);
}

//Triggering the fogger for low humidity or high temp
void MonitorTempHumi()
{
  if (current_Temp > temperature_limit && !isFanON)
  {
    //Serial.println("Temp high");
    isHighTemp = true;
    FoggerONOFF(true);
  }
  else
  {
    //Serial.println("Temp High else");
    isHighTemp = false;
  }
}
