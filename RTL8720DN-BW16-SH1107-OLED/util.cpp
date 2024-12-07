#include <Arduino.h>
#include "util.h"
#include "icon.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

extern Adafruit_SH1107 display;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

String formatSSID(String ssid) {
  if (ssid == "") {
    return "HIDDEN";  // For hidden networks
  } else {
    // Truncate the SSID if it's longer than 12 characters
    if (ssid.length() > 12) {
      return ssid.substring(0, 17);
    } else {
      return ssid;  // Return the SSID as is
    }
  }
}

void drawLabel(int x, int y, String label) {
  int16_t x1, y1;                  // Variables to store text bounding box coordinates
  uint16_t textWidth, textHeight;  // Variables to store text dimensions

  // Get the text bounds for the given label
  display.getTextBounds(label.c_str(), 0, 0, &x1, &y1, &textWidth, &textHeight);

  // Calculate the starting position for centered text
  int textX = x - (textWidth / 2);
  int textY = y - (textHeight / 2);

  // Set cursor and print the label
  display.setTextColor(SH110X_WHITE);
  display.setCursor(textX, textY);
  display.print(label);
}

void drawButton(int x, int y, int width, const char* label, bool highlight) {
  int height = 24;  // Shorter height
  int radius = 5;   // Radius for rounded corners

  // Draw the button
  if (highlight) {
    display.fillRoundRect(x, y, width, height, radius, SH110X_WHITE);  // Filled for highlight
    display.setTextColor(SH110X_BLACK);                                // Inverted text color
  } else {
    display.drawRoundRect(x, y, width, height, radius, SH110X_WHITE);  // Outline for non-highlight
    display.setTextColor(SH110X_WHITE);
  }

  // Calculate text dimensions
  int16_t x1, y1;                  // Variables to store the text bounding box coordinates
  uint16_t textWidth, textHeight;  // Variables to store text dimensions
  display.getTextBounds(label, 0, 0, &x1, &y1, &textWidth, &textHeight);

  // Center the text within the button
  int textX = x + (width - textWidth) / 2;    // Center text horizontally
  int textY = y + (height - textHeight) / 2;  // Center text vertically

  display.setCursor(textX, textY);
  display.setTextSize(1);
  display.print(label);
}


void drawInputBox(int x, int y, const char* label, bool highlight) {
  int width = 128 - x;
  int height = 18;

  // Draw the box
  if (highlight) {
    display.fillRect(x, y, width, height, SH110X_WHITE);  // Filled box for highlight
    display.setTextColor(SH110X_BLACK);                   // Inverted text color
    display.setCursor(128 - 10, y + 5);
    display.print("<");
  } else {
    display.drawRect(x, y, width, height, SH110X_WHITE);  // Outline box for non-highlight
    display.setTextColor(SH110X_WHITE);
  }

  // Print the label inside the box
  display.setCursor(x + 5, y + 5);
  display.setTextSize(1);
  display.print(label);
}

void _draw90InCorners() {
  // Dimensions for the rectangles
  int width1 = 10;   // Horizontal rectangle width
  int height1 = 1;   // Horizontal rectangle height
  int width2 = 1;    // Vertical rectangle width
  int height2 = 10;  // Vertical rectangle height

  // Top-left corner (reverted angle)
  display.drawRect(0, 0, width1, height1, SH110X_WHITE);  // Horizontal
  display.drawRect(0, 0, width2, height2, SH110X_WHITE);  // Vertical

  // Top-right corner (reverted angle)
  display.drawRect(128 - width1, 0, width1, height1, SH110X_WHITE);  // Horizontal
  display.drawRect(128 - width2, 0, width2, height2, SH110X_WHITE);  // Vertical

  // Bottom-left corner (reverted angle)
  display.drawRect(0, 128 - height1, width1, height1, SH110X_WHITE);  // Horizontal
  display.drawRect(0, 128 - height2, width2, height2, SH110X_WHITE);  // Vertical

  // Bottom-right corner (reverted angle)
  display.drawRect(128 - width1, 128 - height1, width1, height1, SH110X_WHITE);  // Horizontal
  display.drawRect(128 - width2, 128 - height2, width2, height2, SH110X_WHITE);  // Vertical
  display.display();
}

void showScreenTitle(char* text) {
  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Calculate the horizontal center for the text
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int16_t x = (SCREEN_WIDTH - w) / 2;
  int16_t y = 5;  // Top of the screen

  // Set cursor position and print text
  display.setCursor(x, y);
  display.print(text);
  display.setCursor(5, 5);
  display.cp437(true);
  display.print("<");
  display.display();
  display.cp437(false);
}

void displayFrame() {
  _draw90InCorners();
}