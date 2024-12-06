#ifndef STRUCT_H
#define STRUCT_H

#include <Arduino.h>

// Define the Network structure
struct Network {
  String ssid;
  int rssi;
  String encryptionTypeEx;
  String encryptionType;
};

typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];
  short rssi;
  uint8_t channel;
} WiFiScanResult;

struct WiFiSignal {
  String ssid;
  unsigned char addr[6];
  unsigned int channel;
  signed char rssi;
  String bssid_str;
  uint8_t bssid[6];
  String wps_type;
  String bss_type;
};

extern Network networks[];
extern const int MAX_NETWORK_COUNT;

#endif
