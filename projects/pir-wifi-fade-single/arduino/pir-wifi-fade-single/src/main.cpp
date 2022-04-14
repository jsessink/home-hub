#include <WiFi.h>
#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <HTTPClient.h>

#include <ControllerConfig.h>
#include <AccessPointConfig.h>
#include <DeviceList.h>

#include "API/src/API.h"
#include "PIRControl/src/PIRControl.h"
#include "LEDControl/src/LEDControl.h"
#include "Settings/src/Settings.h"

using namespace DeviceList;

PIRControl pirControl;
LEDControl ledControl;
API api;

AsyncWebServer server(80);

bool attemptingWiFiConnection;
void checkWiFiStatusAndConnect()
{
  if(WiFi.status() != WL_CONNECTED)
  {
    if (!attemptingWiFiConnection)
    {
      Serial.println("WiFi was disconnected, attempting reconnect.");
      WiFi.reconnect();

      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }

      Serial.println("Reconnected!");

      // Let host know we exist again by doing a GET request to a specific endpoint
      // Send default data to remote controller to store on its own flash memory to offload that data so it can be used "offline" if necessary
      HTTPClient http;
      String endpoint = String("http://" + WiFi.dnsIP().toString() + "/connection/new-connection?controller-ip=" + WiFi.localIP().toString() + "&controller-device-name=" + activeDevice + "&controller-type=" + ControllerConfig::PIR_RGB);

      Serial.println("Making GET request to - ");
      Serial.println(endpoint);

      http.begin(endpoint);
      http.GET();
    }
  }
}


void setup()
{
  Serial.begin(115200);

  Serial.println("Setting previously ON settings to the light strip, in case it was turned off by a switch or accident.");
  
  // LED settings to ESP configuration
  ledControl.ledSetup();

  // Init the settings in case defaults need to be set
  Settings bootedSettings = api.initGetSettings();
  ledControl.initBootedSettings(bootedSettings);

  // Init PIR setup and listener on Core 0 (everything else defaults to 1);
  pirControl.pirSetup();

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting Host Access Point");

  // Connect to Home WiFi network with SSID and password
  WiFi.mode(WIFI_STA);

  attemptingWiFiConnection = true;
  WiFi.begin(AccessPointSSID, AccessPointPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  attemptingWiFiConnection = false;

  Serial.println("");
  Serial.println("Connected to Access Point.");
  Serial.println("Assigned IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  Serial.println("Host / HUB IP: ");
  Serial.println(WiFi.dnsIP());
  Serial.println("");

  // Setup for SPIFFS for accessing data storage on the ESP32 flash memory
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Let host know we exist by doing a GET request to a specific endpoint
  // Send default data to remote controller to store on its own flash memory to offload that data so it can be used "offline" if necessary
  HTTPClient http;
  String endpoint = String("http://" + WiFi.dnsIP().toString() + "/connection/new-connection?controller-ip=" + WiFi.localIP().toString() + "&controller-device-name=" + activeDevice + "&controller-type=" + ControllerConfig::PIR_RGB);

  Serial.println("Making GET request to - ");
  Serial.println(endpoint);

  http.begin(endpoint);
  http.GET();

  // Async - Does not need to be in loop
  api.handleServiceRouting(&server);

  // Initialize OTA updater (needs to be before server boots)
  //  -- Visit http://[ip]:[port]/update to update the firmware OTA
  //  --- Upload the firmware as .bin by using PlatformIO output found in /.pio/build/nodemcu-32s/firmware.bin
  AsyncElegantOTA.begin(&server);

  server.begin();

  // Check connection constantly in case the hub goes down and needs to reconnect
  for(;;)
  {
    delay(1000);
    checkWiFiStatusAndConnect();
  }
}

void loop() {}
