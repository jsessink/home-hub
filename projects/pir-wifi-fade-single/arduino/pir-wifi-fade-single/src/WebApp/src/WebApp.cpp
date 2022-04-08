#include <WiFi.h>

#include "SPIFFS.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "LEDControl/src/LEDControl.h"
#include "WebApp.h"

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

void WebApp::onRGBAChange(){
    Serial.println("ok then");
}

void WebApp::handleServiceRouting()
{
    _server->on("/api/rgba-change", HTTP_POST,
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

            int r = doc["r"];
            int g = doc["g"];
            int b = doc["b"];
            
            LEDControl ledControl;
            ledControl.handleLEDRequest(r, g, b);

            request->send(200);
        }
    );

    _server->on("/api/update-settings", HTTP_POST,
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
            Serial.println(String("data=") + (char*)data);
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

            
            Serial.println("Received JSON!");

            request->send(200);
        }
    );
}

void WebApp::initServer()
{
    _server->begin();
}

void WebApp::handleClient(AsyncWebServer *server)
{
    _server = server;

    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(SPIFFS, "/index.html", String(), false); });

    handleUIRouting();
    handleServiceRouting();

    // client = server.available(); // Listen for incoming clients

    // if (client)
    // { // If a new client connects,
    //     currentTime = millis();
    //     previousTime = currentTime;
    //     Serial.println("New Client."); // print a message out in the serial port
    //     String currentLine = "";       // make a String to hold incoming data from the client
    //     while (client.connected() && currentTime - previousTime <= timeoutTime)
    //     { // loop while the client's connected
    //         currentTime = millis();
    //         if (client.available())
    //         {                           // if there's bytes to read from the client,
    //             char c = client.read(); // read a byte, then
    //             Serial.write(c);        // print it out the serial monitor
    //             header += c;
    //             if (c == '\n')
    //             { // if the byte is a newline character
    //                 // if the current line is blank, you got two newline characters in a row.
    //                 // that's the end of the client HTTP request, so send a response:
    //                 if (currentLine.length() == 0)
    //                 {
    //                     // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    //                     // and a content-type so the client knows what's coming, then a blank line:
    //                     client.println("HTTP/1.1 200 OK");
    //                     client.println("Content-type:text/html");
    //                     client.println("Connection: close");
    //                     client.println();

    //                     // Display the HTML web page
    //                     client.println("<!DOCTYPE html><html>");
    //                     client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    //                     client.println("<link rel=\"icon\" href=\"data:,\">");
    //                     client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
    //                     client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>");
    //                     client.println("</head><body><div class=\"container\"><div class=\"row\"><h1>ESP Color Picker</h1></div>");
    //                     client.println("<a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a> ");
    //                     client.println("<input class=\"jscolor {onFineChange:'update(this)'}\" id=\"rgb\"></div>");
    //                     client.println("<script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);");
    //                     client.println("document.getElementById(\"change_color\").href=\"?r\" + Math.round(picker.rgb[0]) + \"g\" +  Math.round(picker.rgb[1]) + \"b\" + Math.round(picker.rgb[2]) + \"&\";}</script></body></html>");
    //                     // The HTTP response ends with another blank line
    //                     client.println();

    //                     // Request sample: /?r201g32b255&
    //                     // Red = 201 | Green = 32 | Blue = 255
    //                     if (header.indexOf("GET /?r") >= 0)
    //                     {
    //                         int _pos1 = header.indexOf('r');
    //                         int _pos2 = header.indexOf('g');
    //                         int _pos3 = header.indexOf('b');
    //                         int _pos4 = header.indexOf('&');


    //                         LEDControl ledControl;
    //                         ledControl.handleLEDRequest(
    //                             _pos1,
    //                             _pos2,
    //                             _pos3,
    //                             _pos4,
    //                             header.substring(_pos1 + 1, _pos2),
    //                             header.substring(_pos2 + 1, _pos3),
    //                             header.substring(_pos3 + 1, _pos4)
    //                         );
    //                     }
    //                     // Break out of the while loop
    //                     break;
    //                 }
    //                 else
    //                 { // if you got a newline, then clear currentLine
    //                     currentLine = "";
    //                 }
    //             }
    //             else if (c != '\r')
    //             {                     // if you got anything else but a carriage return character,
    //                 currentLine += c; // add it to the end of the currentLine
    //             }
    //         }
    //     }
    //     // Clear the header variable
    //     header = "";
    //     // Close the connection
    //     client.stop();
    //     Serial.println("Client disconnected.");
    //     Serial.println("");
    // }
}
