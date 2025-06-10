/*
   Written by Ahmad Hanis (http://github.com/ahmadhanis)
   Updated: 9 June 2025
   This sketch reads temperature and humidity from DHT11 sensor using ESP32
   and sends the data to the Favoriot IoT platform (API v2) via HTTP POST.

   You need:
   - Favoriot account (https://platform.favoriot.com)
   - Device registered and API Key generated
   - ESP32 + DHT11 connected to GPIO 4
*/

#include <WiFi.h>     // WiFi library for ESP32
#include <dht.h>      // DHT sensor library (legacy)

// === Pin Configuration ===
#define DHTPIN 4      // DHT11 data pin connected to GPIO4
#define LED 2         // Optional onboard LED for status indication

// === WiFi Credentials ===
const char* ssid = "myUUM_Guest";   // Replace with your WiFi SSID
const char* password = "";          // Replace with your WiFi password

// === Favoriot Configuration ===
const String deviceDeveloperId = "esp32_project_1_device@ahmadhanis"; // Device Developer ID from Favoriot
const String apiKey = ""; // API Key from account settings
const char* favoriotServer = "apiv2.favoriot.com";  // Favoriot server hostname
const int favoriotPort = 80;                        // HTTP port

// === Objects ===
dht DHT;               // DHT sensor object
WiFiClient client;     // WiFi client object for HTTP connection

void setup() {
  Serial.begin(115200);            // Start Serial Monitor
  pinMode(LED, OUTPUT);            // Set LED pin as output
  digitalWrite(LED, LOW);          // Turn LED off initially

  Serial.println("Connecting to WiFi...");
  if (connectToWiFi()) {
    Serial.println("✅ WiFi connected");
  } else {
    Serial.println("❌ Failed to connect to WiFi. Check credentials.");
  }
}

void loop() {
  // Only run if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    int chk = DHT.read11(DHTPIN);  // Read DHT11 sensor data
    float temp = DHT.temperature;
    float hum = DHT.humidity;
    // Validate sensor reading
    if (!isnan(temp) && !isnan(hum)) {
      Serial.printf("Temperature: %.2f °C\nHumidity: %.2f %%\n", temp, hum);

      // Create JSON payload to send to Favoriot
      String json = "{\"device_developer_id\":\"" + deviceDeveloperId +
                    "\",\"data\":{\"temperature\":" + String(temp, 2) +
                    ",\"humidity\":" + String(hum, 2) + "}}";

      Serial.println("Sending JSON to Favoriot:");
      Serial.println(json);

      // Send JSON payload to Favoriot
      sendToFavoriot(json);

    } else {
      Serial.println("❌ Failed to read from DHT11 sensor.");
    }

  } else {
    Serial.println("❌ WiFi not connected.");
  }

  delay(20000); // Delay 10 seconds before next reading
}

// === WiFi Connection Function ===
bool connectToWiFi() {
  WiFi.softAPdisconnect(true);     // Disconnect from any softAP
  WiFi.disconnect();               // Disconnect existing WiFi
  WiFi.mode(WIFI_STA);             // Set to station mode
  WiFi.begin(ssid, password);      // Begin WiFi connection

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    Serial.print(".");
    delay(500);
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  return false;
}

// === Send Data to Favoriot Function ===
void sendToFavoriot(const String& payload) {
  if (client.connect(favoriotServer, favoriotPort)) {
    Serial.println("📡 Connected to Favoriot");

    // Send HTTP POST headers
    client.println("POST /v2/streams HTTP/1.1");
    client.println("Host: apiv2.favoriot.com");
    client.println("Content-Type: application/json");
    client.println("cache-control: no-cache");
    client.print("apikey: ");
    client.println(apiKey);
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println("Connection: close");
    client.println();
    client.println(payload);  // Send JSON payload

    // Read and print response from Favoriot
    while (client.connected() && !client.available()) delay(10);
    while (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);

      // Basic response status check
      if (line.indexOf("201 Created") != -1) {
        Serial.println("✅ Data sent successfully!");
        digitalWrite(LED, HIGH); delay(100); digitalWrite(LED, LOW); // Blink LED
      } else if (line.indexOf("401 Unauthorized") != -1) {
        Serial.println("❌ Unauthorized: Check API Key or Device Developer ID.");
      } else if (line.indexOf("400") != -1) {
        Serial.println("⚠️ Bad request: Check your JSON format.");
      }
    }

    client.stop();  // Close the connection
  } else {
    Serial.println("❌ Failed to connect to Favoriot server.");
  }
}
