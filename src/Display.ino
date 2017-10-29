#include "TideDisplay.h"
#include <ThingSpeak.h>

#define PIN 7

// ********** Device Parameters **********
const int displayHeight = 78; // tide display height in inches
const int numDisplayPixels = 278; // number of pixels on tide display

// ********** NON CONFIGURABLE Parameters **********
TCPClient client; // client for Thingspeak connection
char server[] = "api.thingspeak.com"; // url for Thingspeak connection
TideDisplay display = TideDisplay(numDisplayPixels, PIN, displayHeight);

Timer updateDisplay(5000, displayUpdate);
void displayUpdate()
{
    display.bubble();
}

Timer takeReadings(20000, readingUpdate);
void readingUpdate()
{
    connectToThingspeak();
}

void connectToThingspeak()
{
    int heightReading = ThingSpeak.readStringField(22641, 1).toInt();
    double nextTideReading = ThingSpeak.readStringField(131258, 1).toFloat();
    int dirReading = ThingSpeak.readStringField(47960, 1).toInt();
    
    if (heightReading != NULL && dirReading != NULL && nextTideReading != NULL) {
        double tideHeight = ((3449 - heightReading) / 24.5);
        double nextTide = (nextTideReading + 1) * 12;
        bool tideRising = (dirReading == -1 ? false : true);

        display.heightUpdate(tideHeight, nextTide, tideRising);
        
        //display.showStatus(true); // uncomment to show green LED with successful connection
    } else {
        display.showStatus(false);
    }
}

void setup()
{
    Serial.begin(9600);
    delay(500);
    
    ThingSpeak.begin(client);
    display.start();
    
    updateDisplay.start();
    takeReadings.start();
    
    readingUpdate();
}

void loop()
{
    
}