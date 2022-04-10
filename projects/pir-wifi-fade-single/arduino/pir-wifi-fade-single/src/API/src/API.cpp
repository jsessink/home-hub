#include<iostream>
#include<string>
using namespace std;

#include <WiFi.h>

#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

#include "API.h"
#include "Settings/src/Settings.h"
#include "Response.h"
#include "LEDControl/src/LEDControl.h"

void API::changeColor(ColorSettings colorSettings)
{
    Preferences preferences;
    Response response;

    Serial.println("Attempting to update preferences.");

    preferences.begin("rgb-settings", false);

    // Set color to lights w/ fade effect
    ColorSettings currentColorSettings;
        currentColorSettings.colorR = preferences.getUChar("color-r");
        currentColorSettings.colorG = preferences.getUChar("color-g");
        currentColorSettings.colorB = preferences.getUChar("color-b");

    // Send settings to the chip
    LEDControl ledControl;
    ledControl.handleColorChange(currentColorSettings, colorSettings);

    // Update settings preferences   
    preferences.putUChar("color-r", colorSettings.colorR);
    preferences.putUChar("color-g", colorSettings.colorG);
    preferences.putUChar("color-b", colorSettings.colorB);

    preferences.end();

    Serial.println("Preferences updated.");

    return;
}

// Gets settings from flash memory
// Initializes new defaults if no settings are present
Settings API::initGetSettings()
{
    Preferences preferences;
    Settings settings;

    // Start with RGB settings
    preferences.begin("rgb-settings", false);

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

    _server->on("/api/rgb-change", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Color change request received!");

        String r = request->getParam("r")->value();
        int r_num = atoi(r.c_str());
        String g = request->getParam("g")->value();
        int g_num = atoi(g.c_str());
        String b = request->getParam("b")->value();
        int b_num = atoi(b.c_str());

        ColorSettings colorSettings;
            colorSettings.colorR = r_num;
            colorSettings.colorG = g_num;
            colorSettings.colorB = b_num;

        Serial.println("Attempting to change the color.");
        Serial.println("Incoming red: ");
        Serial.println(colorSettings.colorR);
        Serial.println("Incoming green: ");
        Serial.println(colorSettings.colorG);
        Serial.println("Incoming blue: ");
        Serial.println(colorSettings.colorB);

        API::changeColor(colorSettings);

        Serial.println("Setting a response to send back: ");
        Response response;
        response.statusCode = 200;
        response.message = "Updated controller and flash preferences.";
        Serial.print(response.message);

        request->send(response.statusCode, String(), response.message);
        return;
    });

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
