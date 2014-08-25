//plotly_setup.h

#ifndef PLOTY_SETUP_H
#define PLOTY_SETUP_H

#define nTraces 7
#define TOKEN_TEMPERATURE   "moep"
#define TOKEN_AIRHUMIDITY   "moep"
#define TOKEN_BRIGHTNESS    "moep"
#define TOKEN_MOISTURE      "moep"
#define TOKEN_WATERLEVELOK  "moep"
#define TOKEN_PUMPSTATE     "moep"
#define TOKEN_LIGHTSTATE    "moep"

char *tokens[nTraces] = {TOKEN_TEMPERATURE,
                        TOKEN_AIRHUMIDITY,
                        TOKEN_BRIGHTNESS,
                        TOKEN_MOISTURE,
                        TOKEN_WATERLEVELOK,
                        TOKEN_PUMPSTATE,
                        TOKEN_LIGHTSTATE};
plotly plotlygraph = plotly(
  "plotly_username", 
  "plotly_api_key", 
  tokens, 
  "pwrplnt", 
  nTraces);

#endif // PLOTY_SETUP_H