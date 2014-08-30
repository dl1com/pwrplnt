//pwrplnt.ino

#include "Arduino.h"
#include <SPI.h>

#include <SerialCommand.h>

#include <Wire.h> 
#include <EEPROM.h> 
#include <Time.h>
#include <DS1307RTC.h>

#include "settings.h"
#include "cpwrplnt.h"

#if ENABLE_ETHERNET
  #include <Ethernet.h>
  byte MACaddress[]         = {0xDE,0xAD,0xB0,0x0B,0xCA,0xFE};
  byte IPaddress[]          = {83,133,178,122};
  byte DNSserverIPaddress[] = {8,8,8,8};
  byte gatewayIPaddress[]   = {83,133,178,65};
  byte subnet[]             = {255,255,255,192};
  EthernetClient ethernetClient;
#endif //ENABLE_ETHERNET

#if ENABLE_PLOTLY
  #include "plotly_streaming_ethernet.h"
  #include "plotly_setup.h"
#endif //ENABLE_PLOTLY

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

    // set up timing device
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet) 
    { Serial.println("Unable to sync with the RTC"); }

    // sCmd msg Handlers
    addMsgHandlers();

#if ENABLE_ETHERNET
    // for online logging and plotting
    Ethernet.begin(
      MACaddress, 
      IPaddress, 
      DNSserverIPaddress, 
      gatewayIPaddress, 
      subnet);
#endif //ENABLE_ETHERNET

#if ENABLE_PLOTLY
    // plot.ply
    plotlygraph.fileopt="extend";
    bool success = plotlygraph.init(); // alternatively: extend
    if(!success){while(true){}} // TODO while(true), rlly?
    plotlygraph.openStream();
#endif //ENABLE_PLOTLY     

    // init Pwrplnt object
    Pwrplnt.init();
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
        Serial.println("");
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

#if ENABLE_DWEETIO
      // Reporting to dweet.io
      Serial.print("Dweeting...");
      if(ethernetClient.connect("dweet.io", 80))
      {          
        Serial.print("success!");
        ethernetClient.print("GET /dweet/for/");
        ethernetClient.print(DWEETIO_THINGNAME);
        ethernetClient.print("?active=");
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
        
        ethernetClient.println("HOST dweet.io");
        ethernetClient.println();

        ethernetClient.stop();
      }
      Serial.println("");
#endif //ENABLE_DWEETIO

#if ENABLE_PLOTLY
      // Reporting to plot.ly
      Serial.println("Plot.ly...");
      plotlygraph.plot(millis()+14400000, Pwrplnt.getTemperature(), tokens[0]);
      plotlygraph.plot(millis()+14400000, Pwrplnt.getAirHumidity(), tokens[1]);
      plotlygraph.plot(millis()+14400000, Pwrplnt.getBrightness(), tokens[2]);
      plotlygraph.plot(millis()+14400000, Pwrplnt.getMoisture(), tokens[3]);
      plotlygraph.plot(millis()+14400000, Pwrplnt.getWaterLevelOk(), tokens[4]);
      plotlygraph.plot(millis()+14400000, Pwrplnt.getPumpState(), tokens[5]);
      plotlygraph.plot(millis()+14400000, Pwrplnt.getLightState(), tokens[6]);
#endif //ENABLE_PLOTLY
    }
}

void addMsgHandlers()
{
    // Setup callbacks for SerialCommand commands
    sCmd.addCommand("help",    showHelp);
    sCmd.addCommand("showStatus", showStatus);
    sCmd.addCommand("showSettings", showSettings);
    sCmd.addCommand("showEthernetSettings", showEthernetSettings);
    sCmd.addCommand("setActive", setActive);
    sCmd.addCommand("setMinMoisture", setMinMoisture);
    sCmd.addCommand("setTargetMoisture", setTargetMoisture);
    sCmd.addCommand("setWateringDuration", setWateringDuration);
    sCmd.addCommand("setWateringPause", setWateringPause);
    sCmd.addCommand("setLightIntensity", setLightIntensity);
    sCmd.addCommand("setSunriseTime", setSunriseTime);
    sCmd.addCommand("setSunsetTime", setSunsetTime);
    sCmd.addCommand("setTime",  setRTCTime);
    sCmd.addCommand("resetEEPROM", resetEEPROM);
    sCmd.addDefaultHandler(showHelp);
}

