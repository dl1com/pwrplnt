//pwrplnt.ino

#include "Arduino.h"
#include <SerialCommand.h>

#include <Time.h>
#include <dht11.h>
#include <DS1302.h>

#include "settings.h"
#include "cpwrplnt.h"

dht11 DHT11;
DS1302 rtc(PIN_DS1302_CE, 
            PIN_DS1302_IO,
            PIN_DS1302_SCLK);

cPwrplnt Pwrplnt;

SerialCommand sCmd;

void setup()
{
    Serial.begin(115200);

    /*set rtc to running mode */
    rtc.halt(false);
    setSyncProvider(getTime);
    if(timeStatus()!= timeSet) 
    {
        Serial.println("Unable to sync with the RTC");
    }

    Pwrplnt.init();

    // Setup callbacks for SerialCommand commands
    sCmd.addCommand("help",    showHelp);
    sCmd.addCommand("showStatus", showStatus);
    sCmd.addCommand("showSettings", showSettings);
    sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")

    Serial.println("initialized");
}

void loop()
{
    // read time and decide to call pwrplnt maintenance
    static time_t t_old;    
    time_t t_now = now();
    if(minute(t_now) != minute(t_old) 
        && !(minute(t_now) % MAINTENANCE_INTERVAL))
    {
        // work
        Serial.println("working");
        Pwrplnt.maintain();
    }
    t_old = t_now;
}

void showHelp() {
  Serial.println();
  Serial.println("Available Commands:");
  Serial.println("showStatus");
  Serial.println("showSettings");
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
    Serial.println(rtc.getTimeStr());
    Serial.print(" Temperature: ");
    Serial.println(Pwrplnt.getTemperature());
    Serial.print(" Air Humidity: ");
    Serial.println(Pwrplnt.getAirHumidity());
    Serial.print(" Moisture: ");
    Serial.println(Pwrplnt.getMoisture());
    Serial.print(" Brightness: ");
    Serial.println(Pwrplnt.getBrightness());
    Serial.print(" Low Water Level: ");
    Serial.println(Pwrplnt.getLowWaterLevel());
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

void unrecognized(const char *command) {
  showHelp();
}

// Helper function to get time from RTC module
time_t getTime()
{
  Time t = rtc.getTime();
  tmElements_t tm;
  tm.Second = t.sec;   
  tm.Minute = t.min;
  tm.Hour   = t.hour;  // mask assumes 24hr clock
  tm.Wday   = t.dow;
  tm.Day    = t.date;
  tm.Month  = t.mon;
  tm.Year   = t.year;
  return(makeTime(tm));
}
