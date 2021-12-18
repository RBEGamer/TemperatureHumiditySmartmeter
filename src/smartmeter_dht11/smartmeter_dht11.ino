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
#include <FS.h> //Include File System Headers
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor -> EPS8266 Pin D2
#define DHTTYPE    DHT11     // DHT 22 (AM2302)

#define DEFAULT_MQTT_BROKER "192.168.178.89"
#define DEFAULT_MQTT_TOPIC "/iot"
#define DEFAULT_MQTT_BROKER_PORT "1883"
#define MDNS_NAME "SMARTMETER" // set hostname
#define WEBSITE_TITLE "SMARTMETER Configuration" // name your device
#define VERSION "1.0"

#if defined(ESP8266)
const String BOARD_INFO= "SMARTMETER_FW_" + String(VERSION) + "_BOARD_" + "ESP8266";
#elif defined(ESP32)
const String BOARD_INFO= "SMARTMETER_FW_" + String(VERSION) + "_BOARD_" + "ESP32";
#endif


uint32_t delayMS;
const char* MQTT_BROKER = "192.168.178.89";
long lastMsg = 0;
char msg[50];
int value = 0;
String mqtt_broker_url = "";
String mqtt_topic = "";
String mqtt_broker_port = "";
String last_error = "";


const char* file_mqtt_server = "/mqttbroker.txt";
const char* file_mqtt_topic = "/mqtttopic.txt";
const char* file_mqtt_broker_port = "/mqttbrokerport.txt";

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);
DHT_Unified dht(DHTPIN, DHTTYPE);


const String phead_1 = "<!DOCTYPE html><html><head><title>";
const String phead_2 = "</title>"
                       "<meta http-equiv='content-type' content='text/html; charset=utf-8'>"
                       "<meta charset='utf-8'>"
                       "<link "
                       "href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/themes/base/"
                       "jquery-ui.css' rel=stylesheet />"
                       "<script "
                       "src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js'></"
                       "script>"
                       "<script "
                       "src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/"
                       "jquery-ui.min.js'></script>"
                       "<style>"
                       "html, body {"
                       "  background: #F2F2F2;"
                       " width: 100%;"
                       " height: 100%;"
                       " margin: 0px;"
                       " padding: 0px;"
                       " font-family: 'Verdana';"
                       " font-size: 16px;"
                       " color: #404040;"
                       " }"
                       "img {"
                       " border: 0px;"
                       "}"
                       "span.title {"
                       " display: block;"
                       " color: #000000;"
                       " font-size: 30px;"
                       "}"
                       "span.subtitle {"
                       " display: block;"
                       " color: #000000;"
                       " font-size: 20px;"
                       "}"
                       ".sidebar {"
                       " background: #FFFFFF;"
                       " width: 250px;"
                       " min-height: 100%;"
                       " height: 100%;"
                       " height: auto;"
                       " position: fixed;"
                       " top: 0px;"
                       " left: 0px;"
                       " border-right: 1px solid #D8D8D8;"
                       "}"
                       ".logo {"
                       " padding: 25px;"
                       " text-align: center;"
                       " border-bottom: 1px solid #D8D8D8;"
                       "}"
                       ".menu {"
                       " padding: 25px 0px 25px 0px;"
                       " border-bottom: 1px solid #D8D8D8;"
                       "}"
                       ".menu a {"
                       " padding: 15px 25px 15px 25px;"
                       " display: block;"
                       " color: #000000;"
                       " text-decoration: none;"
                       " transition: all 0.25s;"
                       "}"
                       ".menu a:hover {"
                       " background: #0088CC;"
                       " color: #FFFFFF;"
                       "}"
                       ".right {"
                       " margin-left: 250px;"
                       " padding: 50px;"
                       "}"
                       ".content {"
                       " background: #FFFFFF;"
                       " padding: 25px;"
                       " border-radius: 5px;"
                       " border: 1px solid #D8D8D8;"
                       "}"
                       "</style>";

