// ESP32 WiFi Manager with EEPROM Configuration Storage
// Last Updated: April 23, 2025
// Program Flow Overview:
// 1. On startup, read WiFi credentials from EEPROM.
// 2. If the SSID is missing, or button on GPIO 0 is pressed, enter AP (Access Point) mode.
// 3. In AP mode, ESP32 hosts a configuration webpage at 192.168.4.1:
//    - Shows stored credentials
//    - Scans for and lists available WiFi networks in a table
//    - Allows users to submit new credentials
// 4. Submitted credentials are stored back to EEPROM and the device restarts.
// 5. If credentials are valid, ESP32 attempts to connect to WiFi in STA (Station) mode.
// 6. If connection fails, fallback to AP mode for configuration.
// This sketch allows ESP32 to boot into STA mode with stored credentials or fallback to AP mode
// where users can enter new credentials through a simple web UI. Data is stored using EEPROM.
// Additional Features:
// - Scans for nearby WiFi networks and displays them in a responsive table.
// - Displays current stored credentials.
// - Allows updating WiFi credentials and device ID.
// - Non-blocking WiFi scan with live table update.
// - LED indicator on GPIO 2 lights up when in AP mode.
// - Improved user feedback messages on configuration save and data clear.
// Author: Ahmad Hanis | Date: April 23, 2025
// ------------------------------------------------------

#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

WebServer server(80);  // Create a web server object on port 80

// Global variables for storing WiFi credentials and device ID
String ssid, pass, devid, content;
bool apmode = false;  // Flag to indicate if device is in Access Point mode
bool scanComplete = false;  // Flag to prevent repeat scans
String scannedNetworks = "<p>Scanning networks...</p>";  // HTML content for scanned WiFi list

// Setup function runs once when the device starts
void setup() {
  pinMode(2, OUTPUT); // LED indicator on GPIO 2 for AP mode
  digitalWrite(2, LOW); // Turn off LED initially
  Serial.begin(115200);  // Start serial monitor for debugging
  Serial.println("Waiting for 5 seconds!");
  readData();  // Read stored credentials from EEPROM

  // If no SSID is found, start in Access Point mode
  if (ssid.length() == 0) {
    Serial.println("No SSID found in EEPROM, forcing AP mode.");
    ap_mode();
    return;
  }

  delay(5000);  // Give user time to press button if needed
  pinMode(0, INPUT_PULLUP);  // Set pin 0 as input with pull-up (for button input)

  // If button is pressed during boot, force AP mode
  if (digitalRead(0) == 0) {
    apmode = true;
    ap_mode();
  } else {
    // Try connecting to WiFi using stored credentials
    if (testWifi()) {
      Serial.println("WiFi Connected!!!");
    } else {
      ap_mode();
      apmode = true;
    }
  }
}

// Main loop to handle incoming requests and WiFi scan
void loop() {
  if (apmode) {
    server.handleClient();  // Listen for web clients

    // Only scan once and store results
    if (!scanComplete) {
      int n = WiFi.scanComplete();
      if (n == -2) WiFi.scanNetworks(true); // If no scan, initiate
      else if (n >= 0) {
        // Build HTML table from scanned networks
        scannedNetworks = "<table border='1' style='width:100%; border-collapse:collapse;'>";
        scannedNetworks += "<tr><th>SSID</th><th>Signal Strength</th></tr>";
        for (int i = 0; i < n; ++i) {
          scannedNetworks += "<tr><td>" + WiFi.SSID(i) + "</td><td>" + String(WiFi.RSSI(i)) + " dBm</td></tr>";
        }
        scannedNetworks += "</table>";
        scanComplete = true;  // Prevent further scanning
        WiFi.scanDelete();  // Free memory
      }
    }
  }
}

// Reads SSID, Password and DevID from EEPROM
void readData() {
  EEPROM.begin(512);
  Serial.println("Reading From EEPROM..");
  ssid = pass = devid = "";  // Clear previous values

  // Read SSID
  for (int i = 0; i < 20; i++) {
    char ch = EEPROM.read(i);
    if (isPrintable(ch)) ssid += ch;
  }
  // Read Password
  for (int i = 20; i < 40; i++) {
    char ch = EEPROM.read(i);
    if (isPrintable(ch)) pass += ch;
  }
  // Read DevID
  for (int i = 40; i < 60; i++) {
    char ch = EEPROM.read(i);
    if (isPrintable(ch)) devid += ch;
  }

  // Print credentials to serial monitor
  Serial.println("WiFi SSID: " + ssid);
  Serial.println("WiFi Password: " + pass);
  Serial.println("DevID: " + devid);

  EEPROM.end();
}

