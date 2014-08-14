//cpwrplnt.h

#ifndef CPWRPLNT_H
#define CPWRPLNT_H

#include "Arduino.h"
#include "settings.h"

#include <dht11.h>
#include <DS1302.h>

class cPwrplnt
{
public:
    cPwrplnt();

    void init(Time);
    void maintain(Time);

    void setWateringDuration(byte sec);
    void setLightIntensity(byte intensity);
    void setSunriseTime(Time);
    void setSunsetTime(Time);

    //Getters
    // Settings
    byte getMinMoisture(void) const
        { return m_minMoisture; }
    unsigned int getWateringDuration(void) const
        { return m_wateringDuration; }
    unsigned int getWateringPause(void) const
        { return m_wateringPause; }
    byte getLightIntensity(void) const
        { return m_lightIntensity; }
    Time getSunriseTime(void) const
        { return m_timeSunrise; }
    Time getSunsetTime(void) const
        { return m_timeSunset; }
    // States
    bool getPumpState(void) const
        { return m_pumpState; }
    byte getLightState(void) const
        { return m_lightState; }
    // Measurements
    byte getMoisture(void) const
        { return m_moisture; }
    byte getAirHumidity(void) const
        { return m_airHumidity; }
    byte getTemperature(void) const
        { return m_temperature; }
    byte getBrightness(void) const
        { return m_brightness; }
    bool getLowWaterLevel(void) const
        { return m_lowWaterLevel; }

private:

    /* get humidity, brightness, temperature */
    void performMeasurements(void);
    /* turn pump on or off */
    void switchPump(bool state);
    /* turn light on or off */
    void setLight(byte intensity);

    // Settings
    /* Moisture level to start watering */
    byte m_minMoisture; /* (%) */
    unsigned int m_wateringDuration; /* (seconds) */
    /* time to forbid watering after watering */
    unsigned int m_wateringPause; /* (seconds) */
    byte m_lightIntensity;
    Time m_timeSunrise;
    Time m_timeSunset;
    // States
    bool m_pumpState;
    Time m_lastPumpStart;
    Time m_lastPumpStop;
    byte m_lightState;
    // Measurements
    byte m_moisture;     /* (%) */
    byte m_airHumidity;  /* (%) */
    byte m_temperature;  /* degree celcius */
    byte m_brightness;   /* (%) */     
    bool m_lowWaterLevel; /* true when water tank has to be refilled */
};

#endif