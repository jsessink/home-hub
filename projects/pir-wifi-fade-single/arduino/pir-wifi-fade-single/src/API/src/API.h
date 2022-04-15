#ifndef API_h
#define API_h

#include <string>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "Settings/src/Settings.h"
#include "Response.h"

class API {
    private:
        AsyncWebServer *_server;
        static void changeColor(Settings updatedSettings);
        static void changeSetting(String settingName, String settingValue, String type);

      public:
        WiFiClient client;
        void handleServiceRouting(AsyncWebServer *server);
        Settings initGetSettings();
};

#endif