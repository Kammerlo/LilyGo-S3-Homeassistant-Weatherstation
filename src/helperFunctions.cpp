#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "helperFunctions.h"
#include "secrets.h"

/**
 * This function parses the OpenWeatherIcon to display the right icon. 
 * Icons are in the form e.g. '09d.png'. 
 * Documentation for OpenWeatherAPI is here: https://openweathermap.org/weather-conditions#Icon-list
*/
const unsigned short* getWeatherImage(String icon) {
  int iconNumber = std::stoi(icon.c_str());
  const unsigned short* weatherImage;
  switch(iconNumber) {
    case 01:
      weatherImage = clear_sky;
      break;
    case 02:
      weatherImage = few_clouds;
      break;
    case 03: 
      weatherImage = scattered_clouds;
      break;
    case 04:
      weatherImage = scattered_clouds;
      break;
    case 9:
      weatherImage = shower_rain;
      break;
    case 10:
      weatherImage = rain;
      break;
    case 11:
      weatherImage = thunderstorm;
      break;
    case 13:
      weatherImage = snow;
      break;
    case 50:
      weatherImage = mist;
      break;
  }
  return weatherImage;
}

/**
 * Function just for splitting the date nicely :)
 * Converting from 2023-08-06 to 06.08. (German date format)
*/
String customDateSplit(String str) {
  String dateParts[3];
  int wordCount = 0;
  char* char_array = new char[str.length() + 1];
  strcpy(char_array,str.c_str());
    // Returns first token
    char *token = strtok(char_array, "-");
   
    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != NULL)
    {
        dateParts[wordCount] = token;
        wordCount+= 1;
        token = strtok(NULL, "-");
    }
    String correctDate = dateParts[2] + "." + dateParts[1] + ".";
    return correctDate;
}

String httpCall(String url) {
  WiFiClient wifiClient;
  HTTPClient httpClient;

  String payload;
  httpClient.addHeader("Content-Type","application/json");
  if(httpClient.begin(wifiClient,url.c_str())) {
    int httpCode = httpClient.GET();
    if (httpCode > 0) {
      if(httpCode == 200) {
        payload = httpClient.getString();
      } else {
        // HTTP Error
        payload = "HTTP-Error: " + String(httpCode) + " payload:" + httpClient.getString();
      }
      
    } else {
      payload = httpClient.errorToString(httpCode);
    }
  } else {
    payload = "HTTPClient Begin failed:" + httpClient.getString();
  }
  httpClient.end();
  return payload;
}

void connectToWifi() {
  WiFi.begin(SSID,WIFI_PASSWORD);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

/**
 * Returns the current weather conditions for my location.
*/
OpenWeatherStruct getOpenWeatherStruct() {
    StaticJsonDocument<1024> doc;
    String payload = httpCall(OPENWEATHER_URL);
    DeserializationError error = deserializeJson(doc, payload);
    OpenWeatherStruct currentDataStruct;
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return currentDataStruct;
    }
    
    currentDataStruct.weather_description = doc["weather"][0]["description"].as<String>();
    currentDataStruct.temperature = doc["main"]["temp"];
    currentDataStruct.humidity = doc["main"]["humidity"]; 
    currentDataStruct.iconName = doc["weather"][0]["icon"].as<String>();
    currentDataStruct.main = doc["weather"][0]["main"].as<String>();
    currentDataStruct.description = doc["weather"][0]["description"].as<String>();
    return currentDataStruct;
}

/**
 * Returns the forecast for the next 24Hours. If you need more you need to increase the JsonDocument Size and the iteration in the for-loop.
 * Documentation of API is here: https://openweathermap.org/forecast5
*/
OpenWeatherForecast getWeatherForecast() {
  String payload = httpCall(FORECAST_URL);
  DynamicJsonDocument doc(6144);

  DeserializationError error = deserializeJson(doc, payload);
  OpenWeatherForecast obj;
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return obj;
  }
  
  for(int i = 0; i < 8; i++) {
    obj.temp[i] = doc["list"][i]["main"]["temp"];
    obj.rain[i] = doc["list"][i]["pop"];
  }
  return obj;
}