#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>

const char *ssid = "";
const char *password = "";

#define botToken ""
#define chatID ""

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

void setup() {
  Serial.begin(115200);
  client.setInsecure();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP Address");
  Serial.println(WiFi.localIP());
  bot.sendMessage(chatID, "Code Started", "");
  Serial.println("Bot connected");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("New message received");
      String chat_id = bot.messages[0].chat_id;
      String text = bot.messages[0].text;

      bot.sendMessage(chat_id, "Received your message: " + text);

      bot.getUpdates(bot.last_message_received + 1);
      numNewMessages--;
    }

    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      if (bot.sendMessage(chatID, input)) {
        Serial.println("Message sent to Telegram");
      } else {
        Serial.println("Failed to send message to Telegram");
      }
    }
  } else {
    Serial.println("WiFi Disconnected");
  }
  delay(1000);
}