//pwrplnt.ino

#include "Arduino.h"

#include "cpwrplnt.h"
#include "settings.h"

cPwrplnt Pwrplnt;

void setup()
{
    Serial.begin(115200);

        /*set rtc to rnning mode */
    rtc.halt(false);


    Serial.println("initialized");
}

void loop()
{
    // read time and decide to call pwrplnt maintenance
    static Time t_old;    
    Time t = rtc.getTime();
    if(t.min != t_old.min 
        && !(t.min % MAINTENANCE_INTERVAL))
    {
        // work
        Serial.println("working");
        Pwrplnt.maintain();
    }
    t_old = t;

}