#include <Arduino.h>
#undef max

#include "util.h"
#include "pin.h"
#include "struct.h"
#include "scan_ap.h"
#include "icon.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ezButton.h>

#include "rtc.h"
#include "vector"
#include "wifi_conf.h"
#include "map"
#include "wifi_cust_tx.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "debug.h"
#include "WiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"

// reuse global variables
extern Adafruit_SH1107 display;
extern ezButton BTN_NEXT;
extern ezButton BTN_PREV;
extern ezButton BTN_SEL;
extern ezButton BTN_BACK;
extern uint8_t selectedMenu;
extern bool isMainMenu;
extern bool isPageLoaded;

//WIFI scanning
bool isWifiScanning = false;
uint8_t wifiCount = 0;
int totalPages = 0;
uint8_t currentPage = 0;
uint8_t itemsPerPage = 4;

std::vector<WiFiScanResult> wifi_scan_results;

void printLegend(bool isShowing, uint8_t numerator, uint8_t denominator) {

  // Format the text as "numerator / denominator"
  String text = String(numerator) + "/" + String(denominator);

  display.setTextSize(1);              // Set the text size (1 is small size)
  display.setTextColor(SH110X_WHITE);  // Set text color to white (or your preferred color)

  // Get the text bounds to calculate its width and height
  int16_t x1, y1;                  // Variables to store the text bounding box coordinates
  uint16_t textWidth, textHeight;  // Variables to store text dimensions
  display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);

  // Calculate X position to center the text
  uint8_t x = (128 - textWidth) / 2;

  // Set the Y position to be near the bottom
  uint8_t y = 128 - textHeight;  // Bottom of the screen, adjusted for text height

  display.fillRect(x, y, textWidth, textHeight, SH110X_BLACK);

  // Set the cursor position and print the text
  display.setCursor(x, y);
  if (isShowing) {
    display.print(text);
  }
}

void sortNetworksByRSSI() {
  for (int i = 0; i < wifiCount - 1; i++) {
    for (int j = 0; j < wifiCount - i - 1; j++) {
      if (wifi_scan_results[j].rssi < wifi_scan_results[j + 1].rssi) {
        // Swap networks[j] and networks[j + 1]
        WiFiScanResult temp = wifi_scan_results[j];
        wifi_scan_results[j] = wifi_scan_results[j + 1];
        wifi_scan_results[j + 1] = temp;
      }
    }
  }
}

void printScrollbar() {
  uint8_t screenWidth = 128;
  uint8_t screenHeight = 128;

  // Margins
  const uint8_t topMargin = 30;
  const uint8_t bottomMargin = 15;

  // Draw dotted line with top and bottom margins
  for (int y = topMargin; y < screenHeight - bottomMargin; y += 4) {
    display.drawPixel(screenWidth - 2, y, SH110X_WHITE);  // Dotted line on the right side
  }

  // Effective height for the scrollbar
  uint8_t effectiveHeight = screenHeight - topMargin - bottomMargin;

  // Scrollbar dimensions
  uint8_t scrollbarWidth = 3;  // Width of the scrollbar
  uint8_t rectHeight = effectiveHeight / totalPages;
  display.fillRect(128 - scrollbarWidth, 30 + (currentPage * rectHeight), scrollbarWidth, rectHeight, SH110X_WHITE);
}

void drawRSSIStrength(int x, uint8_t y, int rssi) {
  uint8_t lineHeight = 2;  // Height of each vertical line segment
  uint8_t lineWidth = 1;   // Width of each vertical line
  uint8_t spacing = 1;     // Spacing between lines

  // Determine the number of bars based on RSSI strength
  uint8_t bars = 0;
  if (rssi >= -55) {  // Strong signal
    bars = 3;
  } else if (rssi >= -65) {  // Medium signal
    bars = 2;
  } else {  // Weak signal
    bars = 1;
  }

  // Draw the RSSI bars
  for (int i = 0; i < bars; i++) {
    uint8_t barX = x + i * (lineWidth + spacing);  // Calculate x-position of each bar
    uint8_t barHeight = (i + 1) * lineHeight;      // Bar height increases with each bar
    uint8_t barY = y - barHeight;                  // Adjust y-position for height

    // Draw a filled rectangle for the bar
    display.fillRect(barX, barY, lineWidth, barHeight, SH110X_WHITE);
  }
}

