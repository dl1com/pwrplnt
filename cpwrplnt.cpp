//cpwrplnt.cpp
#include "cpwrplnt.h"

#include "eepromanything.h"

#include <dht11.h>
dht11 DHT11;

cPwrplnt::cPwrplnt() : 
    m_doActions(true),
    m_minMoisture(20),
    m_targetMoisture(80),
    m_wateringDuration(10),
    m_wateringPause(60),
    m_lightIntensity(255),
    m_timeSunrise(28000), // 08:00
    m_timeSunset(72000)   // 20:00
{
    // intentionally left blank
}

void cPwrplnt::init(void)
{
    // Read settings and states from EEPROM
    EEPROM_readAnything(ADDR_ACTIVE, m_doActions);
    EEPROM_readAnything(ADDR_MINMOISTURE, m_minMoisture);
    EEPROM_readAnything(ADDR_TARGETMOISTURE, m_targetMoisture);
    EEPROM_readAnything(ADDR_WATERINGDURATION, m_wateringDuration);
    EEPROM_readAnything(ADDR_WATERINGPAUSE, m_wateringPause);
    EEPROM_readAnything(ADDR_LIGHTINTENSITY, m_lightIntensity);
    EEPROM_readAnything(ADDR_SUNRISETIME, m_timeSunrise);
    EEPROM_readAnything(ADDR_SUNSETTIME, m_timeSunset);
    EEPROM_readAnything(ADDR_LASTPUMPSTART, m_lastPumpStart);
    EEPROM_readAnything(ADDR_LASTPUMPSTOP, m_lastPumpStop);
}

void cPwrplnt::resetEEPROM()
{
    time_t tnow = now();
    EEPROM_writeAnything(ADDR_LASTPUMPSTOP, tnow);
    EEPROM_writeAnything(ADDR_LASTPUMPSTOP, tnow);
}

void cPwrplnt::setActive(bool active)
{
    m_doActions = active;
    EEPROM_writeAnything(ADDR_ACTIVE, m_doActions);
}

void cPwrplnt::setMinMoisture(byte minMoisture)
{
    if(0 <= minMoisture && minMoisture <= 100)
    {
        m_minMoisture = minMoisture;
        EEPROM_writeAnything(ADDR_MINMOISTURE, m_minMoisture);
    }
}

void cPwrplnt::setTargetMoisture(byte targetMoisture)
{
    if(0 <= targetMoisture && targetMoisture <= 100)
    {
        m_targetMoisture = targetMoisture;
        EEPROM_writeAnything(ADDR_TARGETMOISTURE, m_targetMoisture);
    }
}

void cPwrplnt::setWateringDuration(byte sec)
{
    m_wateringDuration = sec;
    EEPROM_writeAnything(ADDR_WATERINGDURATION, m_wateringDuration);
}

void cPwrplnt::setWateringPause(byte sec)
{
    m_wateringPause = sec;
    EEPROM_writeAnything(ADDR_WATERINGPAUSE, m_wateringPause);
}

void cPwrplnt::setLightIntensity(byte intensity)
{
    m_lightIntensity = intensity;
    EEPROM_writeAnything(ADDR_LIGHTINTENSITY, m_lightIntensity);
}

void cPwrplnt::setSunriseTime(time_t t)
{
    if(t < SECS_PER_DAY)
    {
        m_timeSunrise = t;
        EEPROM_writeAnything(ADDR_SUNRISETIME, m_timeSunrise);
    }
}

void cPwrplnt::setSunsetTime(time_t t)
{
    if(t < SECS_PER_DAY)
    {
        m_timeSunset = t;
        EEPROM_writeAnything(ADDR_SUNSETTIME, m_timeSunset);
    }
}

void cPwrplnt::performMeasurements(void)
{
    Serial.println(" performMeasurements");

    unsigned int tmp_meas;

    // brightness (%)
    tmp_meas = 1023-analogRead(PIN_BRIGHTNESS);
    m_brightness = (byte)((tmp_meas*100)/1023);

    // air humidity
    // air temperature
    DHT11.read(PIN_DHT11);
    // TODO variable temperature calibration
    m_temperature = DHT11.temperature;
    m_temperature -= 2; // Hard coded calibration
    m_airHumidity = DHT11.humidity;

    // soil moisture (%)
    tmp_meas = 1023-analogRead(PIN_MOISTURE);
    m_moisture = (byte)((tmp_meas*100)/1023);

    // tank water level
    // TODO implement
    m_waterLevelOk = true;
}

void cPwrplnt::performActions(void)
{    
    if(!m_doActions)
    {
        Serial.println(" Pwrplnt disabled");
        setLight(0);
        switchPump(false);
        return;
    }

    Serial.println(" performActions");
    time_t tnow = now();

    // switch Lights according to time of day
    if(elapsedSecsToday(tnow) > m_timeSunrise
        && elapsedSecsToday(tnow) < m_timeSunset)
    {
        // Turn on lights
        // TODO take currently measured brightness into account
        setLight(m_lightIntensity);
        Serial.println("  Daytime!");
    }
    else
    {
        // Turn the lights off
        setLight(0);
        Serial.println("  Nighttime!");
    }

    // only act with the pump if the level is ok
    if(m_waterLevelOk)
    {
        // Activate pump accordingly to soil moisture
        if(m_moisture < m_minMoisture)
        {
            Serial.print("  too dry!...");
            // Only start pump if enough time has passed
            // since the last stop
            // and pump is off
            if(tnow > (m_lastPumpStop + m_wateringPause)
                && false == m_pumpState)
            {
                switchPump(true);
            }
            else
            {
                if(m_pumpState)
                {
                    Serial.println("pump already running");
                }
                else
                {
                    Serial.println("but pump not allowed to start again yet");
                }
            }
        }

        // Stop pump if moisture level is reached or timeout
        // and pump is running
        if ((m_moisture > m_targetMoisture)
            || (tnow > (m_lastPumpStart + m_wateringDuration))
            && true == m_pumpState)
        {            
            switchPump(false);
        }
    }
    else
    {   // not enough water in reservoir
        Serial.println("  Reservoir low!");
        switchPump(false); 
    }
}

void cPwrplnt::switchPump(bool state)
{
    m_pumpState = state;
    // Relay works inverted
    digitalWrite(PIN_PUMPRELAIS, !m_pumpState);

    if(m_pumpState)
    {
        Serial.println("  Pump on");        
        m_lastPumpStart = now();
        EEPROM_writeAnything(ADDR_LASTPUMPSTART, m_lastPumpStart);
    }
    else
    {
        Serial.println("  Pump off");        
        m_lastPumpStop = now();
        EEPROM_writeAnything(ADDR_LASTPUMPSTOP, m_lastPumpStop);
    }

}

void cPwrplnt::setLight(byte intensity)
{
    m_lightState = intensity;
    // Relais works inverted
    digitalWrite(PIN_LIGHTPWM, !m_lightState);
    // TODO do PWM analogWrite(PIN_LIGHTPWM, !m_lightState);
}