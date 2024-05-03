#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <CredCallMeBot.h> // CMB_PHONE and CMB_API_KEY defined
#include "LinkedList.h"
#include <LittleFS.h>

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
#define REAL_SEND_MSG true

// Milliseconds in 1 second.
#define SEC (1000)
// Milliseconds in 1 minute.
#define MIN (60 * SEC)

// Deep sleep time in microseconds.
#define SLEEP_INTERVAL 1000 * (DEBUG ? 15 * SEC : 5 * MIN)

bool connectToWiFi(String ssid, String pass)
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

/// @brief Sends WhatsApp message about failed WiFi networks.
/// @param ssids Names of failed WiFi networks.
/// @return HTTP response code for sent message.
int sendWhatsAppMessage(String ssids)
{
  writeln("sendWhatsAppMessage");
#if REAL_SEND_MSG
  //* https://www.callmebot.com/blog/free-api-whatsapp-messages/
  String url = "http://api.callmebot.com/whatsapp.php?";
  url = url + "phone=" + CMB_PHONE;
  // https://www.callmebot.com/blog/test-whatsapp-api/ 📶 WiFi mreže: Sveti Sava, Tech... ne rade.
  url = url + "&text=" + "📶%20WiFi%20mreže:%20" + ssids + "%20ne%20rade.";
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

enum CheckResult
{
  NotChecked,
  Good,
  Bad
};

struct Net
{
  String ssid;
  String pass;
  CheckResult result;
};

/// @brief Reads data about wifi networks from data/nets.csv.
/// @return List of networks that should be checked.
LinkedList<Net *> *readNetworkData()
{
  LinkedList<Net *> *nets = new LinkedList<Net *>();
  File f = LittleFS.open("nets.csv", "r");
  if (f)
  {
    while (f.available())
    {
      String line = f.readStringUntil('\n');
      if (line.length() == 0)
        continue;
      // e.g. "1+WiFiName+WiFiPass"
      if (line[0] == '1')
      {
        int idx1 = line.indexOf('+');
        if (idx1 == -1)
          continue;
        int idx2 = line.indexOf('+', idx1 + 1);
        if (idx2 == -1 || idx1 >= idx2)
          continue;
        Net *net = new Net{line.substring(idx1 + 1, idx2), line.substring(idx2 + 1, line.length() - 1), NotChecked};
        nets->add(net);
      }
    }
    f.close();
  }
  return nets;
}

void setup()
{
  serialDebugBegin();
  writeln("\n***START***");
  pinMode(led.getPin(), OUTPUT);
  LittleFS.begin();

  writeln("Checking WiFi networks...");
  LinkedList<Net *> *nets = readNetworkData();
  Net *goodNet = NULL;
  String badNets = "";
  for (int i = 0; i < nets->size(); i++)
  {
    Net *net = nets->get(i);
    writeln(net->ssid);
    bool success = connectToWiFi(net->ssid, net->pass);
    writeln(success);
    net->result = success ? Good : Bad;
    if (success)
    {
      if (goodNet == NULL)
        goodNet = net;
    }
    else
      badNets += (badNets != "" ? ", " : "") + net->ssid;
  }
  writeln(badNets);

  // if there are bad networks to report on and at least one good network to be able to send message
  if (badNets != "" && goodNet != NULL)
  {
    writeln("Sending WA msg...");
    bool success = connectToWiFi(goodNet->ssid, goodNet->pass);
    if (success)
      sendWhatsAppMessage(badNets);
    else
      ESP.reset();
  }
  
  writeln("Sleep...");
  ESP.deepSleep(SLEEP_INTERVAL);
}

void loop()
{
}
