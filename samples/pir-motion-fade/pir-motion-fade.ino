#include <WiFi.h>

#define fadeIncrement 2
#define durationOn 10000 // In Milliseconds

const int ledPin = 32;
const int pirPin = 27;

const char *ssid = "";
const char *password = "";

WiFiServer server(80);

//// Set your Static IP address
// IPAddress local_IP(192, 168, 1, 100);
//// Set your Gateway IP address
// IPAddress gateway(192, 168, 1, 1);
// IPAddress subnet(255, 255, 0, 0);

unsigned int pirState = LOW;
unsigned int pirReadVal = 0;
unsigned int brightness = 0;
unsigned long triggerTime = 0; // Time when the motion detection triggered

void initWiFi()
{
  // Configures static IP address
  //  if (!WiFi.config(local_IP, gateway, subnet)) {
  //    Serial.println("STA Failed to configure");
  //  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());

  server.begin();
}

void initWiFiClient()
{
  WiFiClient client = server.available(); // listen for incoming clients

  if (client)
  {                                // if you get a client,
    Serial.println("New Client."); // print a message out the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        if (c == '\n')
        { // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else
          { // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H"))
        {
          digitalWrite(5, HIGH); // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L"))
        {
          digitalWrite(5, LOW); // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void setup()
{
  Serial.begin(9600);

  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());

  initWiFiClient();

  pinMode(pirPin, INPUT);

  ledcAttachPin(ledPin, 0); // assign a led pins to a channel
  ledcSetup(0, 4000, 8);    // 12 kHz PWM, 8-bit resolution
}

void loop()
{
  pirReadVal = digitalRead(pirPin);

  if (pirReadVal == HIGH)
  {
    // Always update trigger time to track when we need to fade back out after no motion (not just after fade in)
    triggerTime = millis();

    if (pirState == LOW)
    {
      Serial.println("Motion detected!");
      fadeIn();
      pirState = HIGH;
    }
  }
  else
  {
    if (pirState == HIGH)
    {
      // Wait defined duration (from the time of trigger) then fade out and switch pirState to allow a next fade in
      if (millis() > triggerTime + durationOn)
      {
        Serial.println("No motion for duration, fading out");
        fadeOut();
        pirState = LOW;
      }
    }
  }
}

void fadeIn()
{
  do
  {
    brightness += fadeIncrement;
    ledcWrite(0, brightness);
    delay(30);
  } while (brightness <= 255);
}

void fadeOut()
{
  do
  {
    brightness -= fadeIncrement;
    ledcWrite(0, brightness);
    delay(30);
  } while (brightness > 0);
}
