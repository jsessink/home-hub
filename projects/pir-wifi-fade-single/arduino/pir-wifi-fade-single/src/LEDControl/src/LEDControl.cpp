#include <Arduino.h>

#include "LEDControl.h"
#include "Settings/src/Settings.h"
#include "API/src/Response.h"

void LEDControl::ledSetup()
{
    // configure LED PWM functionalities
    ledcSetup(redChannel, freq, resolution);
    ledcSetup(greenChannel, freq, resolution);
    ledcSetup(blueChannel, freq, resolution);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(redPin, redChannel);
    ledcAttachPin(greenPin, greenChannel);
    ledcAttachPin(bluePin, blueChannel);
}

void LEDControl::handleColorChange(Settings currentSettings, Settings newSettings)
{
    // Set color to lights w/ fade effect
    int rValDirection = currentSettings.colorR < newSettings.colorR ? 1 : -1;
    int gValDirection = currentSettings.colorG < newSettings.colorG ? 1 : -1;
    int bValDirection = currentSettings.colorB < newSettings.colorB ? 1 : -1;

    do
    {
        if (currentSettings.colorR != newSettings.colorR) {
            currentSettings.colorR += rValDirection;
            ledcWrite(redChannel, currentSettings.colorR);
        }

        if (currentSettings.colorG != newSettings.colorG) {
            currentSettings.colorG += gValDirection;
            ledcWrite(greenChannel, currentSettings.colorG);
        }

        if (currentSettings.colorB != newSettings.colorB) {
            currentSettings.colorB += bValDirection;
            ledcWrite(blueChannel, currentSettings.colorB);
        }

        delay(newSettings.fadeInSpeed / 255);
    } while (
        currentSettings.colorR != newSettings.colorR ||
        currentSettings.colorG != newSettings.colorG ||
        currentSettings.colorB != newSettings.colorB
    );

    return;
}
