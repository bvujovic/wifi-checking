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
#define REAL_SEND_MSG false

// Milliseconds in 1 second.
#define SEC (1000)
// Milliseconds in 1 minute.
#define MIN (60 * SEC)

// Deep sleep time in microseconds.
#define SLEEP_INTERVAL 1000 * (DEBUG ? 15 * SEC : 5 * MIN)

enum CheckResult
{
  NotChecked,
  Good,
  Bad
};

struct Net
{
  bool checkWiFi;
  String ssid;
  String pass;
  CheckResult result;
};

LinkedList<Net *> *nets = new LinkedList<Net *>();
Net *goodNet = NULL;
String badNets = "";
String goodNets = "";
String skippedNets = "";

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
int sendWhatsAppMessage()
{
  writeln("sendWhatsAppMessage");
#if REAL_SEND_MSG
  //* https://www.callmebot.com/blog/free-api-whatsapp-messages/
  String url = "http://api.callmebot.com/whatsapp.php?";
  url = url + "phone=" + CMB_PHONE;
  // https://www.callmebot.com/blog/test-whatsapp-api/ 📶 WiFi mreže: Sveti Sava, Tech... NE rade.
  url = url + "&text=" + "📶+Školske+WiFi+mreže:%0A";
  if (badNets != "")
    url += "NE+rade:+" + badNets;
  if (badNets != "" && goodNets != "")
    url += "%0A"; // separator (new line) between list of bad and good nets
  if (goodNets != "")
    url += "Rade:+" + goodNets;
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

/// @brief Reads data about wifi networks from data/nets.csv.
/// @return List of networks that should be checked.
void readNetworkData()
{
  writeln("readNetworkData");
  File f = LittleFS.open("nets.csv", "r");
  if (f)
  {
    while (f.available())
    {
      String line = f.readStringUntil('\n');
      writeln(line);
      if (line.length() == 0)
        continue;
      // e.g. "1+WiFiName+WiFiPass+2"
      // if (line[0] == '1') // 1 - check the wifi, 0 - skip
      {
        int idx1 = line.indexOf('+'); // '+' between check? and ssid
        if (idx1 == -1)
          continue;
        int idx2 = line.indexOf('+', idx1 + 1); // '+' between ssid and pass
        if (idx2 == -1 || idx1 >= idx2)
          continue;
        int idx3 = line.indexOf('+', idx2 + 1); // '+' between pass and result
        if (idx3 == -1 || idx2 >= idx3)
          continue;
        // string "0" or "1" or "2" -> character '0' or '1' or '2' -> number (int) 0 or 1 or 2
        int res = line.substring(idx3 + 1, line.length())[0] - '0';
        Net *net = new Net{
            (bool)(line[0] - '0'),
            line.substring(idx1 + 1, idx2),
            line.substring(idx2 + 1, idx3),
            (CheckResult)(res >= NotChecked && res <= Bad ? res : NotChecked)};
        // writeln(net->ssid);
        // writeln(net->pass);
        // writeln(net->result);
        nets->add(net);
      }
      // else skippedNets += line + '\n';
    }
    f.close();
  }
}

void saveNetworkData()
{
  writeln("saveNetworkData");
  File f = LittleFS.open("nets.csv", "w");
  if (f)
  {
    // // checked nets
    for (int i = 0; i < nets->size(); i++)
    {
      Net *net = nets->get(i);
      // e.g. "1+WiFiName+WiFiPass+2"
      String line = String(net->checkWiFi) + "+" + net->ssid + "+" + net->pass + "+" + net->result + "\n";
      f.write(line.c_str());
    }
    // // skipped nets
    // f.write(skippedNets.c_str());
    f.close();
  }
}

void checkNets()
{
  writeln("Checking WiFi networks...");
  for (int i = 0; i < nets->size(); i++)
  {
    Net *net = nets->get(i);
    if (!net->checkWiFi)
      continue;
    writeln(net->ssid);
    bool success = connectToWiFi(net->ssid, net->pass);
    writeln(success);
    if (success)
    {
      if (goodNet == NULL)
        goodNet = net;
      if (net->result == Bad) // wifi was bad, now it's good
        goodNets += (goodNets != "" ? ", " : "") + net->ssid;
    }
    else if (net->result != Bad) // wifi was bad or not checked
      badNets += (badNets != "" ? ", " : "") + net->ssid;
    net->result = success ? Good : Bad;
    WiFi.disconnect();
  }
  writeln("Bad nets: " + badNets);
  writeln("Good nets: " + goodNets);
}

void setup()
{
  serialDebugBegin();
  writeln("\n***START***");
  pinMode(led.getPin(), OUTPUT);
  LittleFS.begin();

  readNetworkData();
  checkNets();

  // if there are bad networks to report on and at least one good network to be able to send message
  if ((badNets != "" || goodNets != "") && goodNet != NULL)
  {
    writeln("Sending WA msg...");
    bool success = REAL_SEND_MSG ? connectToWiFi(goodNet->ssid, goodNet->pass) : true;
    if (success)
    {
      badNets.replace(' ', '+');
      goodNets.replace(' ', '+');
      sendWhatsAppMessage();
    }
    else
      ESP.reset();
  }
  saveNetworkData();

  writeln("Sleep...");
  ESP.deepSleep(SLEEP_INTERVAL);
}

void loop()
{
}
