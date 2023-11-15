#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
// For OLED Screen
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

const char* ssid = "MOTO G5S(2.4)";
const char* password = "44441199";
// const int redLedPin = D1;

const char* api_key = "AIzaSyBIpPvFSdqEKapPRm9Q8TjEhUCaop39jVQ";
const char* db_url = "https://earthquake-detection-adxl-default-rtdb.firebaseio.com";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

unsigned long sendDataPrevMillis = 0;
char dataPayload[100] = "";
char tempX[20];
char tempY[20];
char tempZ[20];
bool SMSstatus;

// For OLED Screen
/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c  //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void clearOLED(void) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
}

void setup() {
  // pinMode(redLedPin, OUTPUT);
  Serial.begin(115200);

  // For OLED Screen
  delay(250);                        // wait for the OLED to power up
  display.begin(i2c_Address, true);  // Address 0x3C default
                                     //display.setContrast (0); // dim display

  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wifi");
  clearOLED();
  display.println("Connecting to Wifi");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.println(".");
  display.display();

  }
  Serial.println("\nConnected to WiFi");
  clearOLED();
  display.println("\nConnected to WiFi");
  display.display();



  Serial.print("IP: ");
  Serial.print(WiFi.localIP());
  Serial.print("");

  /* Initialise the sensor */
  if (!accel.begin()) {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1)
      ;
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);

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
  /* Get a new sensor event */
  sensors_event_t event;
  accel.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: ");
  Serial.print(event.acceleration.x);
  Serial.print("  ");
  Serial.print("Y: ");
  Serial.print(event.acceleration.y);
  Serial.print("  ");
  Serial.print("Z: ");
  Serial.print(event.acceleration.z);
  Serial.print("  ");
  Serial.println("m/s^2 ");
  dtostrf(event.acceleration.x, 5, 2, tempX);
  dtostrf(event.acceleration.y, 5, 2, tempY);
  dtostrf(event.acceleration.z, 5, 2, tempZ);
  // Format the string with x, y, and z values separated by commas
  strcat(dataPayload, "{");
  strcat(dataPayload, tempX);
  strcat(dataPayload, ",");
  strcat(dataPayload, tempY);
  strcat(dataPayload, ",");
  strcat(dataPayload, tempZ);
  strcat(dataPayload, "}");
  Serial.print(dataPayload);
  clearOLED();
  display.println(dataPayload);
  display.display();

  delay(100);

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.getBool(&fbdo, "/LED/digital")) {
      if (fbdo.dataType() == "boolean") {
        // Serial.print(fbdo);
        SMSstatus = fbdo.boolData();
        // Serial.print("Successfuly sent SMS" + fbdo.dataPath() + ": " + SMSstatus + " {" + fbdo.dataType() + "}");
        // digitalWrite(redLedPin, SMSstatus);
      }
    } else {
      Serial.print("READ FAILED: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setString(&fbdo, "/LED/txt", dataPayload)) {
      Serial.print(dataPayload);
      clearOLED();
      display.println(dataPayload);
  display.display();

      delay(500);
    } else {
      Serial.print("WRITE FAILED: " + fbdo.errorReason());
    }
  }
  strcpy(dataPayload, "");  // Copying an empty string to dataPayload
}