const String pstart = "</head>"
                      "<body style='font-size:62.5%;'>"
                      "<div class='sidebar'>"
                      "<div class='logo'>"
                      "<span class='title'>SmartMeter</span>"
                      "<span class='subtitle'>- Configuration -</span>"
                      "</div>"
                      "<div class='menu'>"
                      "<a href='index.html'>Settings</a>"
                      "</div>"
                      "</div>"
                      "<div class='right'>"
                      "<div class='content'>";

const String pend = "</div>"
                    "</div>"
                    "</body>"
                    "</html>";
                    


// ONLY READ THE FIRST LINE UNTIL NEW LINE !!!!!
String read_file(const char* _file, String _default = "")
{
    File f = SPIFFS.open(_file, "r");
    String tmp = _default;
    if (!f) {
        last_error = "open filesystem file_ntp_server failed";
    }
    else {
        tmp = f.readStringUntil('\n');
        last_error = "read from FS:" + String(_file) + " " + tmp;
    }
    return tmp;
}

void restore_eeprom_values()
{ 
    mqtt_broker_url = read_file(file_mqtt_server,DEFAULT_MQTT_BROKER);
    mqtt_topic = read_file(file_mqtt_topic,DEFAULT_MQTT_TOPIC);
    mqtt_broker_port = read_file(file_mqtt_broker_port, String(DEFAULT_MQTT_BROKER_PORT));
}

bool write_file(const char* _file, String _content)
{
    File f = SPIFFS.open(_file, "w");
    if (!f) {
        last_error = "Oeffnen der Datei fehlgeschlagen";
        return -1;
    }
    f.print(_content);
    f.close();
    return 0;
}

void save_values_to_eeprom(){
    write_file(file_mqtt_server, mqtt_broker_url);
    write_file(file_mqtt_topic, mqtt_topic);
    write_file(file_mqtt_broker_port, mqtt_broker_port);
}


void write_deffault_to_eeprom(){        
  mqtt_broker_url = DEFAULT_MQTT_BROKER;
  mqtt_broker_port = DEFAULT_MQTT_BROKER_PORT;
  mqtt_topic = DEFAULT_MQTT_TOPIC;
  save_values_to_eeprom();
}


void setup() {
    Serial.begin(9600);


    if (SPIFFS.begin()) {
        Serial.println("SPIFFS Initialisierung....OK");
    }
    else {
        Serial.println("SPIFFS Initialisierung...Fehler!");
    }
    // LOAD SETTINGS
    restore_eeprom_values();


    //WEBSERVER ROUTES
    delay(1000);
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.on("/index.html", handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();

    //REGISTER MDNS
    if (MDNS.begin((MDNS_NAME + String(ESP.getChipId())).c_str())) {
    }

    
    //START OTA LIB
    ArduinoOTA.setHostname((MDNS_NAME + String(ESP.getChipId())).c_str());
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        }
        else { // U_SPIFFS
            type = "filesystem";
        }
        SPIFFS.end();
    });
    ArduinoOTA.onEnd([]() {});
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {});
    ArduinoOTA.onError([](ota_error_t error) {});
    ArduinoOTA.onEnd([]() {
        if (SPIFFS.begin()) {
            restore_eeprom_values(); // RESTORE FILE SETTINGS
        }
    });
    ArduinoOTA.begin();

    // INIT DHT SENSOR
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    dht.humidity().getSensor(&sensor);



    //SETUP MQTT
    setup_mqtt_client();
    Serial.println("_setup_complete_");
    
}


String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}


void mqtt_reconnect() {
  // Loop until we're reconnected
 if(client.connected()) {return;}
    // Attempt to connect
    if (client.connect((MDNS_NAME + String(ESP.getChipId())).c_str())) {
      last_error = "MQTT CLICNET CONNCTED";
    } else {
      last_error = "MQTT CLCIENT CONECT FAILED WITH" + String(client.state());
  }
}

