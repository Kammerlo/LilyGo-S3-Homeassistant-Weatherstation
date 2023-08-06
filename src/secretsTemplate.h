#ifndef    SECRETS_H
  #define  SECRETS_H
  #include <Arduino.h>
  #include <map>

    const String SSID = "";
    const String  WIFI_PASSWORD = "";
    const String  HA_TOKEN = "";
    const String HA_BASE_URL = "";
    const String  location_lat = "";
    const String  location_lon = "";
    const String  weather_token = "";

    const String OPENWEATHER_URL = "http://api.openweathermap.org/data/2.5/weather?lat="+ location_lat +"&lon="+ location_lon +"&appid="+ weather_token +"&units=metric";
    const String FORECAST_URL = "http://api.openweathermap.org/data/2.5/forecast?lat="+ location_lat +"&lon="+ location_lon +"&appid="+ weather_token +"&units=metric&cnt=8";

    const String HUMIDITY_ENTITY_NETATMO = "";
    const String TEMPERATURE_ENTITY_NETATMO = "";
    const String BATTERY_ENTITY_NETATMO = "";
    const String OFFICE_TEMPERATURE = "";

    const String VILLAGE_NAME = "";

   
#endif