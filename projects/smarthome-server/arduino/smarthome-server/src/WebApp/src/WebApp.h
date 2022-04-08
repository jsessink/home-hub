#include <string>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#ifndef WebApp_h
#define WebApp_h

class WebApp {
    private:
        AsyncWebServer *_server;

        void handleUIRouting();
        void handleServiceRouting();

    public:
        WiFiClient client;
        
        void handleClient(AsyncWebServer *server);
};

#endif