
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include "WiFi.h"
#include <iostream>
#include <string>
#include "IkeaVindriktningSerialCom.h" 
#include "IkeaVindriktningTypes.h"
particleSensorState_t state;
uint32_t delayMS;
const char* SSID = "ProDevMo";
const char* PSK = "6226054527192856";
const char* MQTT_BROKER = "192.168.178.89";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
    Serial.begin(9600);
    
    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);

    IkeaVindriktningSerialCom::setup();
    for(int i = 0; i < 10000; i++){
        IkeaVindriktningSerialCom::handleUart(state);
        delay(1);
      }
    
}
 
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
 
    WiFi.begin(SSID, PSK);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
 
void reconnect() {
    while (!client.connected()) {
        Serial.print("Reconnecting...");
        if (!client.connect("ESP8266Client")) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}
void loop() {
    
    char pm25[100];
	
		if (!client.connected()) {
			reconnect();
		}
    client.loop();
	
  
    
	  IkeaVindriktningSerialCom::handleUart(state);
    float PM25=state.lastPM25 * 1.00;
			Serial.print(F("PM2.5: "));

			Serial.print(PM25);
			Serial.println(F(""));

      snprintf (pm25, 50, "%f", PM25);
      client.publish("/iot/smartmeter/pm25/", pm25);
      Serial.print("Publish message: ");
    Serial.println(msg);
   
        
			
		

	
    
   delay(5000);
  
}
