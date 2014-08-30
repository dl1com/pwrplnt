// settings.h

#ifndef SETTINGS_H
#define SETTINGS_H

#define ENABLE_ETHERNET 1
#define ENABLE_DWEETIO  1 // requires Ethernet
#define ENABLE_PLOTLY   1 // requires Ethernet

#define DWEETIO_THINGNAME "pwrplnt"

#define PIN_PUMPRELAIS  23
#define PIN_LIGHTPWM    22
#define PIN_MOISTURE    A8
#define PIN_BRIGHTNESS  A9
#define PIN_DHT11       A11
// TODO Reservoir Level A12

#define ACTION_INTERVAL 10  /* seconds */
#define DWEET_INTERVAL  60 /* seconds */

#endif // SETTINGS_H