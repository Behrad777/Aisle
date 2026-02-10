#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid       = "YOUR_SSID";
const char* password   = "YOUR_PASS";
const char* mqttServer = "broker.example.com";
const uint16_t mqttPort= 1883;
const char* mqttUser   = "user";
const char* mqttPass   = "pass";

WiFiClient     wifiClient;
PubSubClient   mqtt(wifiClient);

int scanTime = 2; //2s scanning
int cartId = 0x1;
StaticJsonDocument<200> payload;

BLEScan* pBLEScan;
BLEAdvertisedDevice* nearestDevices[3];

void find3Nearest(BLEScanResults* foundDevices) {
    nearestDevices[0] = nearestDevices[1] = nearestDevices[2] = nullptr;

    int rssi1 = std::numeric_limits<int>::min();
    int rssi2 = rssi1;
    int rssi3 = rssi1;

    for (int i = 0; i < foundDevices->getCount(); i++) {
        BLEAdvertisedDevice* dev = foundDevices->getDevice(i);
        int r = dev->getRSSI();

        if (r > rssi1) {
            rssi3 = rssi2;   nearestDevices[2] = nearestDevices[1];
            rssi2 = rssi1;   nearestDevices[1] = nearestDevices[0];
            rssi1 = r;       nearestDevices[0] = dev;
        }
        else if (r > rssi2) {
            rssi3 = rssi2;   nearestDevices[2] = nearestDevices[1];
            rssi2 = r;       nearestDevices[1] = dev;
        }
        else if (r > rssi3) {
            rssi3 = r;       nearestDevices[2] = dev;
        }
    }
}
void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

void connectMQTT() {
  mqtt.setServer(mqttServer, mqttPort);
  while (!mqtt.connected()) {
    if (mqtt.connect("ESP32Node", mqttUser, mqttPass)) {
    } else {
      delay(500);
    }
  }
}

void setup() {
  //some time to initialize
  delay(1000);
  Serial.begin(115200);
  connectWifi();
  connectMQTT();
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  //now we know how many devices we scanned
  BLEScanResults *foundDevices = pBLEScan->start(scanTime);
  //reset the json payload
  payload.clear();
  find3Nearest(foundDevices); //extracts the 3 nearest beacons from the scan

  payload[nearestDevices[0]->getAddress().toString().c_str()] = nearestDevices[0]->getRSSI();
  payload[nearestDevices[1]->getAddress().toString().c_str()] = nearestDevices[1]->getRSSI();
  payload[nearestDevices[2]->getAddress().toString().c_str()] = nearestDevices[2]->getRSSI();

  String printablePayload;
  serializeJson(payload, printablePayload);
  Serial.println(printablePayload);


  //free the buffers
  pBLEScan->clearResults();
  delay(500);
}
