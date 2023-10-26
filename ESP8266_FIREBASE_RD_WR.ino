#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

const char* ssid = "MOTO G5S(2.4)";
const char* password = "44441199";
const int redLedPin = D1;

const char* api_key = "AIzaSyBIpPvFSdqEKapPRm9Q8TjEhUCaop39jVQ";
const char* db_url = "https://earthquake-detection-adxl-default-rtdb.firebaseio.com";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
bool ledstatus = false;
unsigned long sendDataPrevMillis = 0;


void setup() {
  pinMode(redLedPin, OUTPUT);
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  Serial.print("IP: ");
  Serial.print(WiFi.localIP());
  Serial.print("");

  config.api_key = api_key;
  config.database_url = db_url;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.print("sign up ok");
    signupOK = true;
  } else {
    Serial.print(config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.getBool(&fbdo, "/LED/digital")) {
      if (fbdo.dataType() == "boolean") {
        ledstatus = fbdo.boolData();
        Serial.print("Successful READ from " + fbdo.dataPath() + ": " + ledstatus + " {" + fbdo.dataType() + "}");
        digitalWrite(redLedPin, ledstatus);
      }
    } else {
      Serial.print("READ FAILED: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setBool(&fbdo, "LED/digital", !ledstatus)) {
      Serial.print("LED is written as " + ledstatus);
    } else {
      Serial.print("WRITE FAILED: " + fbdo.errorReason());
    }
  }
}