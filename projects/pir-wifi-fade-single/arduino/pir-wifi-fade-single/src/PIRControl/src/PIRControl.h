#include <Arduino.h>
#include "Settings/src/Settings.h"

#ifndef PIR_Control_H
#define PIR_Control_H

class PIRControl {
    public:
        const int pin = 14; // GPIO14 / D14

        unsigned int pirState;
        unsigned int currentPIRVal; // Listens on loop
        unsigned long triggerTime;
        unsigned long tripDelayTriggerTime;
        bool tripDelayActive;

        void pirSetup();

    private:
};

#endif