// Function to set device into Access Point mode for configuration
void ap_mode() {
  digitalWrite(2, HIGH); // Turn on LED to indicate AP mode
  Serial.println("AP Mode. Please connect to http://192.168.4.1 to configure");
  apmode = true;
  WiFi.mode(WIFI_AP_STA);  // Enable both AP and Station mode to allow scan
  WiFi.softAP("ESP32_eeprom", "");  // Start AP with default SSID and no password
  WiFi.scanNetworks(true);  // Start scanning for WiFi networks
  Serial.println(WiFi.softAPIP());  // Print AP IP address
  launchWeb(0);  // Start web server
}

// Start the web server
void launchWeb(int webtype) {
  createWebServer(webtype);
  server.begin();
}

// Define routes and page content for web server
void createWebServer(int webtype) {
  if (webtype == 0) {
    // Main configuration page
    server.on("/", []() {
      // HTML structure for configuration page
      content = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
          <meta name="viewport" content="width=device-width, initial-scale=1">
          <style>
            body { font-family: Arial; text-align: center; padding: 20px; background: #f4f4f4; }
            h1 { color: #333; }
            form { background: white; padding: 20px; border-radius: 10px; display: inline-block; }
            input, label { display: block; margin: 10px auto; width: 80%; max-width: 300px; }
            input[type="text"], input[type="password"] {
              padding: 10px; border: 1px solid #ccc; border-radius: 5px; }
            .button {
              background-color: #3CBC8D; color: white; padding: 10px 20px; border: none;
              border-radius: 5px; cursor: pointer; font-size: 16px; }
            table { margin: 10px auto; width: 90%; border: 1px solid #ccc; }
            th, td { padding: 8px; text-align: left; border: 1px solid #ccc; }
          </style>
        </head>
        <body>
          <h1>WiFi Manager</h1>
          <p><strong>Stored SSID:</strong> )rawliteral" + ssid + R"rawliteral(</p>
          <p><strong>Stored Password:</strong> )rawliteral" + pass + R"rawliteral(</p>
          <p><strong>Stored Device ID:</strong> )rawliteral" + devid + R"rawliteral(</p>
          <form method='get' action='setting'>
            <label>WiFi SSID:</label>
            <input type='text' name='ssid' required>
            <label>WiFi Password:</label>
            <input type='password' name='password'>
            <label>Device ID:</label>
            <input type='text' name='devid' required>
            <input class='button' type='submit' value='Save Settings'>
          </form>
          <h3>Available Networks:</h3>
      )rawliteral" + scannedNetworks + R"rawliteral(
        </body>
        </html>
      )rawliteral";

      server.send(200, "text/html", content);
    });

    // Save new WiFi and device ID settings to EEPROM
    server.on("/setting", []() {
      String ssidw = server.arg("ssid");
      String passw = server.arg("password");
      String devidw = server.arg("devid");
      writeData(ssidw, passw, devidw);
      server.send(200, "text/html", "<h2>Settings Saved Successfully</h2><p>The device will now restart to apply the new configuration.</p>");
      delay(2000);
      digitalWrite(2, LOW); // Turn off LED before restart
      ESP.restart();
    });

    // Route to clear all EEPROM data
    server.on("/clear", []() {
      clearData();
      server.send(200, "text/html", "<h2>All Data Cleared</h2><p>The device will restart now. Please reconnect and reconfigure if needed.</p>");
      delay(2000);
      ESP.restart();
    });
  }
}

// Write provided data to EEPROM, with safe character length checks
void writeData(String a, String b, String c) {
  clearData();  // Clear existing EEPROM content before writing new data
  EEPROM.begin(512);
  Serial.println("Writing to EEPROM...");
  for (int i = 0; i < 20; i++) EEPROM.write(i, (i < a.length()) ? a[i] : 0);
  for (int i = 0; i < 20; i++) EEPROM.write(20 + i, (i < b.length()) ? b[i] : 0);
  for (int i = 0; i < 20; i++) EEPROM.write(40 + i, (i < c.length()) ? c[i] : 0);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Write Successful");
}

// Clear all EEPROM contents to default (0)
void clearData() {
  EEPROM.begin(512);
  Serial.println("Clearing EEPROM (first 60 bytes only)...");
  // Only clear the bytes used for SSID (0–19), Password (20–39), DevID (40–59)
  for (int i = 0; i < 60; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  EEPROM.end();
}

// Try connecting to WiFi using current credentials stored in ssid/pass
boolean testWifi() {
  WiFi.softAPdisconnect();  // Ensure AP mode is disabled
  WiFi.disconnect();        // Disconnect any previous connection
  WiFi.mode(WIFI_STA);      // Set mode to Station (client)
  WiFi.begin(ssid.c_str(), pass.c_str());
  int c = 0;
  while (c < 50) {  // Try for ~45 seconds
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);
      Serial.println(WiFi.localIP());
      return true;
    }
    Serial.print(".");
    delay(900);
    c++;
  }
  Serial.println("Connection timed out...");
  return false;
}
