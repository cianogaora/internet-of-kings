

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "time.h"
#include "DHT.h"
#include "ESP32Servo.h"
#include <WebServer.h>
#include <ArduinoJson.h>
#include "bitches.h"


int melody[] = {
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_E5, NOTE_A6,
  NOTE_E5, NOTE_D5,

 NOTE_E5, NOTE_B6,
 NOTE_E5, NOTE_E5, NOTE_C6,
 NOTE_B6, NOTE_G5,

 NOTE_E5, NOTE_G5, NOTE_E6,
 NOTE_E5, NOTE_D5,
 NOTE_D5, NOTE_B5,
 NOTE_FS5, NOTE_E5,
 NOTE_E5,NOTE_E5

};

int noteDurations[] = {
  4, 4,
  8, 16, 8,
  8, 8,

  4, 4,
  8, 16, 8,
  8, 8,
  
  8,8,8,
  16,8,
  16,8,
  8,2,

  4,4
};


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


// Insert your network credentials
#define WIFI_SSID "getoffyourphone"
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


int mic = 27;
int noise = 0;

const int BUZZER_PIN = 12;


// Database main path (to be updated in setup with the user UID)
String databasePath;
String databasePath1;
String databasePath2;
String databasePath3;




// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String presPath = "/pressure";
String timePath = "/timestamp";
String micPath = "/noise";


// Parent Node (to be updated in every loop)
String parentPath;
String parentPath1;
String parentPath2;
String parentPath3;

String ms_value;


int timestamp;
FirebaseJson json;
FirebaseJson json1;
FirebaseJson json2;
FirebaseJson json3;




IPAddress IPaddr = WiFi.localIP();
auto first = IPaddr[0];
String second = String(IPaddr[1]) + ".";
String third = String(IPaddr[2]) + ".";
String fourth = String(IPaddr[3]);
String newIP = first + second + third + fourth;



StaticJsonDocument<250> jsonDocument;
char buffer[250];
WebServer server(80);


const char* ntpServer = "pool.ntp.org";


// DHT sensor
#define DHT11PIN 33
DHT dht(DHT11PIN, DHT11);


// servo motor
Servo myservo;


float temperature;
float humidity;
float window = 0;
int time_to_wake = 0;
long int alarm_time = 0;
bool alarm_set = false; 
bool check_server = true;



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

void setup_routing() {          
  server.on("/post", HTTP_POST, handlePost);    
  // start server    
  server.begin();    
}

void handlePost() {
  if (server.hasArg("plain") == false) {
    //handle error here
    Serial.print("received req, no arg");
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  String ms_value = body.substring(body.indexOf("=") + 1, body.indexOf("&"));
  String set_value = body.substring(body.lastIndexOf("=") + 1);
  ms_value = ms_value.toInt();  

  Serial.println(ms_value); // Output: ms=21600000
  Serial.println(set_value);
  if(set_value.equals("true")) {
    alarm_set = true;
    server.send(200, "application/json", "{Timer Set!}");
    time_to_wake = ms_value.toInt();
    alarm_time = getTime() + time_to_wake;
    Serial.println("alarm time=");
    Serial.print(alarm_time);
  }
}


void setup(){
 Serial.begin(115200);
 dht.begin();
 initWiFi();
 configTime(0, 0, ntpServer);

 pinMode(BUZZER_PIN, OUTPUT);


 // set up the POST request handler
  // start the server
 Serial.println("Server started");


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


  setup_routing();

 // Update database path
 databasePath = "/UsersData/" + uid + "/Tempreadings";
 databasePath1 = "/UsersData/" + uid + "/Humreadings";
 databasePath2 = "/UsersData/" + uid + "/Micreadings";
 databasePath3 = "/UsersData/" + uid + "/IPaddr";
}


void loop(){
 noise = analogRead(mic);
 temperature = dht.readTemperature();
 humidity = dht.readHumidity();

 //if (check_server == true) {
  Serial.println("Checking server...");
  server.handleClient();
  delay(5000);
 //}

 if(alarm_set == true){ 
   if (alarm_set == true && alarm_time < getTime()) {


    int size = sizeof(noteDurations) / sizeof(int);
    for (int i=0; i<4; i++) {
      for (int thisNote = 0; thisNote < size; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(BUZZER_PIN, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration*0.5;
      delay(pauseBetweenNotes);
      noTone(BUZZER_PIN);
      alarm_set == false;
      alarm_time = 1780640450;
      }    
   }
  }
 }

 if (temperature>25 && window == 0) {
   myservo.write(180);
   window = 1;
 }


 if (window == 1 && temperature < 25) {
   myservo.write(0);
   window = 0;
 }



 // Send new readings to database
 if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
   sendDataPrevMillis = millis();


   //Get current timestamp
   timestamp = getTime();
   Serial.print ("time: ");
   Serial.println (timestamp);




   parentPath = databasePath;
   parentPath1 = databasePath1;
   parentPath2 = databasePath2;
   parentPath3 = databasePath3;


   Serial.println(WiFi.localIP());
  
   json.set(timestamp, String(dht.readTemperature()));
   json1.set(timestamp, String(dht.readHumidity()));
   json2.set(timestamp, String(analogRead(mic)));
   json3.set(timestamp, String(WiFi.localIP()));
  
   Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
   Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath1.c_str(), &json1) ? "ok" : fbdo.errorReason().c_str());
   Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath2.c_str(), &json2) ? "ok" : fbdo.errorReason().c_str());
   Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath3.c_str(), &json3) ? "ok" : fbdo.errorReason().c_str());
 }
}





