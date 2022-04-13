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
    _server->on("/api/proxy/get-current-rgb", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Received proxy request to get current color");

      String ip = String("http://" + request->getParam("ip")->value());

      // Ping remote controller to ensure it is ready to GET
      bool isReady = WebApp::pingRemote(ip);
      if (!isReady)
      {
        Response response;
        response.message = "Remote device not ready to receive input";
        response.statusCode = 503;
        request->send(response.statusCode, String(), response.message);
        return;
      }

      String endpoint = String("http://" + ip + "/api/get-current-rgb");

      HTTPClient http;
      http.begin(endpoint);

      Serial.println("Attempting GET");
      // Send the original request's JSON to be deserialized on the remote controller
      int statusCode = http.GET();
      Serial.println("Status received: ");
      Serial.println(statusCode);

      if (statusCode == 200)
      {
        request->send(200, "application/json", http.getString());
      } 
      else 
      {
        request->send(statusCode);
      }

      // Free up resources
      http.end();
    });
  
    _server->on("/api/proxy/rgb", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received proxy request to change color");

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
            response.statusCode = 503;
            request->send(response.statusCode, String(), response.message);
            return;
        }

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
}

void WebApp::handleClient(AsyncWebServer *server)
{
    _server = server;

    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(SPIFFS, "/index.html", String(), false); });

    handleUIRouting();
    handleServiceRouting();
}
