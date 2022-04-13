#include "string"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <StreamUtils.h>

#include "ControllerManagement.h"
#include "HTTP/src/Response.h"

StaticJsonDocument<512> ControllerManagement::getControllerList()
{
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
        return doc;
    }

    Serial.println("Success, returning results");
    return doc;
}
