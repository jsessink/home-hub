#include <string>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "Settings/src/Settings.h"
#include "Response.h"
#include <DeviceList.h>

#ifndef API_h
#define API_h

using namespace DeviceList;

class API {
    private:
        AsyncWebServer *_server;

        unsigned long currentTime = millis();
        unsigned long previousTime = 0;
        const long timeoutTime = 2000;

        static void changeColor(ColorSettings updatedColorSettings);

    public:
        WiFiClient client;
        void handleServiceRouting(AsyncWebServer *server);
        ColorSettings getCurrentColorSettings();
        Settings initGetSettings();

};

#endif