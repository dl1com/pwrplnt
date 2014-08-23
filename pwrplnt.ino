//pwrplnt.ino

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>
#include <SerialCommand.h>

#include <Wire.h> 
#include <EEPROM.h> 
#include <Time.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#include "settings.h"
#include "cpwrplnt.h"

SerialCommand sCmd;
EthernetClient ethernetClient;
cPwrplnt Pwrplnt;

byte MACaddress[] = {0xDE,0xAD,0xBE,0xEF,0xFE,0xED};
byte IPaddress[] = {192,168,1,123};
byte DNSserverIPaddress[] = {8,8,8,8};
byte gatewayIPaddress[] = { 192, 168, 1, 1 };
byte subnet[] = { 255, 255, 255, 0 };
char serverName[] = "dweet.io";

void setup()
{
    Serial.begin(115200);

    Ethernet.begin(MACaddress, IPaddress, DNSserverIPaddress, gatewayIPaddress, subnet);

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

    // Show current settings to user
    showSettings();

    // do inital measurements
    Pwrplnt.performMeasurements();

}

void loop()
{
    sCmd.readSerial();

    time_t tnow = now();

    static bool worked = true;
    // read time and decide to call pwrplnt maintenance
    if (tnow % ACTION_INTERVAL)
    {
        worked = false;
    }
    else if(!worked)
    {
        // work
        Serial.println("working...");
        Pwrplnt.performMeasurements();
        Pwrplnt.performActions();
        worked = true;
    }

    static bool didDweet = false;
    // read time and decide to Dweet
    if (tnow % DWEET_INTERVAL)
    {
      didDweet = false;
    }
    else if(!didDweet)
    {
      Pwrplnt.performMeasurements();
      // only one try to connect
      // TODO add fibonacci timeout
      didDweet = true;

      // Reporting to dweet.io
      if(ethernetClient.connect(serverName, 80))
      {          
        Serial.println("Dweeting...");
        ethernetClient.print("GET /dweet/for/pwrplnt?");
        ethernetClient.print("active=");
        ethernetClient.print(Pwrplnt.getActive());
        ethernetClient.print("&temperature=");
        ethernetClient.print(Pwrplnt.getTemperature());
        ethernetClient.print("&airHumidity=");
        ethernetClient.print(Pwrplnt.getAirHumidity());
        ethernetClient.print("&brightness=");
        ethernetClient.print(Pwrplnt.getBrightness());
        ethernetClient.print("&moisture=");
        ethernetClient.print(Pwrplnt.getMoisture());
        ethernetClient.print("&waterLevelOk=");
        ethernetClient.print(Pwrplnt.getWaterLevelOk());
        ethernetClient.print("&pumpState=");
        ethernetClient.print(Pwrplnt.getPumpState());
        ethernetClient.print("&lightState=");
        ethernetClient.print(Pwrplnt.getLightState());

        ethernetClient.println(" HTTP/1.1");

        ethernetClient.print("HOST ");
        ethernetClient.println(serverName);
        ethernetClient.println();
        }
    }
}

void showHelp() {
  Serial.println();
  Serial.println("Available Commands:");
  Serial.println("showStatus");
  Serial.println("showSettings");
  Serial.println("setActive");
  Serial.println("setMinMoisture [%]");
  Serial.println("setTargetMoisture [%]");
  Serial.println("setWateringDuration [sec]");
  Serial.println("setWateringPause [sec]");
  Serial.println("setLightIntensity [%]");
  Serial.println("setSunriseTime [hour] [minute]");
  Serial.println("setSunsetTime [hour] [minute]");
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
  char *arg;
  byte hour = 0;
  byte minute = 0;
  arg = sCmd.next();
  if (arg != NULL) {
    hour = atoi(arg);
  }
  else return;
  arg = sCmd.next();
  if (arg != NULL) {    
    minute = atoi(arg);
  }
  else return;

  Serial.print("Sunrise set to ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  Pwrplnt.setSunriseTime((hour*3600) + (minute * 60));
}

void setSunsetTime()
{
  char *arg;
  byte hour = 0;
  byte minute = 0;
  arg = sCmd.next();
  if (arg != NULL) {
    hour = atoi(arg);
  }
  else return;
  arg = sCmd.next();
  if (arg != NULL) {    
    minute = atoi(arg);
  }
  else return;

  Serial.print("Sunset set to ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  Pwrplnt.setSunsetTime((hour*3600) + (minute * 60));
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
  setTime(tm.Hour,tm.Minute,tm.Second,tm.Day,tm.Month,tm.Year);
  RTC.set(makeTime(tm));
  Serial.println("time set");
}
