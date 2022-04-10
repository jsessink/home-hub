#include <Arduino.h>

#include "LEDControl.h"
#include "Settings/src/Settings.h"

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