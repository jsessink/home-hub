#include <string>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#ifndef Web_App
#define Web_App

class WebApp {
    private:
        AsyncWebServer *_server;
        unsigned long currentTime = millis();
        unsigned long previousTime = 0;
        const long timeoutTime = 2000;

        void handleUIRouting();
        void handleServiceRouting();
        static void onRGBAChange();

    public:
        WiFiClient client;
        
        void handleClient(AsyncWebServer *server);
        void initServer();
};

#endif