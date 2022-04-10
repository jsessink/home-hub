#include "Settings/src/Settings.h"

#ifndef LED_Control_H
#define LED_Control_H

class LEDControl {
    // Red, green, and blue pins for PWM control
    const int redPin = 27;   // GPIO27 / D27
    const int greenPin = 33; // GPIO33 / D33
    const int bluePin = 32;  // GPIO32 / D32

    // Setting PWM frequency, channels and bit resolution
    const int freq = 5000;
    const int redChannel = 0;
    const int greenChannel = 1;
    const int blueChannel = 2;

    // Bit resolution 2^8 = 256
    const int resolution = 8;

    public:
        void ledSetup();
        void handleLEDRequest(
            int r,
            int g,
            int b
        );
        void initBootedSettings(Settings settings);
};

#endif
