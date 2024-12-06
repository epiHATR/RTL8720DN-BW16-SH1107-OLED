#include <Arduino.h>
#undef min
#undef max

#include "util.h"
#include "struct.h"
#include "deauther.h"
#include "icon.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ezButton.h>

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
#include <ezButton.h>

extern Adafruit_SH1107 display;
extern ezButton BTN_NEXT;
extern ezButton BTN_PREV;
extern ezButton BTN_SEL;
extern ezButton BTN_BACK;
extern uint8_t selectedMenu;
extern bool isMainMenu;
extern bool isPageLoaded;

// deauthenticate scanning
bool isDeauthenticating = false;
bool isChangingSSID = false;
uint8_t activeSSID = 0;
uint8_t networkCount = 0;
uint8_t activeInput = 0;

// for deauther
#define FRAMES_PER_DEAUTH 5
char *buttonsString = "De-Authenticate";

std::vector<WiFiScanResult> scan_results;

rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result) {
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
    scan_results.push_back(result);
  }
  networkCount = scan_results.size();
  return RTW_SUCCESS;
}

uint8_t scanWifiNetworks() {
  display.setTextColor(SH110X_WHITE);
  display.setCursor(15, 45);
  display.print("Scanning Networks");
  display.display();
  Serial.println("Scanning WiFi networks for DeAuthentication (5s)...");
  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    delay(5000);
    return 0;
  } else {
    return 1;
  }
}

void showScanResult() {
  display.clearDisplay();
  displayFrame();
  showScreenTitle("DE-AUTHENTICATE");
  String ssid = formatSSID(scan_results[activeSSID].ssid);
  drawInputBox(0, 40, (String(ssid)).c_str(), activeInput == 0);
  display.setCursor(5, 65);
  display.setTextColor(SH110X_WHITE);
  display.print("[" + scan_results[activeSSID].bssid_str + "]");
  drawButton(15, 92, 100, buttonsString, activeInput == 1);
}

void showDeautherScreen() {
  display.clearDisplay();
  displayFrame();
  showScreenTitle("DE-AUTHENTICATE");
  scanWifiNetworks();
  showScanResult();
}

void startDeauther() {
  if (isDeauthenticating) {
    Serial.println("Deauthenticating");
  }
}

void deautherSetup() {
  WiFi.disablePowerSave();
  wifi_on(RTW_MODE_PROMISC);
  wifi_enter_promisc_mode();
  if (!isPageLoaded) {
    isPageLoaded = true;
    showDeautherScreen();
  }

  display.display();
}

void deautherLoop() {
  if (BTN_NEXT.isReleased()) {
    if (isChangingSSID) {
      activeSSID += 1;
      if (activeSSID >= networkCount) {
        activeSSID = networkCount - 1;
      }
    } else {
      if (activeInput < 1) {
        activeInput += 1;
      } else {
        activeInput = 1;
      }
    }
    isPageLoaded = false;
  }

  if (BTN_PREV.isReleased()) {
    if (isChangingSSID) {
      if (activeSSID > 0) {
        activeSSID -= 1;
      } else {
        activeSSID = 0;
      }
    } else {
      if (activeInput > 0) {
        activeInput -= 1;
      } else {
        activeInput = 0;
      }
    }
    isPageLoaded = false;
  }

  if (BTN_SEL.isReleased()) {
    if (activeInput == 0) {
      if (isChangingSSID) {
        isChangingSSID = false;
      } else {
        isChangingSSID = true;
      }
    } else {
      if (!isDeauthenticating) {
        buttonsString = "Stop";
        isDeauthenticating = true;
      } else {
        isDeauthenticating = false;
        buttonsString = "De-Authenticate";
      }
    }
    isPageLoaded = false;
  }

  if (BTN_BACK.isReleased()) {
    isMainMenu = true;
    selectedMenu = -1;
    isPageLoaded = false;
  }

  if (!isPageLoaded) {
    showScanResult();
    isPageLoaded = true;
  } else {
    startDeauther();
  }
  display.display();
}
