//pwrplnt.ino

#include "Arduino.h"
#include <SerialCommand.h>

#include <Wire.h>  
#include <Time.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#include "settings.h"
#include "cpwrplnt.h"

SerialCommand sCmd;
cPwrplnt Pwrplnt;

void setup()
{
    Serial.begin(115200);

    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet) 
    { Serial.println("Unable to sync with the RTC"); }

    Pwrplnt.init();

    // Setup callbacks for SerialCommand commands
    sCmd.addCommand("help",    showHelp);
    sCmd.addCommand("showStatus", showStatus);
    sCmd.addCommand("showSettings", showSettings);
    sCmd.addCommand("setTime",  setRTCTime);
    sCmd.setDefaultHandler(unrecognized);      

    Serial.println("initialized");
}

void loop()
{
    static bool worked = false;
    // read time and decide to call pwrplnt maintenance
    if(!(minute() % MAINTENANCE_INTERVAL)
        && !worked)
    {
        // work
        Serial.println("working");
        Pwrplnt.maintain();
        worked = true;
    }
    else if (minute() % MAINTENANCE_INTERVAL)
    {
        worked = false;
    }
}

void showHelp() {
  Serial.println();
  Serial.println("Available Commands:");
  Serial.println("showStatus");
  Serial.println("showSettings");
  Serial.println("setRTCTime [year] [mon] [day] [hour] [min] [sec]");
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
    Serial.print(" Temperature: ");
    Serial.println(Pwrplnt.getTemperature());
    Serial.print(" Air Humidity: ");
    Serial.println(Pwrplnt.getAirHumidity());
    Serial.print(" Moisture: ");
    Serial.println(Pwrplnt.getMoisture());
    Serial.print(" Brightness: ");
    Serial.println(Pwrplnt.getBrightness());
    Serial.print(" Water Level Ok?: ");
    Serial.println(Pwrplnt.getWaterLevelOk());
    Serial.println();
    Serial.print(" Pump State: ");
    Serial.println(Pwrplnt.getPumpState());
    Serial.print(" LightState: ");
    Serial.println(Pwrplnt.getLightState());
}

void showSettings() {
    Serial.println();
    Serial.print(" Minimum Soil Moisture: ");
    Serial.println(Pwrplnt.getMinMoisture());
    Serial.print(" Targeted Soil Moisture: ");
    Serial.println(Pwrplnt.getTargetMoisture());
    Serial.print(" Watering Duration: ");
    Serial.println(Pwrplnt.getWateringDuration());
    Serial.print(" Watering Pause: ");
    Serial.println(Pwrplnt.getWateringPause());
    Serial.print(" Light Intensity: ");
    Serial.println(Pwrplnt.getLightIntensity());
    Serial.print(" Sunrise Time: ");
    Serial.println(Pwrplnt.getSunriseTime());
    Serial.print(" Sunset Time: ");
    Serial.println(Pwrplnt.getSunsetTime());
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
}

void unrecognized(const char *command) {
  showHelp();
}
