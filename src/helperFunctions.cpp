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

OpenWeatherStruct getOpenWeatherStruct() {
    StaticJsonDocument<1024> doc;
    // put your main code here, to run repeatedly:
    String payload = httpCall(OPENWEATHER_URL);
    // deserializeJson nimmt auch den httpClient - Hier eine Möglichkeit Speicher zu sparen.
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

haSensorStruct getHaSensorStruct(String entity){
  StaticJsonDocument<768> doc;
  String payload = httpCall(HA_BASE_URL + entity);

  // deserializeJson nimmt auch den httpClient - Hier eine Möglichkeit Speicher zu sparen.
    DeserializationError error = deserializeJson(doc, payload);
    haSensorStruct currentSensorData;
    if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return currentSensorData;
    }
    currentSensorData.state = doc["state"].as<String>();
    currentSensorData.unit = doc["attributes"]["unit_of_measurement"].as<String>();
    return currentSensorData;
}

OpenWeatherForecast getWeatherForecast() {
  String url = "http://api.openweathermap.org/data/2.5/forecast?lat=52.3830785&lon=10.657896&appid=f6fefa3ba2838f33fae50a289a04c2ad&units=metric&cnt=8";
  String payload = httpCall(url);
  StaticJsonDocument<6144> doc;
  OpenWeatherForecast obj;
  DeserializationError error = deserializeJson(doc,payload);
  if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return obj;
  }
  JsonArray list = doc["list"];
  for(int i = 0; i <= 7; i++) {
    obj.temp[i] = list[i]["main"]["temp"];
    obj.rain[i] = list[i]["pop"];
  }
  return obj;
}