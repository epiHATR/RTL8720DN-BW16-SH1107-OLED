#include "menu.h"
#include "struct.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define OLED_RESET -1

Adafruit_SH1107 display = Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 100000);

void setup() {
  Serial.begin(115200);
  display.begin(0x3C, OLED_RESET);
  delay(10);
  display.clearDisplay();
  display.display();
  menuSetup();
}

void loop() {
  menuLoop();
}