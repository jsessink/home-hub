#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <StreamUtils.h>

#include "C:/projects/_personal/arduino/projects/_shared/WiFiConfig.h"
#include "C:/projects/_personal/arduino/projects/_shared/AccessPointConfig.h"

#include "WebApp/src/WebApp.h"
#include "AccessPointAPI/src/AccessPointAPI.h"

WebApp webApp;
AccessPointAPI accessPointAPI;

// Set web server port number to 80
AsyncWebServer server(80);

// void onWifiEvent(WiFiEvent_t event)
// {
//   switch (event) {
//     case SYSTEM_EVENT_STA_CONNECTED:
//       Serial.println("ESP32 Connected to WiFi Network");
//       break;
//     case SYSTEM_EVENT_AP_START:
//       Serial.println("ESP32 soft AP started");
//       break;
//     case SYSTEM_EVENT_AP_STACONNECTED:
//       Serial.println("Station connected to ESP32 soft AP");
//       Serial.println("Station IP - ");
//       Serial.println(event);
//       break;
//     case SYSTEM_EVENT_AP_STADISCONNECTED:
//       Serial.println("Station disconnected from ESP32 soft AP");
//       break;
//     default: break;
//   }
// }

void setup()
{
  // Debugger
  Serial.begin(115200);

  // Setup for SPIFFS for accessing data storage on the ESP32 flash memory for the Web assets
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Setup for EEPROM for data storage on Controller settings with 512 bytes allocated
  if (!EEPROM.begin(512))
  {
    Serial.println("An Error has occurred while mounting EEPROM");
    return;
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.println("Connecting to WiFI as hostname - ");
  Serial.print(HUBHostName);
  Serial.println("");
  WiFi.setHostname(HUBHostName);
  // WiFi.onEvent(onWifiEvent);
  WiFi.mode(WIFI_MODE_APSTA);

  // Connect to Home WiFi network with SSID and password
  WiFi.begin(WiFiSSID, WiFiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");


  // Setup Access Point on this host device so other controllers can connect
  Serial.println("WiFi Access Point SSID is - ");
  Serial.print(AccessPointSSID);
  Serial.println("");
  // SoftAPHostname is not what you think it is... just find the IPs on the remotes for connectivity
  // Serial.println("SoftAP Hostname is - ");
  // Serial.print(AccessPointHostname);
  // WiFi.softAPsetHostname(AccessPointHostname);
  WiFi.softAP(AccessPointSSID, AccessPointPassword);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP address is - ");
  Serial.print(IP);
  Serial.println("");


  // Setup for SPIFFS for accessing data storage on the ESP32 flash memory
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Handle requests from the clientside webapp
  webApp.handleClient(&server);

  // Handle requests from the local access point
  accessPointAPI.handleAPServiceRouting(&server);

  // Initialize OTA updater (needs to be before server boots)
  //  -- Visit http://[ip]:[port]/update to update the firmware OTA
  //  --- Upload the firmware as .bin by using PlatformIO output found in /.pio/build/nodemcu-32s/firmware.bin
  AsyncElegantOTA.begin(&server);

  server.begin();
}

void loop()
{}
