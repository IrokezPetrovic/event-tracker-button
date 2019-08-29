#include <WiFiClientSecure.h>
#include "device_config.hpp"
namespace Messenger
{
const char *host = "hooks.slack.com";
const int port = 443;
const char *path = "/services/T0F40E7QC/BMDF2297D/qomsfyCQTtIy3IgnRNpUrBDu";



void SendMessage(String slack_path, String message)
{
    WiFiClientSecure httpsClient;
    httpsClient.setInsecure();

    int RETRIES = 10;
    digitalWrite(LED_EVENT, LOW);
    char lengthString[8];
    int length = message.length()+11;
    itoa(length,lengthString, 10);
    

    for (int i = 0; i < RETRIES; i++)
    {
        
        if (httpsClient.connect(host, port))
        {
            httpsClient.print("POST ");
            httpsClient.print(slack_path);
            httpsClient.print(" HTTP/1.1\r\n");
            httpsClient.print("Host: ");
            httpsClient.print(host);
            httpsClient.print("\r\n");
            httpsClient.print("Connection: close\r\n");
            httpsClient.print("Content-Type: application/json\r\n");            
            httpsClient.print("Content-Length: ");
            httpsClient.print(String(lengthString));
            httpsClient.print("\r\n");
            httpsClient.print("\r\n");
            //httpsClient.print("{\"text\":\"FOO_BAR_BAZ\"}");
            httpsClient.print("{\"text\":\"");
            httpsClient.print(message);
            httpsClient.print("\"}");

            httpsClient.stop();
            return;
        }
        digitalWrite(LED_EVENT, HIGH);
        delay(10);
        digitalWrite(LED_EVENT, LOW);
        delay(10);
    }
    digitalWrite(LED_EVENT, HIGH);
}
} // namespace Messenger