void setup_mqtt_client(){
   if(mqtt_broker_url != "" && mqtt_broker_port != ""){
    client.setServer(mqtt_broker_url.c_str(), mqtt_broker_port.toInt());
    if(client.connected()){
     
    }
   }    
}



void handleSave()
{
    // PARSE ALL GET ARGUMENTS
    for (uint8_t i = 0; i < server.args(); i++) {
        // mqtt_broker_url
        if (server.argName(i) == "mqtt_broker_url") {
            mqtt_broker_url = server.arg(i);
            last_error = "set mqtt_broker_url to" + mqtt_broker_url;

        }
        // mqtt_broker_port
        if (server.argName(i) == "mqtt_broker_port") {
            mqtt_broker_port = server.arg(i);
            last_error = "set mqtt_broker_port to" + mqtt_broker_port;
        }
        // mqtt_broker_port
        if (server.argName(i) == "mqtt_topic") {
            mqtt_topic = server.arg(i);
            last_error = "set mqtt_topic to" + mqtt_topic;

        }  
    }
    //SAVE THESE DATA
    save_values_to_eeprom();
    //SAVE MQTT STUFF 
    setup_mqtt_client();
    
    server.send(300, "text/html","<html><head><meta http-equiv='refresh' content='1; url=/' /></head><body>SAVE SETTINGS PLEASE WAIT</body></html>");
}


void handleRoot()
{

    String control_forms = "<hr><h2>DEVICE INFO</h2>";
    control_forms+="<h3>" + String(MDNS_NAME) + String(ESP.getChipId()) + "<br><br>"+BOARD_INFO+"</h3><br>";


     control_forms += 
                     "<br><h3> MQTT SETTINGS </h3>"
                     "<form name='btn_offmq' action='/save' method='GET'>"
                     "<input type='text' value='"+ String(mqtt_broker_url) + "' name='mqtt_broker_url' required placeholder='broker.hivemq.com'/>"
                     "<input type='submit' value='SAVE MQTT BROKER'/>"
                     "</form>"
                     "<form name='btn_off' action='/save' method='GET'>"
                     "<input type='number' value='"+ String(mqtt_broker_port) + "' name='mqtt_broker_port' min='1' max='65536' required placeholder='1883'/>"
                     "<input type='submit' value='SET MQTT BROKER PORT'/>"
                     "</form>"
                     "<form name='btn_off' action='/save' method='GET'>"
                     "<input type='text' value='"+ String(mqtt_topic) + "' name='mqtt_topic' required placeholder='/iot'/>"
                     "<input type='submit' value='SET MQTT BASE TOPIC'/>"
                     "</form>"
                     "<br><hr><h3>LAST SYSTEM MESSAGE</h3><br>" + last_error;


                         


    server.send(200, "text/html", phead_1 + WEBSITE_TITLE + phead_2 + pstart + control_forms + pend);
}


void handleNotFound()
{
    server.send(404, "text/html","<html><head>header('location: /'); </head></html>");
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


void publish_values(){
  float Temperatur=0;
    float Luftfeuchtigkeit=0;
    char temp[100];
    char hum[100];
    char bar[100]; 



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
      client.publish((mqtt_topic + "/"+String(ESP.getChipId())+"/temperature/").c_str(), temp);
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
      client.publish((mqtt_topic + "/"+String(ESP.getChipId())+"/humidity/").c_str(), hum);
    }
  }


unsigned long timeNow = 0;
unsigned long timeLast = 0;

void loop() {

    //HANDLE SERVER
    server.handleClient();

    
     
	
		if (!client.connected()) {
			mqtt_reconnect();
		}
    client.loop();

    //HANDLE OTA
    ArduinoOTA.handle();



    if ((millis() - timeLast) > 1000) {
        timeLast = millis();   
        publish_values();
    }

      
	
    
   delay(70);
  
}
