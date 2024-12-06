#include <Arduino.h>

#include "util.h"
#include "struct.h"
#include "spam_ap.h"
#include "icon.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ezButton.h>

// reuse global variables
extern Adafruit_SH1107 display;
extern ezButton BTN_NEXT;
extern ezButton BTN_PREV;
extern ezButton BTN_SEL;
extern ezButton BTN_BACK;
extern uint8_t selectedMenu;
extern bool isMainMenu;
extern bool isPageLoaded;

// spam beacon
bool isSpamSetup = false;
bool isSpamming = false;

const unsigned char* spammingIcon[1] = {
  tiktokChannel
};

void showSpamScreen() {
  display.clearDisplay();
  displayFrame();
  showScreenTitle("WIFI CLONNING");
  display.drawBitmap(32, 32, spammingIcon[0], 64, 64, SH110X_WHITE);

  display.setCursor(15, 105);
  display.setTextColor(SH110X_WHITE);
  display.print("Scan for Updates.");
  display.display();
}

void spamSetup() {
  if (!isSpamSetup) {
    showSpamScreen();
    isSpamSetup = true;
  }
}

void startSpamming() {
}

void spamLoop() {
}