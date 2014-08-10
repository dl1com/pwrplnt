//cpwrplnt.cpp

#include "cpwrplnt.h"

cPwrplnt::cPwrplnt()
{
    // intentionally left blank
    pinMode(PIN_BRIGHTNESS, INPUT);
}

void cPwrplnt::maintain(void)
{
    // do measurements
    performMeasurements();
    // act accordingly

}

void cPwrplnt::setWateringDuration(byte sec)
{
    m_wateringDuration = sec;
}

void cPwrplnt::performMeasurements(void)
{
    // brightness
    m_brightness = 1023-analogRead(PIN_BRIGHTNESS);
    m_brightness = ((m_brightness*100)/1023);

    // air humidity
    // air temperature
    DHT11.read(PIN_DHT11);
    // TODO temperature calibration
    m_temperature = DHT11.temperature;
    m_airHumidity = DHT11.humidity;

    // soil humidity
    // TODO
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