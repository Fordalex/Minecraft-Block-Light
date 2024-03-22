#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// For a custom domain name.
#include <ESP8266mDNS.h>
#include "credentials.h"

int currentR = 0; // Store current Red value
int currentG = 0; // Store current Green value
int currentB = 0; // Store current Blue value

const char* part1 = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Minecraft Block Light</title>
    <link rel="icon" type="image/png" href="https://cdn2.steamgriddb.com/icon/bf6423635e56a99e9df17852c6bfadca/32/512x512.png">
    <style>
            .container {
                display: flex;
                justify-content: center;
                align-items: center;
                width: 100vw;
                margin-bottom: 500px;
                min-height: 500px;
            }
            .wrapper {
                display: flex;
                justify-content: center;
                flex-direction: column;
                width: 390px;
                position: relative;
            }
            .inventory-img {
                position: absolute;
                z-index: -1;
            }
            .white-top {
                background-color: white;
                width: 100%;
                height: 200px;
            }
            .border {
                border: solid black 4px;
                height: 155px;
            }
            .white-bottom {
                background-color: white;
                width: 100%;
                height: 100px;
            }

            .inventory-wrapper {
                display: flex;
                justify-content: flex-start;
                position: absolute;
                height: 40px;
                width: 230px;
                top: 231px;
                left: 15px;
            }
            .item {
                width: 36px;
                height: 36px;
                margin: -1px;
                display: block;
                margin-bottom: 12px;
                border: solid transparent 3px;
            }
            .item:hover {
                cursor: pointer;
                background-color: rgba(255, 255, 255, 0.3);
            }
        </style>
</head>
<body>
    <div class="container">
            <div class="wrapper"> 
                <div class="white-top"></div>
                <div class="border"></div>
                <div class="white-bottom"></div>
                <img src="https://static.planetminecraft.com/files/resource_media/screenshot/1247/Emerald-Ore_4183506.jpg" width="100%" class="inventory-img">
                <div class="inventory-wrapper">
                    <div class="item" onclick="setColourFromPresets(0, 0, 0)"></div>
                    <div class="item" onclick="setColourFromPresets(255, 0, 0)"></div>
                    <div class="item" onclick="setColourFromPresets(255, 255, 0)"></div>
                    <div class="item" onclick="setColourFromPresets(155, 155, 0)"></div>
                    <div class="item" onclick="setColourFromPresets(0, 15, 255)"></div>
                </div>
            </div>
        </div>
    <input type="color" id="colorPicker" value="#ff0000">
    <button onclick="setColour()">Set Color</button>
    <p>Current color: <span id="currentColor">)rawliteral";

const char* part2 = R"rawliteral(</span></p>
    <script>
    function setColour() {
      var color = document.getElementById('colorPicker').value;
      document.getElementById('currentColor').innerText = 'RGB(' + parseInt(color.substr(1,2), 16) + ', ' + parseInt(color.substr(3,2), 16) + ', ' + parseInt(color.substr(5,2), 16) + ')';
      var xhr = new XMLHttpRequest();
      // Convert hex color to RGB
      var r = parseInt(color.substr(1,2), 16);
      var g = parseInt(color.substr(3,2), 16);
      var b = parseInt(color.substr(5,2), 16);

      // Construct URL with RGB parameters
      xhr.open("GET", "/set-colour?r=" + r + "&g=" + g + "&b=" + b, true);
      xhr.send();
    }

    function setColourFromPresets(r, g, b) {
      document.getElementById('colorPicker').value = '#' + r.toString(16).padStart(2, '0') + g.toString(16).padStart(2, '0') + b.toString(16).padStart(2, '0');
      setColour();
    }
    </script>
</body>
</html>
)rawliteral";


// Initialize web server on port 80
ESP8266WebServer server(80);

// GPIO pins for the RGB LED
const int pinR = 3;
const int pinG = 2;
const int pinB = 5;

void setup() {
  Serial.begin(115200);
  
  // Initialize GPIO pins as outputs
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(2, OUTPUT); // Set GPIO 2 as an output
  digitalWrite(2, HIGH); // Set GPIO 2 high to turn off the LED

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

   // Start the mDNS responder
  if (!MDNS.begin("minecraft")) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    String currentColorValues = "RGB(" + String(currentR) + ", " + String(currentG) + ", " + String(currentB) + ")";
    String fullPage = String(part1) + currentColorValues + String(part2);
    server.send(200, "text/html", fullPage.c_str());
  });
  
  // Route for setting RGB color
  server.on("/set-colour", HTTP_GET, []() {
    String r = server.arg("r");
    String g = server.arg("g");
    String b = server.arg("b");
    
    // Convert arguments to integer and write to LEDs
    analogWrite(pinR, r.toInt());
    analogWrite(pinG, g.toInt());
    analogWrite(pinB, b.toInt());
    
    // Update current RGB values
    currentR = r.toInt();
    currentG = g.toInt();
    currentB = b.toInt();
    
    server.send(200, "text/plain", "Color set");
  });

  // Start server
  server.begin();
}

void loop() {
  server.handleClient();
  MDNS.update();  // Not strictly necessary for basic MDNS functionality
}
