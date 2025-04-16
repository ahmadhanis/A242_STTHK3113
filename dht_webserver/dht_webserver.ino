#include "DHT.h"              // Library for DHT sensor
#include <WebServer.h>        // Library for creating a web server
#include <WiFi.h>             // Library for WiFi functionality

// Define the pin and type of DHT sensor
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials (Access Point mode)
const char* ssid = "DHTServer";
const char* password = "";  // Leave blank for open network

// Create a WebServer object on port 80
WebServer server(80);

// Sensor readings
float h = 0.0;
float t = 0.0;

void setup() {
  Serial.begin(115200);          // Start serial communication
  dht.begin();                   // Initialize DHT sensor

  // Set WiFi to Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);   // Create AP with given SSID and password

  // Define the server route
  server.on("/", handle_root);   // When accessing root URL, call handle_root()

  server.begin();                // Start the web server
  delay(500);                    // Small delay for stability
}

// Function to handle the root path "/"
void handle_root() {
  String content = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  content += "<title>DHT11 Readings</title><style>body{text-align:center;font-family:Arial;}h3{color:#333;}</style></head><body>";
  content += "<h3>DHT11 Sensor Readings</h3>";
  content += "<p>Temperature: " + String(t) + " &deg;C</p>";
  content += "<p>Humidity: " + String(h) + " %</p>";
  content += "</body></html>";
  
  server.send(200, "text/html", content);  // Send HTML page with data
}

void loop() {
  server.handleClient();  // Handle incoming client requests

  // Read temperature and humidity every 2 seconds
  delay(2000);
  float newHumidity = dht.readHumidity();
  float newTemp = dht.readTemperature();  // Read temperature in Celsius

  // Check if readings are valid
  if (!isnan(newHumidity) && !isnan(newTemp)) {
    h = newHumidity;
    t = newTemp;

    // Print readings to Serial Monitor
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F(" %  Temperature: "));
    Serial.print(t);
    Serial.println(F(" °C"));
  } else {
    Serial.println(F("Failed to read from DHT sensor!"));
  }
}
