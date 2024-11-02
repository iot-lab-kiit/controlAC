
#include <ESP8266WiFi.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "ir_LG.h"

const char *ssid = "!!"; //
const char *password = "findyourself";

int irpin = 4; // D2 in our configuration

IRsend irsend(irpin);
WiFiServer server(80);

typedef struct ACSettings
{
  uint8_t power;
  uint8_t temperature;
  uint8_t fanSpeed;
  uint8_t mode;
} ACSettings;

ACSettings acSettings;

void setup()
{
  Serial.begin(115200);
  irsend.begin();

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  loadSettings();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  server.begin();
}

void loop()
{
  WiFiClient client = server.available();
  if (client)
  {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/power") != -1)
    {
      acSettings.power = !acSettings.power;
      irsend.sendLG(acSettings.power ? kLgAcPowerOn : kLgAcPowerOff); // Power code
    }
    else if (request.indexOf("/tempup") != -1)
    {
      if (acSettings.temperature < kLgAcMaxTemp)
      {
        acSettings.temperature++;
        irsend.sendLG(0x20DF40BF); //  Temp Up code
      }
    }
    else if (request.indexOf("/tempdown") != -1)
    {
      if (acSettings.temperature > kLgAcMinTemp)
      {
        acSettings.temperature--;
        irsend.sendLG(0x20DF20DF); //  Temp Down code
      }
    }
    else if (request.indexOf("/fan") != -1)
    {
      acSettings.fanSpeed = (acSettings.fanSpeed + 1) % kLgAcFanEntries; //  fan speeds
      irsend.sendLG(convertFan(acSettings.fanSpeed));                    //  Fan Speed code
    }
    else if (request.indexOf("/mode") != -1)
    {
      acSettings.mode = (acSettings.mode + 1) % 5; // modes (0-4)
      irsend.sendLG(convertMode(acSettings.mode)); //  Mode code
    }
    else if (request.indexOf("/style.css") != -1)
    {
      serveFile("/style.css", "text/css");
      return;
    }
    else if (request.indexOf("/script.js") != -1)
    {
      serveFile("/script.js", "application/javascript");
      return;
    }
    else if (request.indexOf("/") != -1)
    {
      serveFile("/index.html", "text/html");
      return;
    }

    saveSettings();
  }
}

void serveFile(const char *filename, const char *contentType)
{
  File file = SPIFFS.open(filename, "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  WiFiClient client = server.available();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: " + String(contentType));
  client.println("Connection: close");
  client.println();

  while (file.available())
  {
    client.write(file.read());
  }
  file.close();
}

void saveSettings()
{
  File file = SPIFFS.open("/settings.json", "w");
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }

  StaticJsonDocument<200> doc;
  doc["power"] = acSettings.power;
  doc["temperature"] = acSettings.temperature;
  doc["fanSpeed"] = acSettings.fanSpeed;
  doc["mode"] = acSettings.mode;

  if (serializeJson(doc, file) == 0)
  {
    Serial.println("Failed to write to file");
  }
  file.close();
}

void loadSettings()
{
  File file = SPIFFS.open("/settings.json", "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.print("Failed to read file, using default configuration: ");
    Serial.println(error.f_str());
    return;
  }
  file.close();

  acSettings.power = doc["power"];
  acSettings.temperature = doc["temperature"];
  acSettings.fanSpeed = doc["fanSpeed"];
  acSettings.mode = doc["mode"];
}

uint32_t convertFan(uint8_t speed)
{
  switch (speed)
  {
  case kLgAcFanLowest:
    return 0x20DF00FF; // Replace
  case kLgAcFanLow:
    return 0x20DF01FE; // Replace
  case kLgAcFanMedium:
    return 0x20DF02FD; // Replace
  case kLgAcFanMax:
    return 0x20DF03FC; // Replace
  case kLgAcFanAuto:
    return 0x20DF04FB; // Replace
  default:
    return 0x20DF00FF; //  lowest
  }
}

uint32_t convertMode(uint8_t mode)
{
  switch (mode)
  {
  case kLgAcCool:
    return 0x20DF05FA; // Replace
  case kLgAcDry:
    return 0x20DF06F9; // Replace
  case kLgAcFan:
    return 0x20DF07F8; // Replace
  case kLgAcAuto:
    return 0x20DF08F7; // Replace
  case kLgAcHeat:
    return 0x20DF09F6; // Replace
  default:
    return 0x20DF05FA; // code for cool mode
  }
}