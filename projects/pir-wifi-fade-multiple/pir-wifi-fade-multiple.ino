#include <WiFi.h>
#include <LEDFader.h>

#include "Led.h"
#include "C:\Users\joshua.m.sessink\Desktop\Arduino\_projects\_shared\WiFiConfig.h"

#define maxBrightness 25
#define fadeDuration 2000 // Time to fully fade in
#define delayIncrement 200 // Delay time between each set of LEDs to fade in
#define durationOn 20000
#define manualDurationOn 60000

const int serverPort = 80;

//// LEDFader
#define LED_NUM 3

// 6 LEDs (perhaps 2 RGB LEDs)
LEDFader leds[LED_NUM] = {
  LEDFader(27),
  LEDFader(33),
  LEDFader(32)
};


const int pirPin = 14;
unsigned int pirState = LOW;
unsigned int pirReadVal = 0;

// To pause other listeners on loop
volatile bool fadeInLEDs = false;
volatile bool fadeOutLEDs = false;
volatile static int fadeCore = 0; // Core 1 (by index)

// Time when the motion detection triggered
volatile unsigned long triggerTime = 0;

// Manually triggered for condition overrides
volatile bool manuallyOn = false;
volatile bool manuallyOff = false;

WiFiServer server(serverPort);

void initWiFi() {
  WiFi.begin(SSID, WiFiPassword);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
}


void listenWiFiClient() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {
    Serial.println("Connection over WiFi made.");
    String currentLine = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
//            client.println("HTTP/1.1 200 OK");
//            client.println("Content-type:text/html");
//            client.println();
//
//            String onOrOff = (
//              led0.getBrightness() > 0
//              && led1.getBrightness() > 0
//              && led2.getBrightness() > 0
//            ) ? "on" : "off";
//
//            client.print("<h4>Your lights are currently: " + onOrOff + "</h4>");
//            client.print("<button onclick=\"location.href='/on'\">Manually Turn On</button>");
//            client.print("<br/><br/>");
//            client.print("<button onclick=\"location.href='/off'\">Manually Turn Off</button>");
//            
//            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /on")) {
          Serial.println("Request to manually turn on lights received.");
          
//          led0.fadeIn(millis());
//          led1.fadeIn(millis());
//          led2.fadeIn(millis());
          
          pirState = HIGH;
          triggerTime = millis();
          manuallyOff = false;
          manuallyOn = true;
        }
        else if (currentLine.endsWith("GET /off")) {
          Serial.println("Request to manually turn off lights received.");
          
//          led0.fadeOut();
//          led1.fadeOut();
//          led2.fadeOut();
          
          pirState = LOW;
          manuallyOn = false;
          manuallyOff = true;
        }
      }
    }

    client.stop();
  }
}

TaskHandle_t FadeTask;
void fadeTask( void * parameters ) {
  for(;;) {    
    if(fadeInLEDs || fadeOutLEDs) {      
      for (byte i = 0; i < LED_NUM; i++) {
        LEDFader *led = &leds[i];
        led->update();
    
         if (millis() >= triggerTime + (i * delayIncrement)) {
          if (fadeInLEDs) {
            if (led->get_value() == 0) {
              led->fade(maxBrightness, fadeDuration);
            }
          } else {
            led->fade(0, fadeDuration);
          }
         }
      }
  
      if (triggerTime + (LED_NUM * delayIncrement) + fadeDuration  < millis()) {
        fadeInLEDs = false;
        fadeOutLEDs = false;
      }
    }
  }
}


void setup() {  
  Serial.begin(9600);

//  initWiFi();
//  Serial.print("RRSI: ");
//  Serial.println(WiFi.RSSI());
  
  pinMode(pirPin, INPUT);

  // Pin fade effect to core 2 so core 1 can focus on listening for WiFi and PIR
//  xTaskCreatePinnedToCore(
//    fadeTask,
//    "fade",
//    10000,
//    NULL,
//    1,
//    &FadeTask,
//    fadeCore
//  );
}


void loop() {
  pirReadVal = digitalRead(pirPin);
  listenWiFiClient();

  if(fadeInLEDs || fadeOutLEDs) {      
    for (byte i = 0; i < LED_NUM; i++) {
      LEDFader *led = &leds[i];
      led->update();
  
       if (millis() >= triggerTime + (i * delayIncrement)) {
        if (fadeInLEDs) {
          if (led->get_value() == 0) {
            led->fade(maxBrightness, fadeDuration);
          }
        } else {
          led->fade(0, fadeDuration);
        }
       }
    }

    if (triggerTime + (LED_NUM * delayIncrement) + fadeDuration  < millis()) {
      fadeInLEDs = false;
      fadeOutLEDs = false;
    }
  }

  if (pirReadVal == HIGH) {
    if (pirState == LOW && !manuallyOff) {
      triggerTime = millis();
      fadeInLEDs = true;
      pirState = HIGH;
    }
  } 
  else {
    if (pirState == HIGH) {
//      // Wait defined duration (from the time of trigger) then fade out and switch pirState to allow a next fade in
//      if(millis() > triggerTime + durationOn && !manuallyOn){
//        Serial.println("No motion for duration, fading out");
//      } else if (manuallyOn && millis() > triggerTime + manualDurationOn) {
//        manuallyOn = false;
//        Serial.println("Manual trigger duration completed and no further motion detected, fading out");
//      }

      if(millis() > triggerTime + durationOn && !manuallyOn){
        triggerTime = millis();
        fadeOutLEDs = true;
        pirState = LOW;
      }
    }
  }
}
