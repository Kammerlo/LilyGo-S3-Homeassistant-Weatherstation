

#ifndef    HELPER_H
  #define  HELPER_H

  #include <Arduino.h>
  #include <ArduinoJson.h>
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <map>
  #include "secrets.h"
  #include <string>
  #include "images/clear_sky.h"
  #include "images/few_clouds.h"
  #include "images/scattered_clouds.h"
  #include "images/shower_rain.h"
  #include "images/rain.h"
  #include "images/thunderstorm.h"
  #include "images/snow.h"
  #include "images/mist.h"
  #include "images/calendar.h"

  #define FONT_HEIGHT 45


  

  struct OpenWeatherStruct{
    String weather_description;
    String iconName;
    String main;
    String description;
    float temperature;
    float humidity;
  };

  struct OpenWeatherForecast {
    float temp[8];
    float rain[8];

  };

  struct haSensorStruct{
    String state;
    String unit;
  };

  String httpCall(String url);
  void connectToWifi();
  OpenWeatherStruct getOpenWeatherStruct();
  haSensorStruct getHaSensorStruct(String entity);
  OpenWeatherForecast getWeatherForecast();
  const unsigned short* getWeatherImage(String icon);
  String customDateSplit(String str);



#endif