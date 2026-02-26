#include "RTClib.h"
#include <Preferences.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define DISPLAY_RX 16
#define DISPLAY_TX 17
#define LED 2
#define SENSOR 15  // 3000 dark, 300 direct light
#define OUT1 18
#define OUT2 19

#define ACTION_SET_ALM "alm"
#define ACTION_SET_CURRENT "set"
#define ACTION_SET_MODE "raise"

RTC_DS3231 rtc;
HardwareSerial displaySerial(2);
Preferences prefs;
int raiseMode = 0;      //0:Off, 1:Light, 2:time (down with darkness)
int savedAlarmTime[4];  // hh mm of the alarm

double Input;
boolean statusOpen = false;
int darkThreshold = 1000;  // above this, there is dark
int lightThreshold = 400;  // below this, light

#define updateTXT(a, v) \
  displaySerial.print(a); \
  displaySerial.print(".txt=\""); \
  displaySerial.print(v); \
  displaySerial.print(F("\"")); \
  displaySerial.print(F("\xFF\xFF\xFF"))

#define updateVAL(a, v) \
  displaySerial.print(a); \
  displaySerial.print(".val="); \
  displaySerial.print(v); \
  displaySerial.print(F("\xFF\xFF\xFF"))

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
String receivedData = "";


void setup() {

  Serial.begin(115200);

  displaySerial.begin(115200, SERIAL_8N1, DISPLAY_RX, DISPLAY_TX);

  Wire.begin(I2C_SDA, I2C_SCL);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  prefs.begin("my-app", false);
  size_t savedLength = prefs.getBytesLength("SavedAlarm");
  if (savedLength < sizeof(savedAlarmTime)) {
    Serial.println("No saved Alarm time");
    int temp[4] = { 0, 7, 0, 0 };
    prefs.putBytes("SavedAlarm", temp, sizeof(savedAlarmTime));
  }
  prefs.getBytes("SavedAlarm", &savedAlarmTime, sizeof(savedAlarmTime));
  raiseMode = prefs.getInt("SavedMode", 0);
  prefs.end();

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));




  pinMode(LED, OUTPUT);
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);

  Input = analogRead(SENSOR);

  if (Input > darkThreshold)
  {
    /*digitalWrite(OUT1, HIGH);
    delay(500);
    digitalWrite(OUT1, LOW);*/
    statusOpen = false;
  }
  else
  {
    /*digitalWrite(OUT2, HIGH);
    delay(500);
    digitalWrite(OUT2, LOW);*/
    statusOpen = true;
  }

   digitalWrite(LED, LOW);
}




void loop() {


  while (displaySerial.available()) {

    //Serial.print( "0x");
    int inChar = displaySerial.read();
    if (inChar >= 0x20 && inChar <= 0x7E) {
      receivedData += char(inChar);
    } /*
    //Serial.print( inChar, HEX);
    Serial.print( ", ");*/
  }

  if (receivedData.length() > 4) {
    Serial.println(receivedData);
    if (receivedData.indexOf(ACTION_SET_ALM) >= 0) {
      String tmp = receivedData.substring(4);
      int hh = receivedData.substring(4, 6).toInt();
      int mm = receivedData.substring(7).toInt();
      savedAlarmTime[0] = hh / 10;
      savedAlarmTime[1] = hh % 10;
      savedAlarmTime[2] = mm / 10;
      savedAlarmTime[3] = mm % 10;
      prefs.begin("my-app", false);
      prefs.putBytes("SavedAlarm", savedAlarmTime, sizeof(savedAlarmTime));
      prefs.end();

    } else if (receivedData.indexOf(ACTION_SET_CURRENT) >= 0) {

      String tmp = receivedData.substring(4);
      int hh = receivedData.substring(4, 6).toInt();
      int mm = receivedData.substring(7).toInt();
      DateTime now = rtc.now();
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), hh, mm, 0));
      /*
      Serial.print("Setting new current time:");
      Serial.print(hh);
      Serial.print(":");
      Serial.print(mm);
      Serial.println();
      */

    } else if (receivedData.indexOf(ACTION_SET_MODE) >= 0) {
      //Serial.println("Serring new mode");
      if (receivedData.indexOf("time") >= 0)
        raiseMode = 2;
      else if (receivedData.indexOf("light") >= 0)
        raiseMode = 1;
      else if (receivedData.indexOf("no") >= 0)
        raiseMode = 0;

      prefs.begin("my-app", false);
      prefs.putInt("SavedMode", raiseMode);
      prefs.end();
    }
  }

  receivedData = "";



  // Get the current time from the RTC
  DateTime now = rtc.now();
  // Getting each time field in individual variables
  // And adding a leading zero when needed;
  String yearStr = String(now.year(), DEC);
  String monthStr = (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
  String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC);
  String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC);
  String minuteStr = (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
  String secondStr = (now.second() < 10 ? "0" : "") + String(now.second(), DEC);
  String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];

  // Complete time string
  //String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;
  String actualTime = hourStr + ":" + minuteStr + ":" + secondStr;
  String actualDate = dayOfWeek + ", " + dayStr + "." + monthStr + "." + yearStr;

  String alarmTime = String(savedAlarmTime[0], DEC) + String(savedAlarmTime[1], DEC) + ":" + String(savedAlarmTime[2], DEC) + String(savedAlarmTime[3], DEC);
  String actualTimeForCmp = hourStr + ":" + minuteStr;
  //Serial.println(alarmTime);

  // Print the complete formatted time
  //Serial.println(formattedTime);

  updateTXT("currentTime", actualTime);
  updateTXT("currentDate", actualDate);
  updateTXT("currentAlarm", alarmTime);

  updateVAL("checkAlarm", raiseMode == 2 ? 1 : 0);
  updateVAL("checkLight", raiseMode == 1 ? 1 : 0);
  updateVAL("checkDisable", raiseMode == 0 ? 1 : 0);

  // Getting temperature
  /*Serial.print(rtc.getTemperature());
  Serial.println("ºC");*/

  //Serial.println();

  Input = analogRead(SENSOR);
  Serial.println(Input);


  if (raiseMode != 0) {
    if (statusOpen && Input > darkThreshold) {
      digitalWrite(LED, HIGH);
      bool toggle = true;
      for (int i = 0; i < 3; i++) {
        delay(500);
        Input = analogRead(SENSOR);
        delay(500);
        if (Input < darkThreshold) {
          toggle = false;
          break;
        }
      }
      digitalWrite(LED, LOW);
      if (toggle) {
        digitalWrite(OUT1, HIGH);
        delay(500);
        digitalWrite(OUT1, LOW);
        statusOpen = false;
      }
    }

    else if (!statusOpen && Input < lightThreshold && (raiseMode == 1 || (raiseMode == 2 && alarmTime == actualTimeForCmp))) {
      digitalWrite(LED, HIGH);
      bool toggle = true;
      for (int i = 0; i < 3; i++) {
        delay(500);
        Input = analogRead(SENSOR);
        delay(500);
        if (Input > lightThreshold) {
          toggle = false;
          break;
        }
      }
      digitalWrite(LED, LOW);
      if (toggle) {
        digitalWrite(OUT2, HIGH);
        delay(500);
        digitalWrite(OUT2, LOW);
        statusOpen = true;
      }
    }
  }












  delay(1000);
}