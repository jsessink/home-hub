#include "string"
#include <Arduino.h>
#include <ArduinoJson.h>
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
    //  [1]controller-device-name as DeviceSettings.Device
    //  [2]controller-type as ControllerConfig.ControllerType
    //  
    _server->on("/connection/new-connection", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("New connection made");
        Response response;

        String controllerIP = request->getParam(0)->value();
        String controllerDeviceName = request->getParam(1)->value();
        String controllerType = request->getParam(2)->value();

        if (!controllerIP || !controllerDeviceName || !controllerType)
        {
          response.statusCode = 400;
          response.message = "Invalid parameters! Please include a controller-ip, controller-device-name, and controller-type respectively in the arguments list.";
          request->send(response.statusCode, String(), response.message);
          return;
        }

        // Attempt to retrieve existing connections from list to see if it already is present, if not then add it
        // Get current JSON settings from EEPROM
        StaticJsonDocument<1024> doc;

        // Check if any data exists first
        Serial.println("Checking for existing EEPROM JSON data.");
        EepromStream eepromStream(0, 1024);
        DeserializationError error = deserializeJson(doc, eepromStream);
        
        if (error)
        {
            Serial.print(F("EEPROM deserializeJson() failed: "));
            Serial.println(error.f_str());
            
            Serial.println("Nothing present in EEPROM on address 0. Adding a new file!");
            Serial.println("");
            Serial.println("Creating new object with controllerIP: ");
            Serial.println(controllerIP);
            Serial.println("With a controllerDeviceName of: ");
            Serial.println(controllerDeviceName);
            Serial.println("And controllerType of: ");
            Serial.print(controllerType);
            Serial.println("");

            JsonObject ipObject = doc.createNestedObject();
              ipObject["controllerIP"] = controllerIP;
              ipObject["controllerDeviceName"] = controllerDeviceName;
              ipObject["controllerType"] = controllerType;
              ipObject["controllerOnline"] = true;

            Serial.println("Adding new list to EEPROM");
            // Store Hostname, IP, and useIPRequired on the host controller partition
            serializeJson(doc, eepromStream);
            eepromStream.flush(); // -> Calls EEPROM.commit() on esp32

            response.statusCode = 200;
            response.message = "Successfully added new list to flash memory.";
            request->send(response.statusCode, String(), response.message);

            Serial.println("Rebooting...");
            ESP.restart();
            return;
        }
        else 
        {
          Serial.println("Deserialization successful, iterating to find match against incoming IP and device name, if there is a length on the flash data.");
          JsonArray asRootArr = doc.as<JsonArray>();

          int index = -1;

          // Will skip this loop if asRootArr is empty, so no need to check
          for (JsonArray::iterator it = asRootArr.begin(); it != asRootArr.end(); ++it)
          {
            index += 1;
            String currControllerIP = (*it)["controllerIP"];
            String currControllerDeviceName = (*it)["controllerDeviceName"];

            Serial.println("Device IP: ");
            Serial.println(currControllerIP);
            Serial.println("Device name: ");
            Serial.println(currControllerDeviceName);

            if (currControllerDeviceName == controllerDeviceName)
            {
              // If the IP matches and the deviceName matches from above, then we're all set.
              if (currControllerIP == controllerIP)
              {
                Serial.println("Everything matches, returning success.");
                response.statusCode = 200;
                response.message = "IP of this controller already exists, move along";
                request->send(response.statusCode, String(), response.message);
                return;
              }
              // If deviceName matches, but IP doesn't, the device has changed IP.
              //  Remove old IP object and allow the rest of the condition to complete (including the loop, so don't break)
              else
              {
                Serial.println("Device name matches, but IP doesn't. Remove the object");
                JsonArray rootArr = doc.to<JsonArray>();
                  rootArr.remove(index);
              }
            }
            // If deviceName doesn't exist, then we need to just remove it (it's probably a fluke/old/host controller).
            // If deviceName just doesn't match, but the IP does, then we have a new controller on the old IP, so just change names
            else
            {
              if ((!currControllerDeviceName || !currControllerDeviceName.length() || currControllerDeviceName == "null") && currControllerIP == controllerIP)
              {
                Serial.println("No device name, but the IP matches removing this object.");
                JsonArray rootArr = doc.to<JsonArray>();
                  rootArr.remove(index);
              }
              else if (
                currControllerDeviceName != controllerDeviceName &&
                currControllerIP == controllerIP
              )
              {
                Serial.println("Device name does not match controllerDeviceName supplied, but IP matches.");
                Serial.println("Updating IP object's deviceName with given name.");
                Serial.println("Old: ");
                Serial.println(currControllerDeviceName);
                Serial.println("New: ");
                Serial.println(controllerDeviceName);

                // Make it updateable to then write to it
                JsonArray rootArr = doc.to<JsonArray>();
                JsonObject obj = rootArr[index];
                obj[controllerDeviceName] = controllerDeviceName;

                Serial.println("Updating EEPROM");
                // Store Hostname, IP, and useIPRequired on the host controller partition
                serializeJson(doc, eepromStream);
                eepromStream.flush(); // -> Calls EEPROM.commit() on esp32

                response.statusCode = 204;
                response.message = "Existing IP found, but mismatch or no device name prior. Updated with new device name.";
                request->send(response.statusCode, String(), response.message);

                Serial.println("Rebooting...");
                ESP.restart();
                return;
              }
            }
          }

          // If we have gotten here, then either nothing matches the existing data, or there is no existing data to iterate over.
          Serial.println("Nothing found in existing list that matches.");
          Serial.println("Creating new object with controllerIP: ");
          Serial.println(controllerIP);
          Serial.println("With a controllerDeviceName of: ");
          Serial.println(controllerDeviceName);
          Serial.println("And controllerType of: ");
          Serial.print(controllerType);
          Serial.println("");

          JsonObject ipObject = doc.createNestedObject();
            ipObject["controllerIP"] = controllerIP;
            ipObject["controllerDeviceName"] = controllerDeviceName;
            ipObject["controllerType"] = controllerType;
            ipObject["controllerOnline"] = true;

          Serial.println("Updating EEPROM");
          // Store Hostname, IP, and useIPRequired on the host controller partition
          serializeJson(doc, eepromStream);
          eepromStream.flush(); // -> Calls EEPROM.commit() on esp32

          response.statusCode = 200;
          response.message = "Successfully added device.";
          request->send(response.statusCode, String(), response.message);

          Serial.println("Rebooting...");
          ESP.restart();
          return;
        }
    });
}
