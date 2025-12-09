#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "env.h"

// -----------------------------------------------------------------
// ----------------------------- CONFIG -----------------------------
#define WIFI_SSID      MY_WIFI_SSID
#define WIFI_PASSWORD  MY_WIFI_PASSWORD

#define Web_API_KEY    MY_API_KEY
#define DATABASE_URL   MY_DATABASE_URL
#define USER_EMAIL     MY_USER_EMAIL
#define USER_PASS      MY_USER_PASS

constexpr int STM_RX_PIN = 4; // ESP32 RX (to STM32 TX PA2)
constexpr int STM_TX_PIN = 17; // ESP32 TX (to STM32 RX PA3)

HardwareSerial stmSerial(1);
// -----------------------------------------------------------------

// Forward declaration
void processData(AsyncResult &aResult);

// Auth
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

// Firebase components
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;


// ---- Wi-Fi connect ----
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

// ---- Firebase init ----
void setupFirebase() {
  // SSL client config (simple/insecure for tests)
  ssl_client.setInsecure();
  // ssl_client.setConnectionTimeout(1000);
  // ssl_client.setHandshakeTimeout(5);

  // Initialize app with user auth
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  initializeApp(aClient, app, getAuth(user_auth), processData, "authTask");

  // Bind Realtime Database and set URL
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
}

// ---- Send dummy byte + timestamp ----
// void sendDummyByte() {
//   // Dummy byte: 0..255 cycling each second
//   uint8_t value = (millis() / 1000) % 256;

//   // Unix time in ms (from NTP-synced clock)
//   struct timeval tv;
//   gettimeofday(&tv, nullptr);
//   int64_t sentAtMs = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

//   Serial.printf("Sending value=%u, sentAtMs=%lld\n",
//                 value, (long long)sentAtMs);

//   // Write value
//   Database.set<int>(
//     aClient,
//     "/stm32/lastByte/value",
//     (int)value,
//     processData,
//     "RTDB_Send_Value"
//   );

//   // Write timestamp
//   Database.set<double>(
//     aClient,
//     "/stm32/lastByte/sentAtMs",
//     (double)sentAtMs,
//     processData,
//     "RTDB_Send_SentAt"
//   );
// }

// ---- Send one byte to Firebase with server timestamp ----
void sendByteToFirebase(uint8_t value) {

  // object_t rootObj, objValue, objTs;
  // JsonWriter writer;

  // /value = <byte>
  // writer.create(objValue, "/value", static_cast<int>(value));

  // /sentAtServer/.sv = "timestamp"
  // This becomes: "sentAtServer": { ".sv": "timestamp" }
  // writer.create(objTs, "/sentAtServer/.sv", "timestamp");
  // send now = millis() intstead of server timestamp
  //   struct timeval tv;
  // gettimeofday(&tv, nullptr);
  // int64_t sentAtMs = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
  // writer.create(objTs, "/sentAtMs", static_cast<double>(sentAsMs));
  
  //send only a byte value no object
  Database.set<uint8_t>(
    aClient,
    "/stm32/lastByte/value",
    value,
    processData,
    "RTDB_Send_Value"
  );

  // Join both into rootObj
  // writer.join(rootObj, 2, objValue, objTs);

  Serial.printf("Sending byte 0x%02X (%u) with Firebase server timestamp\n",
                value, value);

  // Write whole object to /stm32/lastByte
  // Database.set<object_t>(
  //   aClient,
  //   "/stm32/lastByte",
  //   rootObj,
  //   processData,
  //   "RTDB_Send_LastByte"
  // );
}

// void setupTimeNTP()
// {
//   // Bangkok = UTC+7
//   const long gmtOffset_sec = 7 * 3600;
//   const int daylightOffset_sec = 0;

//   configTime(gmtOffset_sec, daylightOffset_sec,
//              "pool.ntp.org", "time.nist.gov");

//   Serial.print("Syncing time");
//   struct tm timeinfo;
//   while (!getLocalTime(&timeinfo))
//   {
//     Serial.print(".");
//     delay(500);
//   }
//   Serial.println("\nTime synced.");
// }

void setupUART() {
  stmSerial.begin(115200, SERIAL_8N1, STM_RX_PIN, STM_TX_PIN);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  connectWiFi();
  setupFirebase();
  setupUART();

  Serial.println("Setup complete.");
}



void loop() {
  // Maintain auth & async tasks
  app.loop();

  // Only send when app is ready
  if (!app.ready()) {
    // Not authenticated / app not ready yet
    Serial.println("Waiting for Firebase app to be ready...");
    return;
  }
  // Check UART from STM32
  while (stmSerial.available() > 0) {
    int raw = stmSerial.read();
    if (raw < 0) {
      break;
    }

    uint8_t byteValue = static_cast<uint8_t>(raw);

    // Debug print what we received
    Serial.printf("Received from STM32: 0x%02X (%u)\n", byteValue, byteValue);

    // Send to Firebase
    sendByteToFirebase(byteValue);

    delay(5);
  }
}

// ---- Async callback (same style as your reference) ----
void processData(AsyncResult &aResult) {
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n",
                    aResult.uid().c_str(),
                    aResult.eventLog().message().c_str(),
                    aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n",
                    aResult.uid().c_str(),
                    aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n",
                    aResult.uid().c_str(),
                    aResult.error().message().c_str(),
                    aResult.error().code());

  if (aResult.available())
    Firebase.printf("task: %s, payload: %s\n",
                    aResult.uid().c_str(),
                    aResult.c_str());
}
