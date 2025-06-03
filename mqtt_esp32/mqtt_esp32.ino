#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Wi-Fi credentials
const char* ssid     = "your-ap";
const char* password = "your-pass";

// Public Mosquitto test broker
const char* mqttServer = "test.mosquitto.org";
const int   mqttPort   = 1883;

// DHT11 on GPIO19
#define DHTPIN    19
#define DHTTYPE   DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // 1) Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println(" ✓");

  // 2) Configure MQTT server
  mqttClient.setServer(mqttServer, mqttPort);

  // 3) Connect to MQTT
  Serial.print("MQTT connecting to ");
  Serial.print(mqttServer);
  Serial.print(":");
  Serial.print(mqttPort);
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32_TestClient")) {
      Serial.println(" ✓");
    } else {
      Serial.print(" ✗, rc=");
      Serial.print(mqttClient.state());
      Serial.println("; retrying in 2s");
      delay(2000);
    }
  }
}

void loop() {
  // Read DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f", t);
    mqttClient.publish("esp32/dht11/temperature", buf);
    snprintf(buf, sizeof(buf), "%.1f", h);
    mqttClient.publish("esp32/dht11/humidity", buf);
    Serial.printf("T:%s°C  H:%s%%\n", buf, buf);
  } else {
    Serial.println("DHT read failed");
  }

  // Keep MQTT alive
  mqttClient.loop();
  delay(5000);
}
