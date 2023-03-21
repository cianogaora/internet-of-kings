
#define LIGHT_SENSOR_PIN 35 // ESP32 pin GIOP36 (ADC0)

#include <FirebaseESP32.h>
#include  <WiFi.h>

#define FIREBASE_HOST "https://internet-of-kings-default-rtdb.europe-west1.firebasedatabase.app/"
#define WIFI_SSID "Sean's Galaxy S21+ 5G" // Change the name of your WIFI
#define WIFI_PASSWORD "toui5120" // Change the password of your WIFI
#define FIREBASE_Authorization_key "5qAg2nAy9XCvbcAv4yGV4na4DXXQqzDZpFPnr8hB"

FirebaseData firebaseData;
FirebaseJson json;

void setup() {
  // initialize serial communication at 9600 bits per second:
 Serial.begin(9600);
   WiFi.begin (WIFI_SSID, WIFI_PASSWORD);
   Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Firebase.begin(FIREBASE_HOST,FIREBASE_Authorization_key);
  
}

void loop() {
  // reads the input on analog pin (value between 0 and 4095)
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
}

void main(){
    Firebase.setFloat(firebaseData, "/light", analogValue);
    delay(500);
}