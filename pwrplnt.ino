//pwrplnt.ino

#include "Arduino.h"
#include <SerialCommand.h>

#include <Wire.h> 
#include <EEPROM.h> 
#include <Time.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#include "settings.h"
#include "cpwrplnt.h"

SerialCommand sCmd;
cPwrplnt Pwrplnt;


void setup()
{
    Serial.begin(115200);

    // set up Pin Modes
    pinMode(PIN_MOISTURE, INPUT);
    pinMode(PIN_BRIGHTNESS, INPUT);
    pinMode(PIN_DHT11, INPUT);
    pinMode(PIN_PUMPRELAIS, OUTPUT);
    pinMode(PIN_LIGHTPWM, OUTPUT);

    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet) 
    { Serial.println("Unable to sync with the RTC"); }

    Pwrplnt.init();

    // Setup callbacks for SerialCommand commands
    sCmd.addCommand("help",    showHelp);
    sCmd.addCommand("showStatus", showStatus);
    sCmd.addCommand("showSettings", showSettings);
    sCmd.addCommand("setActive", setActive);
    sCmd.addCommand("setMinMoisture", setMinMoisture);
    sCmd.addCommand("setTargetMoisture", setTargetMoisture);
    sCmd.addCommand("setWateringDuration", setWateringDuration);
    sCmd.addCommand("setWateringPause", setWateringPause);
    sCmd.addCommand("setLightIntensity", setLightIntensity);
    sCmd.addCommand("setSunriseTime", setSunriseTime);
    sCmd.addCommand("setSunsetTime", setSunsetTime);
    sCmd.addCommand("setTime",  setRTCTime);
    sCmd.addDefaultHandler(showHelp);      

    showSettings();
}

void loop()
{
    sCmd.readSerial();

    static bool worked = true;
    // read time and decide to call pwrplnt maintenance
    if (now() % MAINTENANCE_INTERVAL)
    {
        worked = false;
    }
    else if(!worked)
    {
        // work
        Serial.println("working...");
        Pwrplnt.maintain();
        worked = true;
    }
}

void showHelp() {
  Serial.println();
  Serial.println("Available Commands:");
  Serial.println("showStatus");
  Serial.println("showSettings");
  Serial.println("setActive");
  Serial.println("setMinMoisture");
  Serial.println("setTargetMoisture");
  Serial.println("setWateringDuration");
  Serial.println("setWateringPause");
  Serial.println("setLightIntensity");
  Serial.println("setSunriseTime");
  Serial.println("setSunsetTime");
  Serial.println("setTime [year] [mon] [day] [hour] [min] [sec]");
  Serial.println();
}

void showStatus() {
    Serial.println();
    Serial.print(day());
    Serial.print(".");
    Serial.print(month());
    Serial.print(".");
    Serial.println(year());
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());
    Serial.print(" Temperature  (C): ");
    Serial.println(Pwrplnt.getTemperature());
    Serial.print(" Air Humidity (%): ");
    Serial.println(Pwrplnt.getAirHumidity());
    Serial.print(" Moisture     (%): ");
    Serial.println(Pwrplnt.getMoisture());
    Serial.print(" Brightness   (%): ");
    Serial.println(Pwrplnt.getBrightness());
    Serial.print(" Water Level Ok? : ");
    Serial.println(Pwrplnt.getWaterLevelOk());
    Serial.println();
    Serial.print(" Pump State: ");
    Serial.println(Pwrplnt.getPumpState());
    Serial.print(" LightState: ");
    Serial.println(Pwrplnt.getLightState());
}

void showSettings() {
    Serial.println();
    Serial.print(" Active                 : ");
    Serial.println(Pwrplnt.getActive());
    Serial.print(" Minimum Soil Moisture  : ");
    Serial.println(Pwrplnt.getMinMoisture());
    Serial.print(" Targeted Soil Moisture : ");
    Serial.println(Pwrplnt.getTargetMoisture());
    Serial.print(" Watering Duration      : ");
    Serial.println(Pwrplnt.getWateringDuration());
    Serial.print(" Watering Pause         : ");
    Serial.println(Pwrplnt.getWateringPause());
    Serial.print(" Light Intensity        : ");
    Serial.println(Pwrplnt.getLightIntensity());
    Serial.print(" Sunrise Time           : ");
    Serial.println(Pwrplnt.getSunriseTime());
    Serial.print(" Sunset Time            : ");
    Serial.println(Pwrplnt.getSunsetTime());
}

void setActive() {
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    if(0 == atoi(arg))
    {
      Pwrplnt.setActive(false);
      Serial.println("Pwrplnt deactivated");
    }
    else if(1 == atoi(arg))
    {
      Pwrplnt.setActive(true);
      Serial.println("Pwrplnt activated");
    }
  }
  else return;
}

void setMinMoisture()
{
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    Pwrplnt.setMinMoisture(atoi(arg));
    Serial.print("minMoisture set to ");
    Serial.println(atoi(arg));
  }
}

void setTargetMoisture()
{
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    Pwrplnt.setTargetMoisture(atoi(arg));
    Serial.print("targetMoisture set to ");
    Serial.println(atoi(arg));
  }
}

void setWateringDuration()
{
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    Pwrplnt.setWateringDuration(atoi(arg));
    Serial.print("wateringDuration set to ");
    Serial.println(atoi(arg));
  }
}

void setWateringPause()
{
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    Pwrplnt.setWateringPause(atoi(arg));
    Serial.print("wateringPause set to ");
    Serial.println(atoi(arg));
  }
}

void setLightIntensity()
{
  char *arg;
  arg = sCmd.next();
  if (arg != NULL) {
    Pwrplnt.setLightIntensity(atoi(arg));
    Serial.print("lightIntensity set to ");
    Serial.println(atoi(arg));
  }
}

void setSunriseTime()
{
  Serial.println("not implemented");
}

void setSunsetTime()
{
  Serial.println("not implemented");
}

void setRTCTime() {
  char *arg;
  tmElements_t tm;

  arg = sCmd.next();    
  if (arg != NULL) {    
    tm.Year = atoi(arg);
  } else return;
  arg = sCmd.next();    
  if (arg != NULL) {    
    tm.Month = atoi(arg);
  } else return;
  arg = sCmd.next();
  if (arg != NULL) {
    tm.Day = atoi(arg);
  } else return;
  arg = sCmd.next();
  if (arg != NULL) {
    tm.Hour = atoi(arg);
  } else return;
  arg = sCmd.next();
  if (arg != NULL) {
    tm.Minute = atoi(arg);
  } else return;
  arg = sCmd.next();
  if (arg != NULL) {
    tm.Second = atoi(arg);
  } else return;  
  
  //set time to RTC module
  RTC.set(makeTime(tm));
  Serial.println("time set");
}
