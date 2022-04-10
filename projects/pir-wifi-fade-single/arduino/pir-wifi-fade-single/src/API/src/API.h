#include <string>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "Settings/src/Settings.h"

#ifndef API_h
#define API_h

class API {
    private:
        AsyncWebServer *_server;
        unsigned long currentTime = millis();
        unsigned long previousTime = 0;
        const long timeoutTime = 2000;

        static void onRGBAChange();

    public:
        WiFiClient client;
        void handleServiceRouting(AsyncWebServer *server);
        Settings initGetSettings();

};

#endif