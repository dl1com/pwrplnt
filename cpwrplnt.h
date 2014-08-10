//cpwrplnt.h

#ifndef CPWRPLNT_H
#define CPWRPLNT_H

#include "Arduino.h"
#include "settings.h"

class cPwrplnt
{
public:
    cPwrplnt();

    void maintain(void);

    void setWateringDuration(byte sec);
    void setLightIntensity(byte intensity);
    void setSunriseTime(Time t);
    void setSunsetTime(Time t);

private:

    /* get humidity, brightness, temperature */
    void performMeasurements(void);
    /* turn pump on or off */
    void switchPump(bool state);
    /* turn light on or off */
    void setLight(byte intensity);

    byte m_wateringDuration; /* seconds */
    byte m_lightIntensity;
    Time m_timeSunrise;
    Time m_timeSunset;

    bool m_pumpState;
    byte m_lightState;
    byte m_soilHumidity; /* percent */
    byte m_airHumidity;  /* percent */
    byte m_temperature;  /* degree celcius */
    byte m_brightness;   /* percent */     
    bool m_lowWaterLevel; /* true when water tank has to be refilled */
};

#endif