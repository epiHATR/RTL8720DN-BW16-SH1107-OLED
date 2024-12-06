#include <Arduino.h>

#include "util.h"
#include "struct.h"
#include "setting.h"
#include "icon.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ezButton.h>

extern Adafruit_SH1107 display;

void settingSetup() {
}

void settingLoop() {
  display.clearDisplay();
  displayFrame();
  showScreenTitle("SETTINGS");
  display.display();
}