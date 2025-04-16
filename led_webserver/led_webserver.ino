#include <WebServer.h>
#include <WiFi.h>

String ssid = "WiFiLED";
String password = "";
WebServer server(80); 
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  server.begin();
  delay(500);
  server.on("/", handle_root);

  server.on("/on", []() {
    digitalWrite(2, LOW);
    delay(100);
    handle_root();
  });

  server.on("/off", []() {
    digitalWrite(2, HIGH);
    delay(100);
    handle_root();
  });
}

void handle_root() {
  String content = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><title>Relay Controller</title></head><body><center><h3>Fan Control System</h3><a href='/on'><button>Off</button></a><br><br><a href='/off'><button>On</button></a></center></body></html>";
  server.send(200, "text/html", content);
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
}