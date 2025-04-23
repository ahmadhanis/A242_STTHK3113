// Filename: ESP32WiFiManager.cpp
// Author: Ahmad Hanis
// Implementation of ESP32 WiFi Manager with EEPROM-based credential storage

#include "ESP32WiFiManager.h"

ESP32WiFiManager::ESP32WiFiManager(int ledPin, int buttonPin)
  : _ledPin(ledPin), _buttonPin(buttonPin), _server(80), _apMode(false), _scanComplete(false) {}

void ESP32WiFiManager::begin() {
  pinMode(_ledPin, OUTPUT);
  digitalWrite(_ledPin, LOW);
  Serial.begin(115200);
  Serial.println("Waiting for 5 seconds!");
  _readData();

  if (_ssid.length() == 0) {
    Serial.println("No SSID found in EEPROM, forcing AP mode.");
    _apModeInit();
    return;
  }

  delay(5000);
  pinMode(_buttonPin, INPUT_PULLUP);

  if (digitalRead(_buttonPin) == 0) {
    _apMode = true;
    _apModeInit();
  } else {
    if (_testWifi()) {
      Serial.println("WiFi Connected!!!");
    } else {
      _apModeInit();
      _apMode = true;
    }
  }
}

void ESP32WiFiManager::handleClient() {
  if (_apMode) {
    _server.handleClient();

    if (!_scanComplete) {
      int n = WiFi.scanComplete();
      if (n == -2) WiFi.scanNetworks(true);
      else if (n >= 0) {
        _scannedNetworks = "<table border='1' style='width:100%; border-collapse:collapse;'>";
        _scannedNetworks += "<tr><th>SSID</th><th>Signal Strength</th></tr>";
        for (int i = 0; i < n; ++i) {
          _scannedNetworks += "<tr><td>" + WiFi.SSID(i) + "</td><td>" + String(WiFi.RSSI(i)) + " dBm</td></tr>";
        }
        _scannedNetworks += "</table>";
        _scanComplete = true;
        WiFi.scanDelete();
      }
    }
  }
}

void ESP32WiFiManager::_readData() {
  EEPROM.begin(512);
  _ssid = _pass = _devid = "";
  for (int i = 0; i < 20; i++) _ssid += char(EEPROM.read(i));
  for (int i = 20; i < 40; i++) _pass += char(EEPROM.read(i));
  for (int i = 40; i < 60; i++) _devid += char(EEPROM.read(i));
  EEPROM.end();
  Serial.println("WiFi SSID: " + _ssid);
  Serial.println("WiFi Password: " + _pass);
  Serial.println("DevID: " + _devid);
}

void ESP32WiFiManager::_writeData(const String& ssid, const String& pass, const String& devid) {
  _clearData();
  EEPROM.begin(512);
  for (int i = 0; i < 20; i++) EEPROM.write(i, (i < ssid.length()) ? ssid[i] : 0);
  for (int i = 0; i < 20; i++) EEPROM.write(20 + i, (i < pass.length()) ? pass[i] : 0);
  for (int i = 0; i < 20; i++) EEPROM.write(40 + i, (i < devid.length()) ? devid[i] : 0);
  EEPROM.commit();
  EEPROM.end();
}

void ESP32WiFiManager::_clearData() {
  EEPROM.begin(512);
  for (int i = 0; i < 60; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  EEPROM.end();
}

void ESP32WiFiManager::_apModeInit() {
  digitalWrite(_ledPin, HIGH);
  Serial.println("AP Mode. Please connect to http://192.168.4.1 to configure");
  _apMode = true;
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32_eeprom", "");
  WiFi.scanNetworks(true);
  Serial.println(WiFi.softAPIP());
  _startWebServer();
}

bool ESP32WiFiManager::_testWifi() {
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid.c_str(), _pass.c_str());
  for (int c = 0; c < 50; c++) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(900);
  }
  Serial.println("Connection timed out...");
  return false;
}

void ESP32WiFiManager::_startWebServer() {
  _defineRoutes();
  _server.begin();
}

void ESP32WiFiManager::_defineRoutes() {
  _server.on("/", [&]() {
    String page = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head><meta name='viewport' content='width=device-width, initial-scale=1'>
      <style>
        body { font-family: Arial; text-align: center; padding: 20px; background: #f4f4f4; }
        form { background: white; padding: 20px; border-radius: 10px; display: inline-block; }
        input, label { display: block; margin: 10px auto; width: 80%; max-width: 300px; }
        input[type="text"], input[type="password"] {
          padding: 10px; border: 1px solid #ccc; border-radius: 5px; }
        .button {
          background-color: #3CBC8D; color: white; padding: 10px 20px; border: none;
          border-radius: 5px; cursor: pointer; font-size: 16px; }
        table { margin: 10px auto; width: 90%; border: 1px solid #ccc; }
        th, td { padding: 8px; text-align: left; border: 1px solid #ccc; }
      </style></head><body>
      <h1>WiFi Manager</h1>
      <p><strong>Stored SSID:</strong> )rawliteral" + _ssid + R"rawliteral(</p>
      <p><strong>Stored Password:</strong> )rawliteral" + _pass + R"rawliteral(</p>
      <p><strong>Stored Device ID:</strong> )rawliteral" + _devid + R"rawliteral(</p>
      <form method='get' action='setting'>
        <label>WiFi SSID:</label>
        <input type='text' name='ssid' required>
        <label>WiFi Password:</label>
        <input type='password' name='password'>
        <label>Device ID:</label>
        <input type='text' name='devid' required>
        <input class='button' type='submit' value='Save Settings'>
      </form><h3>Available Networks:</h3>
    )rawliteral" + _scannedNetworks + R"rawliteral(</body></html>
    )rawliteral";
    _server.send(200, "text/html", page);
  });

  _server.on("/setting", [&]() {
    _writeData(_server.arg("ssid"), _server.arg("password"), _server.arg("devid"));
    _server.send(200, "text/html", "<h2>Settings Saved Successfully</h2><p>The device will now restart to apply the new configuration.</p>");
    delay(2000);
    digitalWrite(_ledPin, LOW);
    ESP.restart();
  });

  _server.on("/clear", [&]() {
    _clearData();
    _server.send(200, "text/html", "<h2>All Data Cleared</h2><p>The device will restart now. Please reconnect and reconfigure if needed.</p>");
    delay(2000);
    ESP.restart();
  });
}
