#include <Arduino.h>
#undef max
#undef min

#include "util.h"
#include "struct.h"
#include "icon.h"
#include "menu.h"
#include "spam_ap.h"
#include "scan_ap.h"
#include "deauther.h"
#include "setting.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ezButton.h>

extern Adafruit_SH1107 display;
extern bool isWifiScanned;

ezButton BTN_NEXT(PB1);
ezButton BTN_PREV(PB2);
ezButton BTN_SEL(PB3);
ezButton BTN_BACK(PA27);

enum Direction {
  RightToLeft,
  LeftToRight
};

#define NUMBER_OF_MENU_ITEM 4
char* menuItems[NUMBER_OF_MENU_ITEM] = { "WIFI SCANNING", "DE-AUTHENTICATE", "WIFI CLONNING", "SETTINGS" };
const unsigned char* menuItemIcons[NUMBER_OF_MENU_ITEM] = {
  menuItem_scan,
  menuItem_deauther,
  menuItem_beacon,
  menuItem_setting
};

bool isMainMenu = true;
uint8_t selectedMenu = -1;
bool isPageLoaded = false;
bool animationTriggered = false;
uint8_t selectedMenuIndex = 0;
Direction direction = RightToLeft;

void _showMenuItem(uint8_t index, Direction direction) {
  uint8_t startX, targetX;

  // Set start and target positions based on direction
  if (direction == RightToLeft) {
    startX = 128;  // Start off-screen to the right
    targetX = 40;  // Target X position (centered horizontally)
  } else {
    startX = -48;  // Start off-screen to the left
    targetX = 40;  // Target X position (centered horizontally)
  }

  const float animationDuration = 0.25;                    // Animation duration in seconds
  const uint8_t fps = 60;                                  // Frames per second (smooth animation)
  const uint8_t animationSteps = animationDuration * fps;  // Total steps
  const uint8_t frameTime = 1000 / fps;                    // Time per frame in milliseconds

  unsigned long lastFrameTime = 0;  // Track the last frame time
  uint8_t step = 0;                 // Track the current step of the animation

  while (step <= animationSteps) {
    unsigned long currentTime = millis();

    // Check if it's time to update the frame
    if (currentTime - lastFrameTime >= frameTime) {
      lastFrameTime = currentTime;

      float t = (float)step / animationSteps;  // Normalized time [0.0, 1.0]
      float easedT = t * t * (3 - 2 * t);      // Smoothstep easing

      uint8_t currentX = startX + (targetX - startX) * easedT;  // Interpolated X position

      // Clear the display area
      display.fillRect(0, 20, 128, 100, SH110X_BLACK);

      // Draw the bitmap at the current position
      display.drawBitmap(currentX, 40, menuItemIcons[index], 48, 48, SH110X_WHITE);

      // Update the display
      display.display();

      // Move to the next step
      step++;
    }
  }

  // Calculate text width for dynamic centering
  int16_t x1, y1;                  // Variables to store the text bounding box coordinates
  uint16_t textWidth, textHeight;  // Variables to store text dimensions
  display.getTextBounds(menuItems[index], 0, 0, &x1, &y1, &textWidth, &textHeight);

  // Calculate X position to center the text
  uint8_t textX = (128 - textWidth) / 2;

  // Display the text dynamically centered
  display.setCursor(textX, 45 + 48);  // Centered horizontally, below the bitmap
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.println(menuItems[index]);
  display.display();
}

void _showAuthorInfo() {
  display.drawBitmap(10, 3, bitmap_header, 109, 20, SH110X_WHITE);
  display.drawBitmap(30, (128 - 10), footer_logo, 75, 11, SH110X_WHITE);
  display.display();
}

void _showScreenBox() {
  display.clearDisplay();
  displayFrame();
  _showAuthorInfo();
}

void menuSetup() {
  selectedMenuIndex = 0;
  BTN_NEXT.setDebounceTime(50);
  BTN_PREV.setDebounceTime(50);
  BTN_SEL.setDebounceTime(50);
  BTN_BACK.setDebounceTime(50);
  _showScreenBox();
}

void menuLoop() {
  BTN_NEXT.loop();
  BTN_PREV.loop();
  BTN_SEL.loop();
  BTN_BACK.loop();

  if (BTN_NEXT.isReleased()) {
    if (isMainMenu) {
      direction = RightToLeft;
      animationTriggered = false;
      if (selectedMenuIndex < (NUMBER_OF_MENU_ITEM - 1)) {
        selectedMenuIndex += 1;
      } else {
        selectedMenuIndex = 0;
      }
    }
  }

  if (BTN_PREV.isReleased()) {
    if (isMainMenu) {
      direction = LeftToRight;
      animationTriggered = false;
      if (selectedMenuIndex > 0) {
        selectedMenuIndex -= 1;
      } else {
        selectedMenuIndex = (NUMBER_OF_MENU_ITEM - 1);
      }
    }
  }

  if (BTN_SEL.isReleased()) {
    if (isMainMenu) {
      isMainMenu = false;
      selectedMenu = selectedMenuIndex;
      switch (selectedMenuIndex) {
        case 0:
          {
            scanSetup();
            break;
          }
        case 1:
          {
            deautherSetup();
            break;
          }
        case 2:
          {
            spamSetup();
            break;
          }
        case 3:
          {
            settingLoop();
            break;
          }
        default: break;
      }
    }
  }

  if (BTN_BACK.isReleased()) {
    isMainMenu = true;
    selectedMenu = -1;
    animationTriggered = false;
    isPageLoaded = false;
    _showScreenBox();
  }

  switch (selectedMenu) {
    case 0:
      {
        scanLoop();
        break;
      }
    case 1:
      {
        deautherLoop();
        break;
      }
    case 2:
      {
        spamLoop();
        break;
      }
    case 3:
      {
        settingLoop();
        break;
      }
    default:
      {
        if (!animationTriggered) {
          _showMenuItem(selectedMenuIndex, direction);
          animationTriggered = true;
        }
        break;
      }
  }
}
