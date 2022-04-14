#include <Arduino.h>
#include <Preferences.h>

#include "PIRControl.h"
#include "Settings/src/Settings.h"

TaskHandle_t pirListen;
void pirListener(void *params)
{
  PIRControl pirControl;
  pinMode(pirControl.pin, INPUT);

  Settings settings;
  Preferences preferences;
  preferences.begin("pir-settings", true);
  settings.alwaysOn = preferences.getBool("always-on");
  settings.accidentalTripDelay = preferences.getUShort("accidental-trip-delay");
  settings.durationOn = preferences.getUShort("duration-on");
  preferences.end();

  if (settings.alwaysOn)
  {
    Serial.println("Always on enabled, running the ON and moving along.");
    // Do a fade in immediately, as it'll be controlled by a light switch

    // Return to avoid the loop on this thread
    return;
  }

  pirControl.pirState = LOW;
  pirControl.tripDelayActive = false;
  pirControl.tripDelayTriggerTime = millis();
  pirControl.triggerTime = millis();

  Serial.println("Starting loop on core 0");
  for (;;)
  {
    pirControl.currentPIRVal = digitalRead(pirControl.pin);
    Serial.println(pirControl.currentPIRVal);

    if (pirControl.currentPIRVal == HIGH)
    {
      // Always update trigger time to track when we need to fade back out after no motion (not just after fade in)
      pirControl.triggerTime = millis();

      // If the state previous to this was LOW, then set to HIGH as it is a new detection
      if (pirControl.pirState == LOW)
      {
        // Check if the trip deplay detection is active, and if so see if criteria was met for the delay
        if (
          pirControl.tripDelayActive &&
          millis() > pirControl.tripDelayTriggerTime + settings.accidentalTripDelay
        )
        {
          Serial.println("Motion detected!");
          // Do something
          pirControl.pirState = HIGH;
          pirControl.tripDelayActive = false;
        }
        else 
        {
          pirControl.tripDelayTriggerTime = millis();
          pirControl.tripDelayActive = true;
        }  
      }
    }
    else
    {
      // If the existing PIR State is HIGH, but the incoming is LOW,
      //  then we lost motion and should take action after our duration criteria is met
      if (pirControl.pirState == HIGH)
      {
        // Wait defined duration (from the time of trigger) then fade out and switch pirState to allow a next fade in
        if (millis() > pirControl.triggerTime + settings.durationOn)
        {
          Serial.println("No motion for duration");
          // Do something
          pirControl.pirState = LOW;
        }
      }
    }

    // Only do this because it's not on the main core
    delay(100);
  }
}

void PIRControl::pirSetup()
{
  xTaskCreatePinnedToCore(
    pirListener,
    "pirListen",
    10000,
    NULL,
    1,
    &pirListen,
    0
  );
}
