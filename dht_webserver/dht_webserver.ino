#include "DHT.h"
#include <WebServer.h>
#include <WiFi.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float h = 0.0;
float t = 0.0;
WebServer server(80); 

String ssid = "DHTServer";
String password = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  server.begin();
  delay(500);
  server.on("/", handle_root);
}

void handle_root() {
  String content = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><title>DHT11</title></head><body><center><h3>DHT 11</h3><p>Temperature" + String(t) + "<br>Humidity" + String(h) + "<br></center></body></html>";
  server.send(200, "text/html", content);
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  delay(2000);
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();
  Serial.print(F("Humidity: "));
  Serial.println(h);
  Serial.print(F("%  Temperature: "));
  Serial.println(t);
}
