#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <iostream>
#include <string>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 4     // Digital pin connected to the DHT sensor -> EPS8266 Pin D2
#define DHTTYPE    DHT11     // DHT 22 (AM2302)




uint32_t delayMS;
const char* MQTT_BROKER = "192.168.178.89";
long lastMsg = 0;
char msg[50];
int value = 0;



WiFiClient espClient;
PubSubClient client(espClient);
DHT_Unified dht(DHTPIN, DHTTYPE);




void setup() {
    Serial.begin(9600);
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    dht.humidity().getSensor(&sensor);
    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);
    
}


String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}


void setup_wifi() {
    delay(10);
    Serial.println();

 
    // START WFIFIMANAGER FOR CAPTIVE PORTAL
    WiFiManager wifiManager;
    wifiManager.setDebugOutput(false);
    wifiManager.setTimeout(120);
    //TRY TO CONNECT
    // AND DISPLAY IP ON CLOCKS HOUR DISPLAY (FOR 2 DIGIT CLOCKS)
    if(wifiManager.autoConnect("SmartmeterConfiguration")){
      String ip = IpAddress2String(WiFi.localIP());
      Serial.println(ip);
      
    }else{
      Serial.println("WIFI CONNECTION ERROR");
    }
    
 
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
 
void reconnect() {
    while (!client.connected()) {
        Serial.print("Reconnecting...");
        if (!client.connect(("smartmeter_" + String(ESP.getChipId())).c_str())) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}
void loop() {
    float Temperatur=0;
    float Luftfeuchtigkeit=0;
    char temp[100];
    char hum[100];
    char bar[100];   
	
		if (!client.connected()) {
			reconnect();
		}
    client.loop();
	sensors_event_t event;
  
    Serial.print("Publish message: ");
    Serial.println(msg);
    dht.temperature().getEvent(&event);
	
		if (isnan(event.temperature)) {
			Serial.println(F("Error reading temperature!"));
		}
		else {
			Serial.print(F("Temperature: "));
			Temperatur=event.temperature;
			Serial.print(Temperatur);
			Serial.println(F("°C"));
			snprintf (temp, 50, "%f", Temperatur);
			client.publish(("/iot/"+String(ESP.getChipId())+"/temperature/").c_str(), temp);
		}
  
	  dht.humidity().getEvent(&event);
	
		if (isnan(event.relative_humidity)) {
			Serial.println(F("Error reading humidity!"));
		}
		else {
		  Serial.print(F("Humidity: "));
		  Luftfeuchtigkeit=event.relative_humidity;
		  Serial.print(Luftfeuchtigkeit);
		  Serial.println(F("%"));
		  snprintf (hum, 50, "%f", Luftfeuchtigkeit);
		  client.publish(("/iot/"+String(ESP.getChipId())+"/humidity/").c_str(), hum);
		}
    
   delay(5000);
  
}
