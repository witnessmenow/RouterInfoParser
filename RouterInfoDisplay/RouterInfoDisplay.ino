/*******************************************************************
    Getting connection details from my Huawei router
    and displaying it on an OLED screen
 *                                                                 *
    Written by Brian Lough
    https://www.youtube.com/channel/UCezJOfu7OtqGzd5xrP3q6WA
 *******************************************************************/

// ----------------------------
// Standard Libraries - Already Installed if you have ESP8266 set up
// ----------------------------

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include "SH1106.h"
// The driver for the OLED display
// Available on the library manager (Search for "oled ssd1306" by Daniel Eichorn)
// https://github.com/squix78/esp8266-oled-ssd1306

#define HOST "192.168.1.1"

// Wiring details
// D3 -> SDA
SH1106 display(0x3c, D3, D5);

WiFiClient client;

//------- Replace the following! ------
char ssid[] = "SSID";       // your network SSID (name)
char password[] = "PASSWORD";  // your network key

String connectionModeReading = "";
String signalReading = "";

void setup() {
  Serial.begin(115200);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
}

void displayInfo(String connectionMode, String signalValue) {
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String connectionMessage = "Connection Type: " + connectionMode;
  display.drawString(0, 0, connectionMessage);

  String signalMessage = "Signal Strength: " + signalValue;
  display.drawString(0, 12, signalMessage);

  display.display();
}

String getRouterConnectionData()
{
  String body = "";
  unsigned long now;
  bool responseReceived;
  if (client.connect(HOST, 80)) {
    Serial.println("connected to host");

    client.print("POST /index/getStatusByAjax.cgi?rid=911"); client.println(" HTTP/1.1");
    client.print("Host:"); client.println(HOST);
    client.println("Content-Length:0");
    client.println();
    delay(10);
    client.println();

    char c;
    now = millis();
    responseReceived = false;
    bool finishedHeaders = false;
    bool currentLineIsBlank = true;
    while (millis() - now < 1500) {
      while (client.available()) {
        char c = client.read();
        Serial.print(c);
        responseReceived = true;


        if (!finishedHeaders) {
          if (currentLineIsBlank && c == '\n') {
            finishedHeaders = true;
          }
        } else {
          body = body + c;
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }

      }
      if (responseReceived) {
      }
    }

    client.stop();
  }
  return body;
}

void parseRouterConnectionData() {
  String xmlString = getRouterConnectionData();
  if (xmlString.length() > 0) {
    signalReading = extractValueFromXml(xmlString, "SIG");
    Serial.println(signalReading);
    connectionModeReading = extractValueFromXml(xmlString, "Modename");
    Serial.println(connectionModeReading);
  } else {
    signalReading = "";
    connectionModeReading = "";
  }
}

String extractValueFromXml(String xmlString, String nodeName) {

  String startNode = "<" + nodeName + ">";
  int startNodeIndex = xmlString.indexOf(startNode);
  Serial.println(startNodeIndex);
  Serial.println(startNodeIndex + startNode.length());

  String endNode = "</" + nodeName + ">";
  int endNodeIndex = xmlString.indexOf(endNode);
  Serial.println(endNodeIndex);

  if (startNodeIndex == -1 || endNodeIndex == -1) {
    return "";
  }
  return xmlString.substring(startNodeIndex + startNode.length(), endNodeIndex);
}

void loop() {
  parseRouterConnectionData();
  displayInfo(connectionModeReading, signalReading);
  delay(10000);
}
