//cpwrplnt.cpp

#include "cpwrplnt.h"

#include <dht11.h>
dht11 DHT11;

cPwrplnt::cPwrplnt() : 
    m_doActions(false),
    m_minMoisture(0),
    m_targetMoisture(0),
    m_wateringDuration(0),
    m_wateringPause(0),
    m_lightIntensity(255),
    m_timeSunrise(0),
    m_timeSunset(0)
{
    // intentionally left blank
}

void cPwrplnt::init(void)
{
    // set up Pin Modes
    pinMode(PIN_MOISTURE, INPUT);
    pinMode(PIN_BRIGHTNESS, INPUT);
    
    // TODO read from EEPROM
    time_t tnow = now();
    // Init Times
    m_lastPumpStart = tnow;
    m_lastPumpStop  = tnow;

}

void cPwrplnt::maintain(void)
{
    // do measurements, then act accordingly
    performMeasurements();

    if(m_doActions)
    {
        performActions();
    }
}

void cPwrplnt::setWateringDuration(byte sec)
{
    m_wateringDuration = sec;
}

void cPwrplnt::setLightIntensity(byte intensity)
{
    m_lightIntensity = intensity;
}

void cPwrplnt::setSunriseTime(time_t t)
{
    m_timeSunrise = t;
}

void cPwrplnt::setSunsetTime(time_t t)
{
    m_timeSunset = t;
}

void cPwrplnt::performMeasurements(void)
{
    unsigned int tmp_meas;

    // brightness (%)
    tmp_meas = 1023-analogRead(PIN_BRIGHTNESS);
    m_brightness = (byte)((tmp_meas*100)/1023);

    // air humidity
    // air temperature
    DHT11.read(PIN_DHT11);
    // TODO temperature calibration
    m_temperature = DHT11.temperature;
    m_airHumidity = DHT11.humidity;

    // soil moisture (%)
    tmp_meas = analogRead(PIN_MOISTURE);
    m_moisture = (byte)((m_brightness*100)/1023);

    // tank water level
    // TODO implement
}

void cPwrplnt::performActions(void)
{    
    time_t tnow = now();

    // switch Lights according to time of day
    // TODO perhaps call using TimeAlarms lib
    // or make use of time_t and modulo
    if(hour(tnow) >= hour(m_timeSunrise)
        && minute(tnow) >= minute(m_timeSunrise)
        && hour(tnow) <= hour(m_timeSunset)
        && minute(tnow) <= minute(m_timeSunset))
    {
        // Turn on lights
        // TODO take currently measured brightness into account
        setLight(m_lightIntensity);
    }
    else
    {
        // Turn of lights
        setLight(0);
    }

    // TODO add measurement of reservoir level
    if( true /* !m_waterLevelOk */)
    {
        // Activate pump accordingly to soil moisture
        if(m_moisture < m_minMoisture)
        {
            // Only start pump if enough time has passed
            // since the last stop
            // and pump is off
            if(tnow > (m_lastPumpStop + m_wateringPause)
                && false == m_pumpState)
            {
                switchPump(true);
                m_lastPumpStart = tnow;
            }
        }

        // Stop pump if moisture level is reached or timeout
        // and pump is running
        if ((m_moisture > m_targetMoisture)
            || (tnow > (m_lastPumpStart + m_wateringDuration))
            && true == m_pumpState)
        {
            switchPump(false);
            m_lastPumpStop = tnow;
        }
    }
}

void cPwrplnt::switchPump(bool state)
{
    m_pumpState = state;
    digitalWrite(PIN_PUMPRELAIS, m_pumpState);
}

void cPwrplnt::setLight(byte intensity)
{
    m_lightState = intensity;
    analogWrite(PIN_LIGHTPWM, m_lightState);
}