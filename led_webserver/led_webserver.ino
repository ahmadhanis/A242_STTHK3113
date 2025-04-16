#include <WiFi.h>          // For WiFi Access Point mode
#include <WebServer.h>     // For handling HTTP requests

// Access Point credentials
const char* ssid = "WiFiLED";
const char* password = ""; // Leave empty for open network

// Define the GPIO pin connected to the relay/LED
#define RELAY_PIN 2

// Create web server on port 80
WebServer server(80);

void setup() {
  Serial.begin(115200);            // Start Serial Monitor
  pinMode(RELAY_PIN, OUTPUT);      // Set relay pin as output
  digitalWrite(RELAY_PIN, LOW);    // Start with relay OFF

  // Set up WiFi as Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);     // Create AP with SSID and no password

  // Define routes
  server.on("/", handle_root);     // Main page
  server.on("/on", []() {          // Turn relay ON
    digitalWrite(RELAY_PIN, LOW);  // LOW to turn ON relay (depends on your module)
    delay(100);
    handle_root();
  });
  server.on("/off", []() {         // Turn relay OFF
    digitalWrite(RELAY_PIN, HIGH); // HIGH to turn OFF relay
    delay(100);
    handle_root();
  });

  server.begin();                  // Start web server
  delay(500);                      // Small delay for stability
}

// Function to serve the main HTML page
void handle_root() {
  String content = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  content += "<title>Relay Controller</title><style>body{text-align:center;font-family:sans-serif;}button{padding:15px 30px;margin:10px;font-size:16px;}</style></head><body>";
  content += "<h3>Fan Control System</h3>";
  content += "<a href='/on'><button>Turn ON</button></a>";
  content += "<a href='/off'><button>Turn OFF</button></a>";
  content += "</body></html>";

  server.send(200, "text/html", content);
}

void loop() {
  server.handleClient();  // Handle incoming HTTP requests
}
