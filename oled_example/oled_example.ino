#include <Adafruit_GFX.h>      // OLED graphics support
#include <Adafruit_SSD1306.h>  // OLED screen driver
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define I2C_SDA 4
#define I2C_SCL 15

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  Serial.println("OLED INIT...");
  setDisplay("Welcome to Universiti", "Utara Malaysia", "Hello");
}

void setDisplay(String a, String b, String c) {
  display.display();
  delay(2000);  // Pause for 2 seconds
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);       // Normal 1:1 pixel scale
  display.setTextColor(WHITE);  // Draw white text
  display.println(a);
  display.setCursor(0, 10);
  display.println(b);
  display.setCursor(0, 20);
  display.println(c);
  display.display();
}
void loop() {
  // put your main code here, to run repeatedly:
}
