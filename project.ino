#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "time.h"
#include "DHT.h"
#include "light.ino"


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "hello David"
#define WIFI_PASSWORD "pooass69"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBbX9Akf7VSG04-KI97ohzxkjt0KHIbW1o"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "dave@gmail.com"
#define USER_PASSWORD "david2023"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://internet-of-kings-default-rtdb.europe-west1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String presPath = "/pressure";
String timePath = "/timestamp";
String lightPath ="/light";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// DHT sensor
#define DHT11PIN 33
DHT dht(DHT11PIN, DHT11);
//light sensor
#define LIGHT_SENSOR_PIN 35

float temperature;
float humidity;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);

  dht.begin();
  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop(){

  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    json.set(tempPath.c_str(), String(dht.readTemperature()));
    json.set(humPath.c_str(), String(dht.readHumidity()));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());

    //light code
    int analogValue = analogRead(LIGHT_SENSOR_PIN);

      Serial.print("Analog Value = ");
      Serial.print(analogValue);   // the raw analog reading

      // We'll have a few threshholds, qualitatively determined
      if (analogValue < 40) {
        Serial.println(" => Dark");
      } else if (analogValue < 800) {
        Serial.println(" => Dim");
      } else if (analogValue < 2000) {
        Serial.println(" => Light");
      } else if (analogValue < 3200) {
        Serial.println(" => Bright");
      } else {
        Serial.println(" => Very bright");
      }    
      Firebase.setFloat(fbdo, "/light", analogValue);
      delay(500);
  }
}