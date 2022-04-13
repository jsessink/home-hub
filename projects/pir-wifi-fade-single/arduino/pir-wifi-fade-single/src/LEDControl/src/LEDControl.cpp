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

void LEDControl::handleLEDRequest(
    int r,
    int g,
    int b
)
{
    ledcWrite(redChannel, r);
    ledcWrite(greenChannel, g);
    ledcWrite(blueChannel, b);
}

void LEDControl::handleColorChange(ColorSettings currentSettings, ColorSettings newSettings)
{
    // Set color to lights w/ fade effect
    int rValDirection = currentSettings.colorR < newSettings.colorR ? 1 : -1;
    int gValDirection = currentSettings.colorG < newSettings.colorG ? 1 : -1;
    int bValDirection = currentSettings.colorB < newSettings.colorB ? 1 : -1;

    Serial.println("Attempting to change the color of the lights.");

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

        delay(1);
    } while (
        currentSettings.colorR != newSettings.colorR ||
        currentSettings.colorG != newSettings.colorG ||
        currentSettings.colorB != newSettings.colorB
    );
    
    Serial.println("All set!");

    return;
}

void LEDControl::initBootedSettings(Settings settings)
{
    // Fade in RGB values together
    unsigned char currentVal = 0;
    do
    {
        // If all colors are set (could be 0), then get out
        if (
            currentVal == settings.colorR &&
            currentVal == settings.colorG &&
            currentVal == settings.colorB
        )
        {
            break;
        }
        
        currentVal += 1;

        // If the currentVal is below or at the brightness previously set, write it
        if (currentVal <= settings.colorR)
        {
            ledcWrite(redChannel, currentVal);
        }

        if (currentVal <= settings.colorG)
        {
            ledcWrite(greenChannel, currentVal);
        }

        if (currentVal <= settings.colorB)
        {
            ledcWrite(blueChannel, currentVal);
        }
        
        // TODO: Maybe use the highest value from RGB instead of max/white?
        delay(settings.fadeInSpeed / 255);
    } while (currentVal < 255);

    Serial.println("Done with default settings.");
    Serial.println(currentVal);
}