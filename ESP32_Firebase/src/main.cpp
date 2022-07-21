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

struct sensorInfo
{
  String temperature = "fields/temperature/doubleValue";
  String humidity = "fields/humidity/doubleValue";
  String CO2 = "fields/CO2/doubleValue";
  String noise = "fields/noise/doubleValue";
  String TVOC = "fields/TVOC/doubleValue";
  String lightLUX = "fields/lightLUX/doubleValue";
  String CO = "fields/CO/doubleValue";
} getValue;
String sensor[7] = {getValue.temperature, getValue.humidity, getValue.CO2, getValue.noise, getValue.TVOC, getValue.CO};
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
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 10000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    String documentPath = "House/Room_1";

    FirebaseJson content;
    
    count = count + 1;
    for (int i = 0; i <= 7; i++)
    {
      int testValue = i;
      content.set(sensor[i], String(testValue).c_str());
    }

    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "temperature,humidity,CO2,noise,TVOC,CO"))
    {
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }

    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw()))
    {
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
      return;
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }

    // count++;
    // count = count + 1;

    // Serial.print("Get a document... ");
    //   if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), ""))
    //   {
    //     Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

    //     // Create a FirebaseJson object and set content with received payload
    //     FirebaseJson payload;
    //     payload.setJsonData(fbdo.payload().c_str());

    //     // Get the data from FirebaseJson object
    //     FirebaseJsonData jsonData;
    //     payload.get(jsonData, "fields/temperature/doubleValue", true);
    //     Serial.println(jsonData.stringValue);
    //   }
    // }
  }
}