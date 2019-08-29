#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "device_config.hpp"
#include "store.hpp"
#include <EEPROM.h>

const char *softAP_ssid = MAINT_SSID;
const char *softAP_password = MAINT_PASS;

const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer webServer(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 1, 1);
IPAddress netMsk(255, 255, 255, 0);

String getContentType(String filename)
{
  if (webServer.hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path)
{
  if (path.endsWith("/"))
    path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = webServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setup_maint()
{
  pinMode(LED_EVENT, OUTPUT);

  SPIFFS.begin();

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500); // Without delay I've seen the IP address blank

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  webServer.on("/maint.html", []() {
    const char *maint = "<html><head></head><body>\
        <h1>MAINTENANCE</h1>\
        <form action=\"store\" method=\"GET\">\
        <h5>WIFI AP</h5>\
        <input name=\"sta_name\" type=\"text\" />\
        <h5>WIFI Password</h5>\
        <input name=\"sta_passwd\" type=\"text\" />\
        <h5>Slack API path</h5>\
        <input name=\"slack_path\" type=\"text\" />\
        <h5>Slack message</h5>\
        <input name=\"slack_message\" type=\"text\"/>\
        <input type=\"submit\"/>\
        </form>";
    webServer.send(200, "text/html", "1");
    webServer.sendContent(maint);
    webServer.sendContent("AAA</body></html>");
  });
  webServer.on("/load", []() {
    //Load options
    storable_options options;
    EEPROM.begin(512);
    options = EEPROM.get(OPTIONS_EEPROM_ADDR, options);
    EEPROM.end();
    char *content = (char *)malloc(1024);
    //format json
    sprintf(content,
            "{\"sta_name\":\"%s\",\"sta_passwd\":\"%s\",\"slack_path\":\"%s\",\"slack_message\":\"%s\"}",
            options.sta_name,
            options.sta_passwd,
            options.slack_path,
            options.slack_message);
    webServer.send(200, "text/plain", content);
    free(content);

  });
  webServer.on("/store", []() {
    int args = webServer.args();
    String sta_name = "";
    String sta_passwd = "";
    String slack_path = "";
    String slack_message = "";

    for (int i = 0; i < args; i++)
    {
      String argName = webServer.argName(i);
      String argValue = webServer.arg(i);
      if (argName == "sta_name")
      {
        sta_name = argValue;
      }

      if (argName == "sta_passwd")
      {
        sta_passwd = argValue;
      }

      if (argName == "slack_path")
      {
        slack_path = argValue;
      }

      if (argName == "slack_message")
      {
        slack_message = argValue;
      }
    }

    storable_options options;
    strcpy(options.sta_name, sta_name.c_str());
    strcpy(options.sta_passwd, sta_passwd.c_str());
    strcpy(options.slack_path, slack_path.c_str());
    strcpy(options.slack_message, slack_message.c_str());

    EEPROM.begin(512);
    EEPROM.put(OPTIONS_EEPROM_ADDR, options);
    EEPROM.commit();
    EEPROM.end();

    const char *maint = "<html><head></head><body><h1>SAVED!</h1></body></html>";
    webServer.send(200, "text/html", maint);
  });
  webServer.onNotFound([]() {
    //const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://192.168.1.1/maint.html\" /></head><body><p>redirecting...</p></body>";
    //webServer.send(200, "text/html", metaRefreshStr);
    if (!handleFileRead(webServer.uri()))
    {
      const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://192.168.1.1/index.html\" /></head><body><p>redirecting...</p></body>";
      webServer.send(200, "text/html", metaRefreshStr);
    }
  });

  webServer.begin();
}

void loop_maint()
{
  dnsServer.processNextRequest();
  webServer.handleClient();
  digitalWrite(LED_EVENT, LOW);
  delay(100);
  digitalWrite(LED_EVENT, HIGH);
  delay(100);
}
