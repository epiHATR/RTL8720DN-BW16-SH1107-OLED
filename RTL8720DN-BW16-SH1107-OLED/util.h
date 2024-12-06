#ifndef util_H
#define util_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

void showScreenTitle(char* text);
void displayFrame();
void drawInputBox(int x, int y, const char* label, bool highlight);
void drawButton(int x, int y, int width, const char* label, bool highlight);
String formatSSID(String ssid);
void drawLabel(int x, int y, String label);
#endif