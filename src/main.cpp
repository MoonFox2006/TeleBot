#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "private.h"

constexpr uint8_t LED_PIN = 0;
constexpr bool LED_LEVEL = LOW;

RTC_DATA_ATTR uint32_t boots = 0;

static bool telegramSendMessage(const char *bot_key, int32_t chat_id, const char *message) {
  WiFiClientSecure client;
  HTTPClient https;
  bool result = false;

  client.setInsecure();
  if (https.begin(client, String("https://api.telegram.org/bot") + bot_key + "/")) {
    String post;

    https.addHeader("Content-Type", "application/json");
    https.addHeader("Connection", "close");
    post = "{\"method\":\"sendMessage\",\"chat_id\":" + String(chat_id) + ",\"text\":\"" + message + "\"}";
    Serial.println(post);
    result = https.POST(post) == 200;
    Serial.println(https.getString());
    https.end();
  }
  return result;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ! LED_LEVEL);

  ++boots;
  Serial.printf("Board boots %u time(s)\r\n", boots);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  Serial.printf("Connecting to \"%s\"", WIFI_SSID);
  while (! WiFi.isConnected()) {
    digitalWrite(LED_PIN, LED_LEVEL);
    delay(25);
    digitalWrite(LED_PIN, ! LED_LEVEL);
    Serial.print('.');
    delay(250 - 25);
  }
  Serial.print(" OK (IP: ");
  Serial.print(WiFi.localIP());
  Serial.println(')');

  {
    char msg[48];

    snprintf(msg, sizeof(msg), "Board \\\"ESP32\\\"\nBoots %u time(s)", boots);
    if (telegramSendMessage(BOT_KEY, CHAT_ID, msg))
      Serial.println("BOT OK");
    else
      Serial.println("BOT fail!");
  }

  Serial.flush();
  esp_sleep_enable_timer_wakeup(30000000); // 30 sec.
  esp_deep_sleep_disable_rom_logging();
  esp_deep_sleep_start();
}

void loop() {}
