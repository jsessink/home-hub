#include<iostream>
#include<string>
using namespace std;

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include <Preferences.h>

#include "API.h"
#include "Settings/src/Settings.h"
#include "Response.h"
#include "LEDControl/src/LEDControl.h"
#include "DeviceSettings.h"

void API::changeColor(Settings updatedSettings)
{
    Preferences preferences;
    Response response;

    preferences.begin("rgb-settings", false);

    // Set color to lights w/ fade effect
    API api;
    Settings currentSettings = api.initGetSettings();

    // Send settings to the chip
    LEDControl ledControl;
    ledControl.handleColorChange(currentSettings, updatedSettings);

    // Update settings preferences   
    preferences.putUChar("color-r", updatedSettings.colorR);
    preferences.putUChar("color-g", updatedSettings.colorG);
    preferences.putUChar("color-b", updatedSettings.colorB);

    preferences.end();

    return;
}

void API::changeSetting(String settingName, String settingValue, String type)
{
    Preferences preferences;
    Response response;

    preferences.begin("pir-settings", false);

    // Update settings preferences
    if (type == "bool")
    {
      preferences.putBool(settingName.c_str(), settingValue == "true");
    }
    else
    {
      preferences.putUShort(settingName.c_str(), settingValue.toInt());
    }

    preferences.end();
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
    settings.accidentalTripDelay = preferences.getUShort("acc-delay", 2000);
    settings.durationOn = preferences.getUShort("duration-on", 10000);
    settings.fadeInSpeed = preferences.getUShort("in-speed", 2000);
    settings.fadeOutSpeed = preferences.getUShort("out-speed", 2500);

    preferences.end();

    return settings;
}

void API::handleServiceRouting(AsyncWebServer *server)
{
    _server = server;

    _server->on("/api/get-device-status", HTTP_GET, [](AsyncWebServerRequest *request) {
      DeviceSettings deviceSettings;
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument jsonSettings(200);
      jsonSettings["controllerDeviceName"] = deviceSettings.activeDevice;
      serializeJson(jsonSettings, *response);

      request->send(response);
      return;
    });

    _server->on("/api/get-current-rgb-settings", HTTP_GET, [](AsyncWebServerRequest *request) {
      API api;
      DeviceSettings deviceSettings;
      Settings settings = api.initGetSettings();

      AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument json(1024);
        json["deviceName"] = deviceSettings.getActiveDeviceName();
        json["r"] = settings.colorR;
        json["g"] = settings.colorG;
        json["b"] = settings.colorB;
        json["alwaysOn"] = settings.alwaysOn;
        json["accidentalTripDelay"] = settings.accidentalTripDelay;
        json["durationOn"] = settings.durationOn;
        json["fadeInSpeed"] = settings.fadeInSpeed;
        json["fadeOutSpeed"] = settings.fadeOutSpeed;

      serializeJson(json, *response);

      request->send(response);
      return;
    });

    _server->on("/api/rgb-change", HTTP_GET, [](AsyncWebServerRequest *request) {
        String r = request->getParam("r")->value();
        int r_num = atoi(r.c_str());
        String g = request->getParam("g")->value();
        int g_num = atoi(g.c_str());
        String b = request->getParam("b")->value();
        int b_num = atoi(b.c_str());

        Settings updatedSettings;
            updatedSettings.colorR = r_num;
            updatedSettings.colorG = g_num;
            updatedSettings.colorB = b_num;

        API::changeColor(updatedSettings);

        Response response;
        response.statusCode = 200;
        response.message = "Updated controller and flash preferences.";

        request->send(response.statusCode, String(), response.message);
        return;
    });

    _server->on("/api/rgb-setting-change", HTTP_GET, [](AsyncWebServerRequest *request) {
        String settingName = request->getParam("setting-name")->value();
        String settingValue = request->getParam("setting-value")->value();

        String updatedSettingType;

        if (settingName == "always-on")
        {
          bool boolVal;
          istringstream(settingValue.c_str()) >> boolVal;
          updatedSettingType = "bool";
        }
        else
        {
          updatedSettingType = "UShort";
        }

        API::changeSetting(settingName, settingValue, updatedSettingType);

        Response response;
        response.statusCode = 200;
        response.message = "Updated controller and flash preferences.";

        request->send(response.statusCode, String(), response.message);
        return;
    });
}