void showHelp() {
  Serial.println();
  Serial.println("Available Commands:");
  Serial.println("showStatus");
  Serial.println("showSettings");
  Serial.println("showEthernetSettings");
  Serial.println("setActive");
  Serial.println("setMinMoisture [%]");
  Serial.println("setTargetMoisture [%]");
  Serial.println("setWateringDuration [sec]");
  Serial.println("setWateringPause [sec]");
  Serial.println("setLightIntensity [%]");
  Serial.println("setSunriseTime [hour] [minute]");
  Serial.println("setSunsetTime [hour] [minute]");
  Serial.println("setTime [year] [mon] [day] [hour] [min] [sec]");
  Serial.println("resetEEPROM");
  Serial.println();
}

void showStatus() {
    Pwrplnt.performMeasurements();

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

void showEthernetSettings() {
#if ENABLE_ETHERNET
  Serial.println();
  Serial.print("MAC    : ");
  Serial.print(MACaddress[0]);Serial.print(":");
  Serial.print(MACaddress[1]);Serial.print(":");
  Serial.print(MACaddress[2]);Serial.print(":");
  Serial.print(MACaddress[3]);Serial.print(":");
  Serial.print(MACaddress[4]);Serial.print(":");
  Serial.println(MACaddress[5]);
  Serial.print("IP     : ");
  Serial.print(IPaddress[0]);Serial.print(".");
  Serial.print(IPaddress[1]);Serial.print(".");
  Serial.print(IPaddress[2]);Serial.print(".");
  Serial.println(IPaddress[3]);
  Serial.print("Gateway: ");
  Serial.print(gatewayIPaddress[0]);Serial.print(".");
  Serial.print(gatewayIPaddress[1]);Serial.print(".");
  Serial.print(gatewayIPaddress[2]);Serial.print(".");
  Serial.println(gatewayIPaddress[3]);
  Serial.print("Subnet : ");
  Serial.print(subnet[0]);Serial.print(".");
  Serial.print(subnet[1]);Serial.print(".");
  Serial.print(subnet[2]);Serial.print(".");
  Serial.println(subnet[3]);
#else
  Serial.println("Ethernet disabled");
#endif //ENABLE_ETHERNET
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
  byte _hour = 0;
  byte _min = 0;
  arg = sCmd.next();
  if (arg != NULL) {
    _hour = atoi(arg);
  }
  else return;
  arg = sCmd.next();
  if (arg != NULL) {    
    _min = atoi(arg);
  }
  else return;

  Serial.print("Sunrise set to ");
  Serial.print(_hour);
  Serial.print(":");
  Serial.println(_min);
  time_t t = (_hour*(time_t)3600) + (_min * (time_t)60);
  Pwrplnt.setSunriseTime(t);
}

void setSunsetTime()
{
  char *arg;
  byte _hour = 0;
  byte _min = 0;
  arg = sCmd.next();
  if (arg != NULL) {
    _hour = atoi(arg);    
  }
  else return;
  arg = sCmd.next();
  if (arg != NULL) {    
    _min = atoi(arg);
  }
  else return;

  Serial.print("Sunset set to ");
  Serial.print(_hour);
  Serial.print(":");
  Serial.println(_min);
  time_t t = (_hour*(time_t)3600) + (_min * (time_t)60);
  Pwrplnt.setSunsetTime(t);
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
  RTC.set(now());
  Serial.println("time set");
  resetEEPROM();
}

void resetEEPROM(void)
{
  Pwrplnt.resetEEPROM();
}
