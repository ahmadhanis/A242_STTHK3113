// Filename: ESP32WiFiManager.h
// Author: Ahmad Hanis
// Description: Arduino library for ESP32 WiFi Manager with EEPROM-based credential storage and AP fallback.
// Last Updated: April 23, 2025

#ifndef ESP32_WIFI_MANAGER_H
#define ESP32_WIFI_MANAGER_H

#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>

class ESP32WiFiManager {
  public:
    ESP32WiFiManager(int ledPin = 2, int buttonPin = 0);
    void begin();
    void handleClient();

  private:
    int _ledPin, _buttonPin;
    String _ssid, _pass, _devid, _scannedNetworks;
    bool _apMode, _scanComplete;
    WebServer _server;

    void _readData();
    void _writeData(const String& ssid, const String& pass, const String& devid);
    void _clearData();
    void _apModeInit();
    bool _testWifi();
    void _startWebServer();
    void _defineRoutes();
};

#endif
