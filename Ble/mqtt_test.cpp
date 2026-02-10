#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

const char* WIFI_SSID = "cloudwifi-1501";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const uint16_t AWS_IOT_PORT = 8883;

static const char* rootCA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

String deviceCert;
String privateKey;
String endpoint;

WiFiClientSecure  net;
PubSubClient      mqtt(net);

String unescapeEnvValue(String value) {
  value.replace("\\n", "\n");
  value.replace("\\\"", "\"");
  value.replace("\\\\", "\\");
  return value;
}

bool loadEnvFile() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return false;
  }

  File envFile = SPIFFS.open("/.env", FILE_READ);
  if (!envFile) {
    Serial.println("Failed to open /.env");
    return false;
  }

  while (envFile.available()) {
    String line = envFile.readStringUntil('\n');
    line.trim();
    if (line.length() == 0 || line.startsWith("#")) {
      continue;
    }

    int eq = line.indexOf('=');
    if (eq <= 0) {
      continue;
    }

    String key = line.substring(0, eq);
    String value = line.substring(eq + 1);
    key.trim();
    value.trim();

    if (value.startsWith("\"") && value.endsWith("\"") && value.length() >= 2) {
      value = value.substring(1, value.length() - 1);
    }

    value = unescapeEnvValue(value);

    if (key == "DEVICE_CERT") {
      deviceCert = value;
    } else if (key == "PRIVATE_KEY") {
      privateKey = value;
    } else if (key=="ENDPOINT"){
        endpoint = value;
    }
  }

  envFile.close();
  return deviceCert.length() > 0 && privateKey.length() > 0;
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("→ Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println(" connected!");
}

void connectAWS() {
  if (deviceCert.isEmpty() || privateKey.isEmpty()) {
    Serial.println("Missing DEVICE_CERT or PRIVATE_KEY");
    return;
  }

  net.setCACert(rootCA);
  net.setCertificate(deviceCert.c_str());
  net.setPrivateKey(privateKey.c_str());

  mqtt.setServer(endpoint.c_str(), AWS_IOT_PORT);

  Serial.print("→ Connecting to AWS IoT");
  while (!mqtt.connected()) {

    if (mqtt.connect("ESP32_Client")) {
      Serial.println(" connected!");
    } else {
      Serial.print('.');
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  loadEnvFile();
  connectWiFi();
  connectAWS();
}

void loop() {
  if (!mqtt.connected()) {
    connectAWS();
  }
  mqtt.loop();

  // 1) Build your JSON payload
  StaticJsonDocument<256> doc;
  doc["device"] = "ESP32_Node";
  doc["uptime"] = millis();
  doc["rssi"]   = WiFi.RSSI();

  String payload;
  serializeJson(doc, payload);

  // 2) Publish to your chosen topic
  const char* topic = "sensors/esp32/status";
  bool ok = mqtt.publish(topic, payload.c_str());
  if (ok) {
    Serial.println("✓ Published JSON:");
    Serial.println(payload);
  } else {
    Serial.println("✗ Publish failed");
  }

  delay(5000);
}
