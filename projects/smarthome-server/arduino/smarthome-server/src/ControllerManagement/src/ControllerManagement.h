#include "string"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include "HTTP/src/Response.h"

#ifndef Controller_Management_H
#define Controller_Management_H

class ControllerManagement {
    public:
        StaticJsonDocument<512> getControllerList();

    private:    
};

#endif
