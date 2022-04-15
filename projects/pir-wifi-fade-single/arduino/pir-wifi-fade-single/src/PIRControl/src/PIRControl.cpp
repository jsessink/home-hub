#include <Arduino.h>
#include <Preferences.h>

#include "API/src/API.h"
#include "PIRControl.h"
#include "LEDControl/src/LEDControl.h"
#include "Settings/src/Settings.h"

TaskHandle_t pirListen;
void pirListener(void *params)
{
  PIRControl pirControl;
  pinMode(pirControl.pin, INPUT);

  API api;

  // Always assume we need to turn the lights off after init, since the main turns it on right away after setup
  pirControl.pirState = HIGH;
  pirControl.tripDelayActive = false;
  pirControl.tripDelayTriggerTime = millis();
  pirControl.triggerTime = millis();

  LEDControl ledControl;

  // Off settings we can keep here
  Settings offSettings = api.initGetSettings();
    offSettings.colorR = 0;
    offSettings.colorG = 0;
    offSettings.colorB = 0;

  // On settings default to existing stored value since they turn on immediately after setup,
  //  but then we need to pull in the loop in case they change between motions by the user.
  Settings latestOnSettings = api.initGetSettings();

  for (;;)
  {
    pirControl.currentPIRVal = digitalRead(pirControl.pin);

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
            millis() > pirControl.tripDelayTriggerTime + latestOnSettings.accidentalTripDelay)
        {
          latestOnSettings = api.initGetSettings();
          ledControl.handleColorChange(offSettings, latestOnSettings);

          pirControl.pirState = HIGH;
          pirControl.tripDelayActive = false;
        }
        else if (!pirControl.tripDelayActive)
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
        if (millis() > pirControl.triggerTime + latestOnSettings.durationOn)
        {
          ledControl.handleColorChange(latestOnSettings, offSettings);

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
      0);
}
