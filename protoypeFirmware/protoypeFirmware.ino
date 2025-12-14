#include "defines.h"

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_INA219 ina219;

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float current_mA = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // Neopixel onboard
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(100, 0, 100));
  pixels.show();

  // INA219
  if (! ina219.begin())
    Serial.println("Failed to find INA219 chip");

  // OLED 
  display.begin(OLED_ADDR, true);
  display.display();
  display.clearDisplay();
}

void loop() {

  // INA219
  current_mA = ina219.getCurrent_mA();
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");

  // Show on OLED
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Pomiar pradu:");
  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
  display.println(current_mA);
  display.display();

  delay(2000);
}
