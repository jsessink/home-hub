#include "string"
#include <sstream>
#include <WiFi.h>
#include <ESP32Ping.h>

#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <HTTPClient.h>

#include "WebApp.h"
#include "ControllerManagement/src/ControllerManagement.h"

bool WebApp::pingRemote(String url)
{
    // Ping controller to see if it exists
    bool pingHostnameSuccess = Ping.ping(url.c_str(), 3);
    return pingHostnameSuccess;
}

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
    _server->on("/api/proxy-change/rgb", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received request to change color");

        // TODO: Set by controller type
        // Pass the body to the controller
        String ip = request->getParam("ip")->value();
        String r = request->getParam("r")->value();
        String g = request->getParam("g")->value();
        String b = request->getParam("b")->value();

        // Ping remote controller to ensure it is ready to GET
        bool isReady = WebApp::pingRemote(ip);

        if (!isReady)
        {
            Response response;
            response.message = "Remote device not ready to receive input";
            response.statusCode = 500;
            request->send(response.statusCode, String(), response.message);
            return;
        }

        Serial.println("ip:" + ip);
        Serial.println("rgb:" + r + g + b);

        String endpoint = ip + "/api/rgb-change?r=" + r + "&g=" + g + "&b=" + b;

        HTTPClient http;
        http.begin("http://" + endpoint);

        Serial.println("Attempting GET");

        // Send the original request's JSON to be deserialized on the remote controller
        int statusCode = http.GET();
        Serial.println("Status received: ");
        Serial.println(statusCode);
        
        // Free up resources
        http.end();
        
        request->send(statusCode);
    });

    _server->on("/api/get-controller-list", HTTP_GET, [](AsyncWebServerRequest *request) {
        ControllerManagement controllerManagement;
        StaticJsonDocument<512> list = controllerManagement.getControllerList();

        String json = "";
        serializeJson(list, json);
        
        request->send(200, "text/plain", json);
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
