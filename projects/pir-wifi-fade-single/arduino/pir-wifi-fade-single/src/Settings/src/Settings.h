#ifndef Settings_H
#define Settings_H

class Settings {
    public: 
        unsigned char colorR;
        unsigned char colorG;
        unsigned char colorB;

        bool alwaysOn;
        unsigned short accidentalTripDelay;
        unsigned short durationOn;
        unsigned short fadeInSpeed;
        unsigned short fadeOutSpeed;
};

#endif
