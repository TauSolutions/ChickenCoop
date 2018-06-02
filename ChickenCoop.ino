#include <Wire.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <Button.h>

#define MORNING_ON    05
#define MORNING_OFF   10
#define AFTERNOON_ON  14
#define EVENING_OFF   21

#define MAIN_COOP_LIGHT  5     //  Relay 1
#define OUTSIDE_LIGHT    4     //  Relay 2
//#define SOMEOTHER_LIGHT  3     //  Relay 3
//#define ANOTHER_LIGHT    2     //  Relay 4

#define LIGHT_SENSOR          A0
#define LIGHT_THRESHOLD_HIGH  850
#define LIGHT_THRESHOLD_LOW   800

byte RTCupdate = 0;
byte CoopAutoLight = 0;
byte suspendAuto = 0;
word lastLightChange = -1;

Button MainCoop_Switch = Button(9, BUTTON_PULLUP_INTERNAL, true, 50);
Button Outside_Switch = Button(8, BUTTON_PULLUP_INTERNAL, true, 50);
//  Next Switch     7
//  Last Switch     6


void setup() {
  time_t time = RTC.get();
  if(time) setTime(time);
  
//  Serial.begin(38400);
  
  LightSetup(MAIN_COOP_LIGHT);
  LightSetup(OUTSIDE_LIGHT);
  
//  Serial.println("Program Started...");
  Alarm.alarmRepeat(MORNING_ON,00,00,AutoLightOn);
  Alarm.alarmRepeat(MORNING_OFF,00,00,AutoLightOff);
  Alarm.alarmRepeat(AFTERNOON_ON,00,00,AutoLightOn);
  Alarm.alarmRepeat(EVENING_OFF,00,00,AutoLightOff);

//timerOn();
//Alarm.timerOnce(60,timerOff);

  int dayMinutes = (hour()*60) + minute();
//  Serial.println(dayMinutes);
  if ((dayMinutes >= (MORNING_ON * 60)) && (dayMinutes < (MORNING_OFF * 60))) {
    CoopAutoLight = 1;
  } else if ((dayMinutes >= (AFTERNOON_ON * 60)) && (dayMinutes < (EVENING_OFF * 60))) {
    CoopAutoLight = 1;
  } else {
    CoopAutoLight = 0;
  }
//AutoLightOn();
}

/*
void timerOn() {
  Alarm.timerRepeat(120, AutoLightOn);
}

void timerOff() {
  Alarm.timerRepeat(120, AutoLightOff);
}
*/

void loop() {
  if(RTCupdate != hour()) { time_t time=RTC.get(); if(time) setTime(time);  RTCupdate = hour(); }
  byte r = digitalRead(MAIN_COOP_LIGHT);
  byte s = MainCoop_Switch.stateChanged();
  int lux = analogRead(LIGHT_SENSOR);
  
  //  Main Coop Light
  if (CoopAutoLight) {
    //  get Ambient Light Value
    //  if light below 100lux turn light on
    //  if light above 150lux turn light off
    if (suspendAuto) {
      if (s) {
//Serial.println("Resume Auto...");
        suspendAuto = false;
        LightToggle(MAIN_COOP_LIGHT);
      }
    } else {
      if (s) {
//Serial.println("Suspend Auto...");
        suspendAuto = true;
        LightToggle(MAIN_COOP_LIGHT);
      } else {
//Serial.println(lux);
        if (r) {
          if (millis() - lastLightChange > 500)
            if (lux < LIGHT_THRESHOLD_LOW) LightOn(MAIN_COOP_LIGHT);
        } else {
          if (millis() - lastLightChange > 500)
            if (lux > LIGHT_THRESHOLD_HIGH) LightOff(MAIN_COOP_LIGHT);
        }
      }
    }
  } else {
    if (s) LightToggle(MAIN_COOP_LIGHT);
  }

  //  North Light
  if (Outside_Switch.stateChanged()) LightToggle(OUTSIDE_LIGHT);
  
  Alarm.delay(0);
}

void LightSetup (int pin) {
  digitalWrite(pin, HIGH);
  pinMode(pin, OUTPUT);
}

void LightOn (int pin) {
  digitalWrite(pin, LOW);
  lastLightChange = millis();
//  Serial.print("Light ");
//  Serial.print(pin);
//  Serial.println(" On");
}

void LightOff (int pin) {
  digitalWrite(pin, HIGH);
  lastLightChange = millis();
//  Serial.print("Light ");
//  Serial.print(pin);
//  Serial.println(" Off");  
}

void LightToggle (int pin) {
  byte r = digitalRead(pin);
//  Serial.print("Toggle Light ");
//  Serial.print(pin);
  if (r) {
//    Serial.println(" from Off");
    LightOn(pin);
  } else {
//    Serial.println(" from On");
    LightOff(pin);
  }
}

void AutoLightOn() {
//  Serial.println("Auto Light On...");
  CoopAutoLight = 1;
//  if(suspendAuto) Serial.println("Resume Auto...");
  suspendAuto = 0;
  LightOn(MAIN_COOP_LIGHT);
}

void AutoLightOff() {
//  Serial.println("Auto Light Off...");
  CoopAutoLight = 0;
//  if(suspendAuto) Serial.println("Resume Auto...");
  suspendAuto = 0;
  LightOff(MAIN_COOP_LIGHT);
}
