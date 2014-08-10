//cpwrplnt.h

#ifndef CPWRPLNT_H
#define CPWRPLNT_H

#include "Arduino.h"

class cPwrplnt
{
public:
    cPwrplnt();

    void maintain(void);

    void setWateringDuration(byte sec);

private:

    void performMeasurements(void);
    /* turn pump on or off */
    void switchPump(bool state);
    /* turn light on or off */
    void switchLight(bool state);

    byte m_wateringDuration; /* in seconds */
    bool m_pumpState;
    bool m_lightState;

};

#endif