#include "string"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include "HTTP/src/Response.h"

#ifndef Controller_Management_H
#define Controller_Management_H

class ControllerManagement {
    public:
        Response addController(AsyncWebServerRequest *request, StaticJsonDocument<200> doc, char* json);
        String getControllerList();

    private:    
};

#endif