rtw_result_t wifiScanResultHandler(rtw_scan_handler_result_t *scan_result) {
  rtw_scan_result_t *record;
  if (scan_result->scan_complete == 0) {
    record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;
    WiFiScanResult result;
    result.ssid = String((const char *)record->SSID.val);
    result.channel = record->channel;
    result.rssi = record->signal_strength;
    memcpy(&result.bssid, &record->BSSID, 6);
    char bssid_str[] = "XX:XX:XX:XX:XX:XX";
    snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", result.bssid[0], result.bssid[1], result.bssid[2], result.bssid[3], result.bssid[4], result.bssid[5]);
    result.bssid_str = bssid_str;
    wifi_scan_results.push_back(result);
  }
  wifiCount = wifi_scan_results.size();
  totalPages = wifiCount / itemsPerPage;

  return RTW_SUCCESS;
}

int scanNetworks() {
  display.setTextColor(SH110X_WHITE);
  display.setCursor(15, 45);
  display.print("Scanning Networks");
  display.display();
  Serial.println("Scanning WiFi networks for Access Point (5s)...");
  wifi_scan_results.clear();
  if (wifi_scan_networks(wifiScanResultHandler, NULL) == RTW_SUCCESS) {
    delay(5000);
    Serial.println(" done!\n");
    return 0;
  } else {
    Serial.println(" failed!\n");
    return 1;
  }
}

void showWifiScanResult() {
  display.clearDisplay();
  displayFrame();
  showScreenTitle("WIFI SCANNED");
  sortNetworksByRSSI();
  uint8_t startIndex = currentPage * itemsPerPage;
  uint8_t endIndex = min(startIndex + itemsPerPage, wifiCount);

  for (int i = startIndex; i < endIndex; i++) {
    uint8_t x = 0;
    uint8_t y = 30 + (i - startIndex) * (20 + 2);

    display.drawLine(x, y, x, y + 15, SH110X_WHITE);
    display.drawLine(x, y + 15, x + 120, y + 15, SH110X_WHITE);
    display.setCursor(x + 5, y + 3);
    display.print(formatSSID(wifi_scan_results[i].ssid));
    drawRSSIStrength(128 - 13, y + 8, wifi_scan_results[i].rssi);
  }

  printScrollbar();
  printLegend(true, currentPage + 1, totalPages);
  display.display();
}

void showScanScreen() {
  display.clearDisplay();
  displayFrame();
  showScreenTitle("WIFI SCANNING");
  scanNetworks();
  showWifiScanResult();
}

void scanSetup() {
  WiFi.disablePowerSave();
  wifi_on(RTW_MODE_PROMISC);
  wifi_enter_promisc_mode();
  if (!isPageLoaded) {
    showScanScreen();
    isPageLoaded = true;
    isWifiScanning = true;
  }
  display.display();
}

void scanLoop() {
  if (!isPageLoaded) {
    if (!isWifiScanning) {
      showScanScreen();
    } else {
      showWifiScanResult();
    }
    isPageLoaded = true;
  }

  if (BTN_NEXT.isReleased()) {
    currentPage += 1;
    if (currentPage >= totalPages - 1) {
      currentPage = totalPages - 1;
    }
    isPageLoaded = false;
  }

  if (BTN_PREV.isReleased()) {
    currentPage -= 1;
    if (currentPage <= 0) {
      currentPage = 0;
    }
    isPageLoaded = false;
  }
  display.display();
}