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

private:

    /* get humidity, brightness, temperature */
    void performMeasurements(void);
    /* turn pump on or off */
    void switchPump(bool state);
    /* turn light on or off */
    void setLight(byte intensity);

    byte m_wateringDuration; /* in seconds */
    
    bool m_pumpState;
    byte m_lightState;
    byte m_soilHumidity; /* percent */
    byte m_airHumidity;  /* percent */
    byte m_temperature;  /* degree celcius */
    byte m_brightness;   /* percent */     

};

#endif