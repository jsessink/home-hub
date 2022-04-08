#include "string"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <StreamUtils.h>

#include "AccessPointAPI.h"
#include "HTTP/src/Response.h"

void AccessPointAPI::handleAPServiceRouting(AsyncWebServer *server)
{
    _server = server;

    // Params accepted
    //  [0]controller-ip as IPAddress (as string)
    //  [1]controller-type as ControllerConfig.ControllerType
    //  
    _server->on("/connection/new-connection", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("New connection made");
        Response response;
        	
        // int paramsNr = request->params();
        // for(int i = 0; i < paramsNr; i++)
        // {
        //     AsyncWebParameter* p = request->getParam(i);
        
        //     Serial.print("Param name: ");
        //     Serial.println(p->name());
        
        //     Serial.print("Param value: ");
        //     Serial.println(p->value());
        
        //     Serial.println("------");
        // }

        String controllerIP = request->getParam(0)->value();
        String controllerType = request->getParam(1)->value();

        // Attempt to retrieve existing connections from list to see if it already is present, if not then add it
        // Get current JSON settings from EEPROM
        StaticJsonDocument<512> doc;

        // Check if any data exists first
        Serial.println("Checking for existing EEPROM JSON data.");
        EepromStream eepromStream(0, 512);

        Serial.println("EEPROM found data on address 0, attempting to deserialize.");
        DeserializationError error = deserializeJson(doc, eepromStream);
        
        if (error)
        {
            Serial.print(F("EEPROM deserializeJson() failed: "));
            Serial.println(error.f_str());
            
            Serial.println("Nothing present in EEPROM on address 0.  Adding a new file!");
            StaticJsonDocument<512> newDoc;

            Serial.println("");
            Serial.println("Creating an array with property of the IP - ");
            Serial.println(controllerIP);
            Serial.println("And type of - ");
            Serial.print(controllerType);
            Serial.println("");

            JsonArray nested = newDoc.createNestedArray(controllerIP);
            nested.add(controllerType);

            doc = newDoc;
        }
        else 
        {
            Serial.println("Found data, here is the existing controller list - ");

            for (JsonPair kv : doc.as<JsonObject>()) {
                String key = kv.key().c_str();
                Serial.println(kv.key().c_str());

                String val = doc[key];
                Serial.println(val);

                if (key == controllerIP)
                {
                    response.statusCode = 200;
                    response.message = "IP of this controller already exists, move along";
                    return response;
                }
            }

            JsonArray nested = doc.createNestedArray(controllerIP);
            nested.add(controllerType);
        }

        
        Serial.println("Updating EEPROM");
        // Store Hostname, IP, and useIPRequired on the host controller partition
        serializeJson(doc, eepromStream);
        eepromStream.flush(); // -> Calls EEPROM.commit() on esp32

        Serial.println("Rebooting...");
        ESP.restart(); 

        response.statusCode = 200;
        response.message = "Success!";

        Serial.println("Success sent");
        return response;
    });
}
