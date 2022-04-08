#include <Arduino.h>

#include "LEDControl.h"

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