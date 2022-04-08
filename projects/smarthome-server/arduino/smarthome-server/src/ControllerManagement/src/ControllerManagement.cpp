#include "string"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <StreamUtils.h>

#include "ControllerManagement.h"
#include "HTTP/src/Response.h"

String ControllerManagement::getControllerList()
{
    return "OK";
}

Response ControllerManagement::addController(AsyncWebServerRequest *request, StaticJsonDocument<200> doc, char* json)
{
    
    String newControllerHostname = doc["controllerHostname"];
    String newControllerIP = doc["controllerIP"];

    Serial.println("Pinging hostname - ");
    Serial.print(newControllerHostname);

    // Ping controller to see if it exists
    bool pingHostnameSuccess = Ping.ping(newControllerHostname.c_str(), 3);
    bool useIPRequired = false;

    Response response;
    
    if (!pingHostnameSuccess)
    {
        // Try the IP instead
        bool pingIPSuccess = Ping.ping(newControllerIP.c_str(), 3);

        Serial.println("Could not ping by Hostname, Pinging IP - ");
        Serial.print(newControllerIP);

        if (!pingIPSuccess) {
            response.statusCode = 400;
            response.message = "Hostname and IP both failed, please double check spelling and IP address fallback";
            return response;
        } else {
            useIPRequired = true;
        }
    }

    // Send default data to remote controller to store on its own flash memory to offload that data so it can be used "offline" if necessary
    WiFiClient client;
    HTTPClient http;

    String endpoint = useIPRequired ? 
        "http://" + newControllerIP + "/api/update-settings" :
        "http://" + newControllerHostname + "/api/update-settings";

    http.begin(client, endpoint);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Send the original request's JSON to be deserialized on the remote controller
    int httpResponseCode = http.POST(json);


    // Get current JSON settings from EEPROM
    StaticJsonDocument<200> updatedDoc;

    // Check if any data exists first
    StaticJsonDocument<200> existingDoc;
    EEPROM.get(0, existingDoc);

    if (!existingDoc.isNull())
    {
        EepromStream eepromStream(0, EEPROM.length());
        DeserializationError error = deserializeJson(existingDoc, eepromStream);
        
        if (error)
        {
            Serial.print(F("EEPROM deserializeJson() failed: "));
            Serial.println(error.f_str());
            response.statusCode = 500;
            response.message = "Unable to obtain existing EEPROM JSON data.  Please try again.";
            return response;
        }

        JsonArray controllers = existingDoc["controllers"];
        Serial.println(controllers);
        Serial.println(controllers.size());

        updatedDoc = doc;
    } 
    else 
    {
        updatedDoc = doc;
    }


    // Store Hostname, IP, and useIPRequired on the host controller partition
    EepromStream eepromStream(0, EEPROM.length());
    serializeJson(updatedDoc, eepromStream);
    EEPROM.commit();

    eepromStream.flush();
    Serial.println("Rebooting...");
    ESP.restart();

    response.statusCode = 200;
    response.message = "Great! host and remote controllers updated successfully!";

    return response;

    // controllerHostname: string;
    // controllerIP: string;
    // controllerType: string;
    // defaultColor: string;
    // alwaysOn: boolean;
    // accidentalTripDelay: number;
    // durationOn: number;
    // fadeInSpeed: number;
    // fadeOutSpeed: number;
}