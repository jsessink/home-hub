#include <WiFi.h>

#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

#include "LEDControl/src/LEDControl.h"
#include "API.h"
#include "Settings/src/Settings.h"

Preferences preferences;

void API::onRGBAChange()
{
    Serial.println("ok then");
}

// Gets settings from flash memory
// Initializes new defaults if no settings are present
Settings API::initGetSettings()
{
    Settings settings;

    // Start with RGB settings
    preferences.begin("rgb-settings", false);

    preferences.clear();

    // Get color values, set default to a little off-white if none present.
    settings.colorR = preferences.getUChar("color-r", 255);
    settings.colorG = preferences.getUChar("color-g", 225);
    settings.colorB = preferences.getUChar("color-b", 210);

    preferences.end();

    // Now get PIR settings
    preferences.begin("pir-settings", false);

    settings.alwaysOn = preferences.getBool("always-on", false);
    settings.accidentalTripDelay = preferences.getUShort("accidental-trip-delay", 2000);
    settings.durationOn = preferences.getUShort("duration-on", 10000);
    settings.fadeInSpeed = preferences.getUShort("fade-in-speed", 1000);
    settings.fadeOutSpeed = preferences.getUShort("fade-out-speed", 1250);

    preferences.end();

    return settings;
}

void API::handleServiceRouting(AsyncWebServer *server)
{
    _server = server;

    _server->on("/api/rgba-change", HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            // Handle basic requests without a body
        },
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
        {
            // Handle file uploads
        },
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            // Handle requests with body -- https://github.com/me-no-dev/ESPAsyncWebServer/issues/195
            // Serial.println(String("data=") + (char*)data);
            char* json = (char*)data;

             // Deserialize the JSON document
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, json);

            // Test if parsing succeeds.
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            int r = doc["r"];
            int g = doc["g"];
            int b = doc["b"];
            
            LEDControl ledControl;
            ledControl.handleLEDRequest(r, g, b);

            request->send(200);
        }
    );

    _server->on("/api/update-settings", HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            // Handle basic requests without a body
        },
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
        {
            // Handle file uploads
        },
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            // Handle requests with body -- https://github.com/me-no-dev/ESPAsyncWebServer/issues/195
            Serial.println(String("data=") + (char*)data);
            char* json = (char*)data;

             // Deserialize the JSON document
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, json);

            // Test if parsing succeeds.
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            
            Serial.println("Received JSON!");

            request->send(200);
        }
    );
}
