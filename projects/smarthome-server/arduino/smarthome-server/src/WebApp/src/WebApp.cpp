#include "string"
#include <WiFi.h>

#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "WebApp.h"
#include "ControllerManagement/src/ControllerManagement.h"

void WebApp::handleUIRouting()
{
    // Pages
    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(SPIFFS, "/index.html", String(), false); });

    _server->on("/404", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(SPIFFS, "/404.html", String(), false); });

    // Asset management
    _server->serveStatic("/global.css", SPIFFS, "/global.css");
    _server->serveStatic("/output/", SPIFFS, "/output/");
}

void WebApp::handleServiceRouting()
{
    _server->on("/api/get-controller-list", HTTP_GET, [](AsyncWebServerRequest *request) {
        ControllerManagement controllerManagement;

        String foo = controllerManagement.getControllerList();
        
        request->send(200, "text/plain", foo);
    });

    _server->on("/api/add-controller", HTTP_POST,
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

            ControllerManagement controllerManagement;
            Response response = controllerManagement.addController(request, doc, json);

            request->send(response.statusCode, "text/plain", response.message);
        });
}

void WebApp::handleClient(AsyncWebServer *server)
{
    _server = server;

    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(SPIFFS, "/index.html", String(), false); });

    handleUIRouting();
    handleServiceRouting();
}
