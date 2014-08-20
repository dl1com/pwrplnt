//cpwrplnt.h

#ifndef CPWRPLNT_H
#define CPWRPLNT_H

#include "Arduino.h"
#include "settings.h"

#include <Time.h>




class cPwrplnt
{
public:
    cPwrplnt();

    void init(void);
    void maintain(void);

    void setWateringDuration(byte sec);
    void setLightIntensity(byte intensity);
    void setSunriseTime(time_t);
    void setSunsetTime(time_t);

    //Getters
    // Settings
    byte getMinMoisture(void) const
        { return m_minMoisture; }
    byte getTargetMoisture(void) const
        { return m_targetMoisture; }
    unsigned int getWateringDuration(void) const
        { return m_wateringDuration; }
    unsigned int getWateringPause(void) const
        { return m_wateringPause; }
    byte getLightIntensity(void) const
        { return m_lightIntensity; }
    time_t getSunriseTime(void) const
        { return m_timeSunrise; }
    time_t getSunsetTime(void) const
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
    bool getWaterLevelOk(void) const
        { return m_waterLevelOk; }

private:

    /* get humidity, brightness, temperature */
    void performMeasurements(void);
    /* switch Light and Pump */
    void performActions(void);

    // Helper
    /* turn pump on or off */
    void switchPump(bool state);
    /* turn light on or off */
    void setLight(byte intensity);

    // Settings
    /* Activate Actors */
    bool m_doActions;
    /* Moisture level to start watering */
    byte m_minMoisture; /* (%) */
    /* Moisture level to stop watering */
    byte m_targetMoisture; /* (%) */
    /* time the pump is running when watering */
    unsigned int m_wateringDuration; /* (seconds) */
    /* time to wait for water to settle, before allowing new watering */
    unsigned int m_wateringPause; /* (seconds) */
    byte m_lightIntensity; /* 0-255 */
    time_t m_timeSunrise;
    time_t m_timeSunset;

    // States
    bool m_pumpState;
    time_t m_lastPumpStart;
    time_t m_lastPumpStop;
    byte m_lightState;

    // Measurements
    byte m_moisture;     /* (%) */
    byte m_airHumidity;  /* (%) */
    byte m_temperature;  /* degree celcius */
    byte m_brightness;   /* (%) */     
    bool m_waterLevelOk; /* true when water tank has to be refilled */
};

#endif