#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "messenger.hpp"
#include "device_config.hpp"
#include <EEPROM.h>
#include "store.hpp"


storable_options options;
volatile int events = 0;
volatile long lastEvent = 0;
#define EVENT_DELAY 1000
void ICACHE_RAM_ATTR OnEventButton()
{
  long now = millis();
  if (now - lastEvent > EVENT_DELAY)
  {
    events++;
    lastEvent = now;
  }
  
}

void setup_prod()
{
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_EVENT, OUTPUT);
  digitalWrite(LED_EVENT, HIGH);
  digitalWrite(LED_WIFI, HIGH);

  interrupts();
  pinMode(BTN_EVENT, INPUT_PULLUP);
  attachInterrupt(BTN_EVENT, OnEventButton, FALLING);
  delay(100);
  EEPROM.begin(512);
  options = EEPROM.get(OPTIONS_EEPROM_ADDR, options);  
  EEPROM.end();
  

  digitalWrite(LED_WIFI, HIGH);
  WiFi.begin(String(options.sta_name), String(options.sta_passwd));

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(50);
    digitalWrite(LED_WIFI, LOW);
    delay(50);
    digitalWrite(LED_WIFI, HIGH);
  }

  digitalWrite(LED_WIFI, LOW);
  
}

String host("maker.ifttt.com");
int port = 443;

void loop_prod()
{
  while (events > 0)
  {
    digitalWrite(LED_EVENT, LOW);
    Messenger::SendMessage(String(options.slack_path), String(options.slack_message));
    digitalWrite(LED_EVENT, HIGH);    
    events--;
    delay(300);
  }
}
