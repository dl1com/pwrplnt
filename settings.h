// settings.h

#ifndef SETTINGS_H
#define SETTINGS_H

#include <dht11.h>
#include <DS1302.h>

#define PIN_DS1302_CE   2
#define PIN_DS1302_IO   3
#define PIN_DS1302_SCLK 4

#define PIN_PUMPRELAIS  6
#define PIN_LIGHTPWM    7
#define PIN_BRIGHTNESS  A5
#define PIN_DHT11       5

#define MAINTENANCE_INTERVAL 5 /* minutes */

dht11 DHT11;
DS1302 rtc(PIN_DS1302_CE, 
            PIN_DS1302_IO,
            PIN_DS1302_SCLK);

#endif SETTINGS_H