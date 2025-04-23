#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

WebServer server(80);

String ssid, pass, devid, content;
bool apmode = false;  //Default AP mode status


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  //Serial monitor
  Serial.println("Waiting for 5 second!");
  readData();
  delay(5000);
  pinMode(0, INPUT_PULLUP);
  if (digitalRead(0) == 0) {
    apmode = true;
    ap_mode();
  } else {
    readData();
    if (testWifi()) {  //If found try to connect to WiFi AP
      Serial.println("WiFi Connected!!!");
    } else {  //If failed set AP mode
      ap_mode();
      apmode = true;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (apmode) {
    server.handleClient();  //If apmode is true listen to incoming connection.
  }
}

void readData() {  //Read from EEPROM memory
  EEPROM.begin(512);
  Serial.println("Reading From EEPROM..");
  for (int i = 0; i < 20; i++) {  //Reading for SSID max for 20 characters long
    if (char(EEPROM.read(i)) > 0) {
      ssid += char(EEPROM.read(i));
    }
  }
  for (int i = 20; i < 40; i++) {  //Reading for WiFi password max for 40 characters long
    if (char(EEPROM.read(i)) > 0) {
      pass += char(EEPROM.read(i));
    }
  }
  for (int i = 40; i < 60; i++) {  //Reading for WiFi password max for 40 characters long
    if (char(EEPROM.read(i)) > 0) {
      devid += char(EEPROM.read(i));
    }
  }

  Serial.println("Wifi ssid: " + ssid);
  Serial.println("Wifi password: " + pass);
  Serial.println("Devid: " + devid);

  EEPROM.end();
}

void ap_mode() {  //Function to set as AP mode
  Serial.println("AP Mode. Please connect to http://192.168.4.1 to configure");
  apmode = true;
  const char* ssidap = "ESP32_eeprom";
  const char* passap = "";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidap, passap);
  Serial.println(WiFi.softAPIP());
  launchWeb(0);  //Create web server
}

void launchWeb(int webtype) {
  createWebServer(webtype);
  server.begin();
}

void createWebServer(int webtype) {  //Create web server HTML interface
  if (webtype == 0) {                //AP mode
    server.on("/", []() {
      content = "<html><head><style>.button {background-color: #3CBC8D;";
      content += "color: white;padding: 5px 10px;text-align: center;text-decoration: none;";
      content += "display: inline-block;font-size: 14px;margin: 4px 2px;cursor: pointer;}";
      content += "input[type=text],[type=password]{width: 100%;padding: 5px 10px;";
      content += "margin: 5px 0;box-sizing: border-box;border: none;";
      content += "background-color: #3CBC8D;color: white;}</style></head><body>";
      content += "<h1>WIFI MANAGER</h1><br>";
      content += "<h3>Current Settings</h3>";
      content += "<table><tr><td><label> WiFi SSID</label></td><td><label>" + ssid + "</label></td></tr>";
      content += "<tr><td><label> WiFi Pasword</label></td><td><label>" + pass + "</label></td></tr>";
      content += "<tr><td><label> Dev ID</label></td><td><label>" + devid + "</label></td></tr></table><br><br>";
      content += "<form method='get' action='setting'>";
      content += "<h3>New WiFi Settings</h3>";
      content += "<table><tr><td><label>WiFi SSID</label></td><td><input type='text' name = 'ssid' lenght=32 ></td></tr>";
      content += "<tr><td><label> WiFi Password</label></td><td><input type='password' name = 'password' lenght=32></td></tr>";
      content += "<tr><td><label> Device ID</label></td><td><input type='text' name = 'devid' lenght=32></td></tr>";
      content += "<tr><td></td><td><input class=button type='submit'></td></tr></table></form>";
      content += "</body></html>";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {  //Respond handler for form submit button
      String ssidw = server.arg("ssid");
      String passw = server.arg("password");
      String devid = server.arg("devid");
      writeData(ssidw, passw, devid);  //Store data from HTML form to EEPROM
      content = "Success..please reboot";
      server.send(200, "text/html", content);
      delay(2000);
      ESP.restart();
    });
    server.on("/clear", []() {  //Clear data in EEPROM
      clearData();
      delay(2000);
      ESP.restart();
    });
  }
}

void writeData(String a, String b, String c) {  //Writing WiFi credentials to EEPROM
  clearData();                        //Clear current EEPROM memory
  EEPROM.begin(512);
  Serial.println("Writing to EEPROM...");
  for (int i = 0; i < 20; i++) {  //Writing for SSID max for 20 characters long
    EEPROM.write(i, a[i]);
  }
  for (int i = 20; i < 40; i++) {  //Reading for password max for 40 characters long
    EEPROM.write(i, b[i - 20]);
    Serial.println(b[i]);
  }
  for (int i = 40; i < 60; i++) {  //Reading for password max for 40 characters long
    EEPROM.write(i, c[i - 20]);
    Serial.println(c[i]);
  }
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Write Successfull");
}

void clearData() {  //Clear current EEPROM memory function
  EEPROM.begin(512);
  Serial.println("Clearing EEPROM ");
  for (int i = 0; i < 512; i++) {
    Serial.print(".");
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  EEPROM.end();
}

boolean testWifi() {  //Test WiFi connection function
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int c = 0;
  while (c < 50) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);
      Serial.println(WiFi.status());
      Serial.println(WiFi.localIP());
      delay(100);
      return true;
    }
    Serial.print(".");
    delay(900);
    c++;
  }
  Serial.println("Connection time out...");
  return false;
}