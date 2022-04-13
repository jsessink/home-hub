#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <StreamUtils.h>

#include <WiFiConfig.h>
#include <AccessPointConfig.h>

#include "WebApp/src/WebApp.h"
#include "AccessPointAPI/src/AccessPointAPI.h"

WebApp webApp;
AccessPointAPI accessPointAPI;

// Set web server port number to 80
AsyncWebServer server(80);

// So, onWifiEvent has proven to be unreliable. Going with a ping every so often instead to see if IP Addresses are still active/online.
//  Since we don't know if they just go offline, or are no longer a part of the network, 
//  we'll have to match a hostname to an IP (and if there is a duplicate, but the dupe is offline, then remove that from the list).

// void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
// {
//   switch (event) {
//     case SYSTEM_EVENT_STA_CONNECTED:
//       Serial.println("ESP32 Connected to WiFi Network");
//       break;
//     case SYSTEM_EVENT_AP_STACONNECTED:
//       Serial.println("Station connected to ESP32 soft AP");
//       Serial.println("Station IP - ");

//       Serial.print(ip4_addr1(&(info.got_ip.ip_info.ip)));
//       Serial.print(".");
//       Serial.print(ip4_addr2(&(info.got_ip.ip_info.ip)));
//       Serial.print(".");
//       Serial.print(ip4_addr3(&(info.got_ip.ip_info.ip)));
//       Serial.print(".");
//       Serial.print(ip4_addr4(&(info.got_ip.ip_info.ip)));

//       Serial.println();
//       Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));

//       break;
//     case SYSTEM_EVENT_AP_STADISCONNECTED:
//       Serial.println("Station disconnected from ESP32 soft AP");
//       Serial.println("Station IP - ");

//       Serial.print(ip4_addr1(&(info.got_ip.ip_info.ip)));
//       Serial.print(".");
//       Serial.print(ip4_addr2(&(info.got_ip.ip_info.ip)));
//       Serial.print(".");
//       Serial.print(ip4_addr3(&(info.got_ip.ip_info.ip)));
//       Serial.print(".");
//       Serial.print(ip4_addr4(&(info.got_ip.ip_info.ip)));

//       Serial.println();
//       Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));

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
