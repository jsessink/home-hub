#include <string>
#include <ESPAsyncWebServer.h>

#ifndef AccessPointAPI_h
#define AccessPointAPI_h

class AccessPointAPI {
    private:
        AsyncWebServer *_server;

      public:        
        void handleAPServiceRouting(AsyncWebServer *server);
};

#endif