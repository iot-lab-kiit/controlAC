#include "Web-AC-control.h"
#if defined(ESP8266)
#include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#endif // ESP8266
#if defined(ESP32)
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Update.h>
#endif // ESP32
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <ESP8266mDNS.h> // For mDNS on ESP8266
#include <IRsend.h>
#include "IRutils.h"
#include "ir_LG.h"

#define AUTO_MODE 3 // kLgAcAuto
#define COOL_MODE 0 // kLgAcCool
#define DRY_MODE 1  // kLgAcDry
#define HEAT_MODE 4 // kLgAcHeat
#define FAN_MODE 2  // kLgAcFan

#define FAN_AUTO 5 // kLgAcFanAuto
#define FAN_MIN 0  // kLgAcFanLowest
#define FAN_MED 2  // kLgAcFanMedium
#define FAN_HI 4   // kLgAcFanMax

// ESP8266 GPIO pin to use for IR blaster.
const uint16_t kIrLed = 4;

// Library initialization for LG AC
IRLgAc ac(kIrLed);

/// ##### End user configuration ######

struct state
{
    uint8_t temperature = 22, fan = 0, operation = 0;
    bool powerStatus;
};

File fsUploadFile;

// core

state acState;

// settings
char deviceName[] = "iotlabac";

#if defined(ESP8266)
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdateServer;
#endif // ESP8266
#if defined(ESP32)
WebServer server(80);
#endif // ESP32

// WiFi credentials (replace with your network SSID and password)
const char *ssid = "real";
const char *password = "suman saha";

bool handleFileRead(String path)
{
    //  send the right file to the client (if it exists)
    // Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";
    // If a folder is requested, send the index file
    String contentType = getContentType(path);
    // Get the MIME type
    String pathWithGz = path + ".gz";
    if (FILESYSTEM.exists(pathWithGz) || FILESYSTEM.exists(path))
    {
        // If the file exists, either as a compressed archive, or normal
        // If there's a compressed version available
        if (FILESYSTEM.exists(pathWithGz))
            path += ".gz"; // Use the compressed verion
        File file = FILESYSTEM.open(path, "r");
        //  Open the file
        server.streamFile(file, contentType);
        //  Send it to the client
        file.close();
        // Close the file again
        // Serial.println(String("\tSent file: ") + path);
        return true;
    }
    // Serial.println(String("\tFile Not Found: ") + path);
    // If the file doesn't exist, return false
    return false;
}

String getContentType(String filename)
{
    // convert the file extension to the MIME type
    if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

void handleFileUpload()
{ // upload a new file to the FILESYSTEM
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        // Serial.print("handleFileUpload Name: "); //Serial.println(filename);
        fsUploadFile = FILESYSTEM.open(filename, "w");
        // Open the file for writing in FILESYSTEM (create if it doesn't exist)
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize);
        // Write the received bytes to the file
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {
            // If the file was successfully created
            fsUploadFile.close();
            // Close the file again
            // Serial.print("handleFileUpload Size: ");
            // Serial.println(upload.totalSize);
            server.sendHeader("Location", "/success.html");
            // Redirect the client to the success page
            server.send(303);
        }
        else
        {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    ac.begin();

    delay(1000);

    Serial.println("Connecting to WiFi...");

    WiFi.mode(WIFI_STA);        // Set ESP32 to Station mode
    WiFi.begin(ssid, password); // Connect to WiFi

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("go to file-upload");
    Serial.print(WiFi.localIP());
    Serial.print("/file-upload");
    Serial.println();
    Serial.println("Upload all the contains of the data folder");

    //    if (!MDNS.begin("iotlabac")) { // Replace "someurlname" with your desired hostname
    //   Serial.println("Error starting mDNS");
    //   return;
    // }
    // Serial.println("mDNS started. Access the device at http://iotlabac.local");

    Serial.println("mounting " FILESYSTEMSTR "...");

    if (!FILESYSTEM.begin())
    {
        // Serial.println("Failed to mount file system");
        return;
    }

#if defined(ESP8266)
    httpUpdateServer.setup(&server);
#endif // ESP8266

    server.on("/state", HTTP_PUT, []()
              {
    DynamicJsonDocument root(1024);
    DeserializationError error = deserializeJson(root, server.arg("plain"));
    if (error) {
      server.send(404, "text/plain", "FAIL. " + server.arg("plain"));
    } else {
      if (root.containsKey("temp")) {
        acState.temperature = (uint8_t) root["temp"];
      }

      if (root.containsKey("fan")) {
        acState.fan = (uint8_t) root["fan"];
      }

      if (root.containsKey("power")) {
        acState.powerStatus = root["power"];
      }

      if (root.containsKey("mode")) {
        acState.operation = root["mode"];
      }

      String output;
      serializeJson(root, output);
      server.send(200, "text/plain", output);

      delay(200);

      ac.setPower(acState.powerStatus);
      ac.setTemp(acState.temperature);

      uint8_t mode;
      switch (acState.operation) {
        case 0: mode = AUTO_MODE; break;
        case 1: mode = COOL_MODE; break;
        case 2: mode = DRY_MODE; break;
        case 3: mode = HEAT_MODE; break;
        case 4: mode = FAN_MODE; break;
        default: mode = AUTO_MODE; break;
      }
      ac.setMode(mode);

      uint8_t fanSpeed;
      switch (acState.fan) {
        case 0: fanSpeed = FAN_AUTO; break;
        case 1: fanSpeed = FAN_MIN; break;
        case 2: fanSpeed = FAN_MED; break;
        case 3: fanSpeed = FAN_HI; break;
        default: fanSpeed = FAN_AUTO; break;
      }
      ac.setFan(fanSpeed);

      ac.send();
    } });

    server.on("/file-upload", HTTP_POST, []()
              { server.send(200); }, handleFileUpload);

    server.on("/file-upload", HTTP_GET, []()
              {
    String html = "<form method=\"post\" enctype=\"multipart/form-data\">";
    html += "<input type=\"file\" name=\"name\">";
    html += "<input class=\"button\" type=\"submit\" value=\"Upload\">";
    html += "</form>";
    server.send(200, "text/html", html); });

    server.on("/", []()
              {
    server.sendHeader("Location", String("ui.html"), true);
    server.send(302, "text/plain", ""); });

    server.on("/state", HTTP_GET, []()
              {
    DynamicJsonDocument root(1024);
    root["mode"] = acState.operation;
    root["fan"] = acState.fan;
    root["temp"] = acState.temperature;
    root["power"] = acState.powerStatus;
    String output;
    serializeJson(root, output);
    server.send(200, "text/plain", output); });

    server.on("/reset", []()
              {
    server.send(200, "text/html", "reset");
    delay(100);
    ESP.restart(); });

    server.serveStatic("/", FILESYSTEM, "/", "max-age=86400");

    server.onNotFound(handleNotFound);

    server.begin();
}

void loop()
{
    // MDNS.update();
    server.handleClient();
}
