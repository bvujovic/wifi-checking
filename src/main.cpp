#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <CredCallMeBot.h>

#include "Blinky.h" // https://github.com/bvujovic/ArduinoLibs/tree/main/Blinky
Blinky led = Blinky::create();

// To print or not to print... debug messages.
#define DEBUG true
#if DEBUG
#define writeln(x) Serial.println(x)
#define serialDebugBegin() Serial.begin(115200)
#else
#define writeln(x)
#define serialDebugBegin()
#endif

// Sending an actual WhatsApp message or not (testing purposes).
#define REAL_SEND_MSG false

// Milliseconds in 1 second.
#define SEC (1000)
// Milliseconds in 1 minute.
#define MIN (60 * SEC)

bool connectToWiFi(const char *ssid, const char *pass)
{
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, pass);
  int i = 0;
  while (!WiFi.isConnected() && i++ < 10)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return WiFi.isConnected();
}

int sendWhatsAppMessage()
{
  writeln("sendWhatsAppMessage");
#if REAL_SEND_MSG
  //* https://www.callmebot.com/blog/free-api-whatsapp-messages/
  String url = "http://api.callmebot.com/whatsapp.php?";
  url = url + "phone=" + CMB_PHONE;
  // 💥Stan, kuhinja, sudopera:
  // VISOK NIVO VODE U SUDOPERI 💦
  url = url + "&text=" + "%F0%9F%92%A5+Stan,+kuhinja,+sudopera:%0AVISOK+NIVO+VODE+U+SUDOPERI+%F0%9F%92%A6";
  url = url + "&apikey=" + CMB_API_KEY;
  writeln(url);
  WiFiClient wiFiClient;
  HTTPClient client;
  client.begin(wiFiClient, url);
  int respCode = client.GET();
  writeln("Resp code: ");
  writeln(respCode);
  if (respCode > 0)
    writeln(client.getString());
  client.end();
  return respCode;
#else // fake send message
  led.blinkOk();
  // return HTTP_CODE_BAD_REQUEST;
  return HTTP_CODE_OK;
#endif
}

void setup()
{
  serialDebugBegin();
  pinMode(led.getPin(), OUTPUT);

  // prolazak kroz listu wifi mreza koje treba proveriti
  // proveravanje svake od njih
  // obavestavanje o problemu, ako postoji
  // spavanje x minuta
}

void loop()
{
}
