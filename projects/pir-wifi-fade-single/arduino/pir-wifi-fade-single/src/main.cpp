#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <HTTPClient.h>

#include "C:/projects/_personal/arduino/projects/_shared/AccessPointConfig.h"
#include "C:/projects/_personal/arduino/projects/_shared/ControllerConfig.h"

#include "WebApp/src/WebApp.h"
#include "LEDControl/src/LEDControl.h"

LEDControl ledControl;
WebApp webApp;

AsyncWebServer server(80);

void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting Host Access Point");

  // Connect to Home WiFi network with SSID and password
  WiFi.mode(WIFI_STA);
  WiFi.begin(AccessPointSSID, AccessPointPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to Access Point.");
  Serial.println("Assigned IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  Serial.println("Host / HUB IP: ");
  Serial.println(WiFi.dnsIP());
  Serial.println("");


  // Let host know we exist by doing a GET request to a specific endpoint
  // Send default data to remote controller to store on its own flash memory to offload that data so it can be used "offline" if necessary
  HTTPClient http;

  String endpoint = "http://" + WiFi.dnsIP().toString() + "/connection/new-connection?controller-ip=" + WiFi.localIP().toString() + "&controller-type=" + ControllerConfig::PIRRGB;

  Serial.println("Making GET request to - ");
  Serial.print(endpoint);

  http.begin(endpoint);
  int httpResponseCode = http.GET();

  Serial.println(httpResponseCode);

  // Write initial LED settings to ESP
  ledControl.ledSetup();

  // Setup for SPIFFS for accessing data storage on the ESP32 flash memory
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Async - Does not need to be in loop
  webApp.handleClient(&server);

  // Initialize OTA updater (needs to be before server boots)
  //  -- Visit http://[ip]:[port]/update to update the firmware OTA
  //  --- Upload the firmware as .bin by using PlatformIO output found in /.pio/build/nodemcu-32s/firmware.bin
  AsyncElegantOTA.begin(&server);

  // server.begin in WebApp
  webApp.initServer();
}

void loop() {}