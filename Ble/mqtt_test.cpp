#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// —— 1) Your Wi-Fi credentials —————————————————————————————
const char* WIFI_SSID = "cloudwifi-1501";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// —— 2) Your AWS IoT Core endpoint & port ————————————————
const char* AWS_IOT_ENDPOINT = "arn:aws:iot:us-east-2:107767828424:thing/esp32_gateway";
const uint16_t AWS_IOT_PORT = 8883;

// —— 3) Your device certificates (PEM format) —————————————
// Amazon Root CA 1
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

// Device certificate
static const char* deviceCert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUTOkF40EjQBJ8eBDFkwqF7KadPoEwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDYxNDIwMTUy
MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPY99qUEsgmqt2WqUokd
gewdsg7ulBSbKz86wdiH0r0qq/HOORy5ona9XLfChILHeMz1mq5342zTGiLNfwpP
qfT83y0LgZmHhM4cIkhimr5zl99pYovDKCS7wVtrkvadxwhbC0WQAsxn0KmzRHrX
VJCDbdAOSvuHNM8heUjFDJfnhMipaKrCCtAPgOFs703i/xHSh+Z5Xec6J0bfaP9N
+uS2KU00dT6ncoqqRi/dG4ZHEJcKfJ396jXz199QQ3xR53xlnWWkSsj0olu6UNJf
kwXcG8xK3fYCJA2zOLpmd6/t5Zhh1yupj2RqagX2g/tSx3bL0bNZcUKTfhP4a8lp
0w0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUCn1y4trgDA6gEW5ud0jCj51gi98wHQYD
VR0OBBYEFEYu7Exnrub8LUyO68LkGKAT1rj1MAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQA5LUuqtXrz0ESse4onL57/Huf5
9I4u8SIEMOZ/M5Z8Tv0dTApyJnNr/5du7uLNnETmWSHY5IQhW+Rj+rbUcYE8yJSd
pfwIbKZcL+7msb+fOLma/M2zF9iJeceNDADn7aMw+tIp/JUiRXT/g8hw2TudgzJr
TaChDtuh9TkQNIU4Ltj/zwBjwoTTXa6BWbp5Yn5zw2oN6c8rSzC/tOV+VwV6D2j8
exz7nwjoRUz2kV/m/y7Wjwzv87me4Y58YVKvkz8aYzIra731ecxzNzbAoMpZvT3j
lQUvwJr3mOZ+/6OFIjFMx15WJsco1KNK4TvZqsdhm0//rf8ifJnOhwd1VeoC
-----END CERTIFICATE-----
)EOF";

// Device private key
static const char* privateKey = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA9j32pQSyCaq3ZapSiR2B7B2yDu6UFJsrPzrB2IfSvSqr8c45
HLmidr1ct8KEgsd4zPWarnfjbNMaIs1/Ck+p9PzfLQuBmYeEzhwiSGKavnOX32li
i8MoJLvBW2uS9p3HCFsLRZACzGfQqbNEetdUkINt0A5K+4c0zyF5SMUMl+eEyKlo
qsIK0A+A4WzvTeL/EdKH5nld5zonRt9o/0365LYpTTR1PqdyiqpGL90bhkcQlwp8
nf3qNfPX31BDfFHnfGWdZaRKyPSiW7pQ0l+TBdwbzErd9gIkDbM4umZ3r+3lmGHX
K6mPZGpqBfaD+1LHdsvRs1lxQpN+E/hryWnTDQIDAQABAoIBAQCoHN+yA8WlMecp
wqG8uOeFkzSIWzu/4bwh2/SR5xDqUyK24Gs/gEE5vch6MJQVUSfZDmj/ahxbpLg4
QIHWQVMwTh/z+eIONmZl7PE+6ArrsbFjNuivsJpMkd7NkmDwfAFxY+8LG+m9MbdU
1G4RS8eRLhmfrNJmbQWR0DGki9swLOo22Wc2gfEcu72BxOiZXkOsrLsWmE5qvIbv
tqsI+/RaFzmgWt5MDqHwMI84kOA415/UMxqS+RYPDp4WgbN+UrJzr7XY2y2y79M5
bv7ov01OiTkwsT1gmRXbnNt4z81vrYGXzMYdQCHdCw8Q1g08oZlBOK43B0m465B+
aZqCEjBlAoGBAPwzaF3zDzuNQuXbPme3f46sUv/RUlxEI0nUL8xZG6Q6+F2TnpU1
/4uxelwyobq3CmEKguInAEzj7Xp9XKMtNW+hrDz0rY8aUtAq+vJsQdI7QYMK+0om
iD9Oa3H26Nq52wzA69qohF65u3Jmif2Cg6WaUY7l/hMvXw4dfp+fHNn7AoGBAPnz
k4YKa4PuwwDe7e/QzPkbn+2tuKvx53mlNazjlVeyyiOKvARfNNXbOhcgOu7fM3tL
fjineifZoi9f80hs9xoXyKOI6gRCch0c4wAteLVguSmxvXFGqNBBF8cY7eLQJUwr
Y3ADbVUp4UmumkHgvI+LcA0Z6g8cSf5sl1mQa8CXAoGAEjmk0SU/dI5Ax1uQpu4b
ybOqS9JUoVZLNoqe6aWG2ddpG1oPm7jLPs0UipUeGuAfX+PikXVAEE5oWvG0QBaE
Rgzo5X7JDA5a7bAtcldgalDd6xpgDnu6XNjN6u5s5WGI8uL+KE8FQdMqOJowXqGP
4efE/Q6hzcEenWaEQAS/kQECgYEAmFS3Utik39oypCQ5NI+3bBZG6K6YPsqF/SAP
z1dAHBuNkA2sOqnfZHpgM9njsNL86CaBPXvsOb8sVd0PQtON4SwfjnMR1Bi+vTlu
qxwLVdcl4eA+CXw7w/f3ytJJoJKkOIC08H5WMrH8aFBuGLojkQQ0/PcPK7hc7B84
OlCHKu8CgYEAtayKzVi/4a9nF4H+OJI3pX5UW4wc7XQaiGJDddRm3/z9StVQMqAG
rH1cgXiNE/ENXqRKXGTNYGBAO7M9xuIzSmYQ3Ub6L5Ie4sNfO5yk3SFh0AFZrmFd
G2OONT2fkJFBLAxp1WnkPwmnmhnsDXbTL1c6qvrYDV655k2E+sgKOFQ=
-----END RSA PRIVATE KEY-----
)EOF";

WiFiClientSecure  net;
PubSubClient      mqtt(net);

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
  net.setCACert(rootCA);
  net.setCertificate(deviceCert);
  net.setPrivateKey(privateKey);

  mqtt.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);

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
