#include <Arduino.h>
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "img_logo.h"
#include "pin_config.h"
#include <EasyHA.h>
#include "helperFunctions.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
 * switch macro. */
#define WAIT 100
const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
EasyHA easyHA(HA_BASE_URL,HA_TOKEN);
TFT_eSPI tft = TFT_eSPI();
int screenNumber = 0;
int rotation = 1;

CalendarStruct getHomeassistantCalendarData(String entity, int endDateOffsetDays) {
  char startDate[20];
  char endDate[20];
  struct tm startTime;
  struct tm endTime;
  getLocalTime(&startTime);
  strftime(startDate,sizeof(startDate),"%Y-%m-%dT00:00:00", &startTime);
  time_t time = mktime(&startTime);
  time += endDateOffsetDays * 86400;
  
  strftime(endDate,sizeof(endDate),"%Y-%m-%dT00:00:00", localtime(&time));
  CalendarStruct calendarstruct = easyHA.getCalendarEntries(entity, String(startDate), String(endDate), 1536);
  return calendarstruct;
}

void drawFirstScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  // Reading needed data
  OpenWeatherStruct ows = getOpenWeatherStruct();
  SensorStruct humidity = easyHA.readSensorValue("wohnzimmer_wohnzimmer_wohnzimmer_draussen_humidity");
  SensorStruct temperature = easyHA.readSensorValue("wohnzimmer_wohnzimmer_wohnzimmer_draussen_temperature");
  SensorStruct battery = easyHA.readSensorValue("wohnzimmer_wohnzimmer_wohnzimmer_draussen_battery_percent");
  SensorStruct zentraleTemp = easyHA.readSensorValue("zentrale_sensor_mit_display_temperature_2");
  const unsigned short* image = getWeatherImage(ows.iconName);
  // Display data
  tft.pushImage(0,35,96,96,image);
  int textSize = 30;
  tft.drawString(("Wetter in Jelpke"),5,5);
  tft.drawString("Temp:", 100, textSize + 5);
  tft.drawString((temperature.state + " C"),250, textSize + 5);
  tft.drawString("Hum:", 100, textSize * 2  + 5);
  tft.drawString((humidity.state + " %"),250, textSize * 2  + 5);
  tft.drawString("Akku:", 100, textSize * 3  + 5);
  tft.drawString((battery.state + " %"),250, textSize * 3  + 5);
  tft.drawString("Zentrale:", 100, textSize * 4  + 5);
  tft.drawString((zentraleTemp.state + " C"),250, textSize * 4  + 5);
}

void drawSecondScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(("Mullabfuhr in Jelpke"),5,5);
  tft.pushImage(0,35,96,96,calendar);
  CalendarStruct calEntries = getHomeassistantCalendarData("calendar.mullabfuhr_jelpke", 5);
  int textSize = 30;
  int y, columnCounter = 0;;
  for(int i = 0; i < calEntries.entries; i++) {
    String startDate = customDateSplit(calEntries.start[i]);
    y = textSize * columnCounter + textSize + 5;
    columnCounter += 1;
    tft.drawString(startDate, 100, y);
    y = textSize * columnCounter + textSize + 5;
    columnCounter += 1;
    String summary = calEntries.summary[i];
    summary.replace("ü","u");
    tft.drawString(summary, 100, y);
  }
}

void setup() {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  pinMode(PIN_BUTTON_1,INPUT);
  pinMode(PIN_BUTTON_2,INPUT);

  Serial.begin(115200);

  tft.begin();
  tft.setRotation(rotation);
  tft.setSwapBytes(true);
  
  delay(2000);
  connectToWifi();
  timeClient.begin();
  configTime(180000,0,"pool.ntp.org");
  
  drawFirstScreen(); // Starting with the first screen
}

void loop() {
  int button1 = digitalRead(PIN_BUTTON_1); // Button 1 for changing screens
  int button2 = digitalRead(PIN_BUTTON_2); // Button 2 for changing rotation
  if(!button1) {
    screenNumber = (screenNumber + 1) % 2; // currently there are two different screens

    switch(screenNumber) {
      case 0:
        drawFirstScreen();
        break;
      case 1:
        drawSecondScreen();
        break;
    }
  }
  if(!button2) {
    rotation = (rotation + 2) & 4; // Changing between 1 and 3, because we only implemented landscape
    tft.setRotation(rotation);
  }
  delay(100);
}
