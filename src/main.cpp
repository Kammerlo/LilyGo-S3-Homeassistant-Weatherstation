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

/**
 * First screen displays Weather Information from Homeassistant Entity. I'm using my Netatmo weather station from outside. 
 * I'm interested in the temperature, humidity and battery.
*/
void drawFirstScreen() {
  // Init screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  // Reading needed data
  OpenWeatherStruct ows = getOpenWeatherStruct();
  SensorStruct humidity = easyHA.readSensorValue(HUMIDITY_ENTITY_NETATMO);
  SensorStruct temperature = easyHA.readSensorValue(TEMPERATURE_ENTITY_NETATMO);
  SensorStruct battery = easyHA.readSensorValue(BATTERY_ENTITY_NETATMO);
  SensorStruct zentraleTemp = easyHA.readSensorValue(OFFICE_TEMPERATURE);
  const unsigned short* image = getWeatherImage(ows.iconName);
  // Display data
  tft.pushImage(0,35,96,96,image);
  int textSize = 30;
  tft.drawString(("Wetter in " + VILLAGE_NAME),5,5);
  tft.drawString("Temp:", 100, textSize + 5);
  tft.drawString((temperature.state + " C"),250, textSize + 5);
  tft.drawString("Hum:", 100, textSize * 2  + 5);
  tft.drawString((humidity.state + " %"),250, textSize * 2  + 5);
  tft.drawString("Akku:", 100, textSize * 3  + 5);
  tft.drawString((battery.state + " %"),250, textSize * 3  + 5);
  tft.drawString("Zentrale:", 100, textSize * 4  + 5);
  tft.drawString((zentraleTemp.state + " C"),250, textSize * 4  + 5);
}
/**
 * Second screen is displaying calendar events. In my case it's garbage collection in my village, since i forget sometimes to put out garbage cans.
 * This motivation is the actual reason for this project :) 
*/
void drawSecondScreen() {
  // Init screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  // Reading needed data
  CalendarStruct calEntries = getHomeassistantCalendarData("calendar.mullabfuhr_jelpke", 5);
  // Display data
  tft.drawString(("Mullabfuhr in " + VILLAGE_NAME),5,5);
  tft.pushImage(0,35,96,96,calendar);
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
/**
 * Third screen is display a forecast. Temperatures and rain probabilities. Everything is calculated to fit a 24h forecast.
*/
void drawThirdScreen() {
  // Init screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  // Reading needed data
  OpenWeatherForecast forecast = getWeatherForecast();
  // Display Data
  tft.drawString(("Forecast in " + VILLAGE_NAME),5,5);
  tft.setTextSize(1);
  int textSize = 10;
  int yStart = 35;
  int yEnd = 155;
  // drawing axis
  tft.drawString(" 40 C",0,yStart);                 // y = 35
  tft.drawString(" 30 C",0,yStart + textSize * 2);  // y = 55
  tft.drawString(" 20 C",0,yStart + textSize * 4);  // y = 75
  tft.drawString(" 10 C",0,yStart + textSize * 6);  // y = 95
  tft.drawString("  0 C",0,yStart + textSize * 8);  // y = 115
  tft.drawString("-10 C",0,yStart + textSize * 10); // y = 135
  tft.drawString("-20 C",0,yStart + textSize * 12); // y = 155
  tft.drawLine(40, yStart - 5, 40, yEnd,TFT_WHITE);
  // drawing rain charts
  // Area for Charts starts at 40, ends at 320, 35 pixels per Bar
  int barHeight, xBarStart = 40;
  for(int i = 0; i < 8; i++) {
    barHeight = 170 - int(forecast.rain[i] * 100 * 1.35); // The display height is 170, the upper text ends at 35 Pixels. So 135 Pixel left for 100% -> 1% = 1.35. Rain probability (e.g. 0.76) * 100 * Pixel-per-percent (1.35)
    tft.fillRect(xBarStart,barHeight,35,170,TFT_SKYBLUE); // change 110 to 170
    xBarStart += 35;
  }
  // drawing temperature chart
  // -20° is at 35 + 120 => 155 -> everything lower than -20° will be drawn at 170
  // 40° is at 35 -> everything higher than 40° will be drawn at 40
  int circleX = 40 + 17; // 35 pixels per step, Circle must be in the middle so divided by 2 and floor to 17 since we need integers
  int circleY, nextY;
  for(int i = 0; i < 8; i++) {
    
    if(forecast.temp[i] > 40) {
      circleY = 40;
    } else if (forecast.temp[i] < -20)
    {
      circleY = 170;
    } else {
      circleY = 160 - int((forecast.temp[i] + 20) * 2.16);
    }
    if(i < 7) {
      nextY = 160 - int((forecast.temp[i + 1] + 20) * 2.16); // drawing a line to the next circle. Could be done more effiently from a coding site, but it's fine for now.
      tft.drawLine(circleX, circleY, circleX + 35, nextY, TFT_WHITE);
    }
    tft.fillCircle(circleX, circleY, 5, TFT_WHITE);
    circleX += 35;
  }
}

void setup() {
  // Initializing display
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  pinMode(PIN_BUTTON_1,INPUT);
  pinMode(PIN_BUTTON_2,INPUT);
  tft.begin();
  tft.setRotation(rotation);
  tft.setSwapBytes(true);

  Serial.begin(9600);

  delay(2000);
  connectToWifi();
  timeClient.begin();
  configTime(180000,0,"pool.ntp.org");
  
  drawFirstScreen(); // Starting with the first screen
}

void drawCurrentScreen() {
  switch(screenNumber) {
      case 0:
        drawFirstScreen();
        break;
      case 1:
        drawSecondScreen();
        break;
      case 2:
        drawThirdScreen();
        break;
    }
}

void loop() {
  int button1 = digitalRead(PIN_BUTTON_1); // Button 1 for changing screens
  int button2 = digitalRead(PIN_BUTTON_2); // Button 2 for changing rotation
  if(!button1) {
    screenNumber = (screenNumber + 1) % 3; // currently there are two different screens
    drawCurrentScreen();
  }
  if(!button2) {
    rotation = (rotation + 2) % 4; // Changing between 1 and 3, because we only implemented landscape
    Serial.println(rotation);
    tft.setRotation(rotation);
    drawCurrentScreen();
  }
  delay(100);
}
