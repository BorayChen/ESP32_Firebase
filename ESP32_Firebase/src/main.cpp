#include <Arduino.h>
#include <SPIFFS.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FB_Const.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "key"
#define WIFI_PASSWORD "abcd@1234"
#define API_KEY "AIzaSyBKGAjaTmgFiHp6Mnb5kX1-0LhZdRW7Mak"
#define FIREBASE_PROJECT_ID "esp32iot-1b164"
#define DATABASE_URL "https://esp32iot-1b164-default-rtdb.firebaseio.com/"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
void setup()
{
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("oooooooook");
    signupOK = true;
  }
  else
  {
    Serial.println("errrrrror");
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  // put your setup code here, to run once:
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    String documentPath = "House/Room_1";

    FirebaseJson content;

    content.set("fields/temperature/doubleValue", String(count).c_str());
    content.set("fields/humidity/doubleValue", String(count).c_str());

    if(Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "temperature,humidity")){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }else{
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())){
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }else{
      Serial.println(fbdo.errorReason());
    }
    count++;
  }
}
