#ifndef Settings_H
#define Settings_H

class ColorSettings {
    public: 
        int colorR;
        int colorG;
        int colorB;
};

class PIRSettings {
    public:
        bool alwaysOn;
        unsigned short accidentalTripDelay;
        unsigned short durationOn;
        unsigned short fadeInSpeed;
        unsigned short fadeOutSpeed;
};

class Settings: public ColorSettings, public PIRSettings {};

#endif
