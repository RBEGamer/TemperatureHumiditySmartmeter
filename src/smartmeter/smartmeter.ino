#include <FS.h> //Include File System Headers


#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif


#ifdef ESP32
#include <WebServer.h>
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true
#include <ESPmDNS.h>
#endif

#include <PubSubClient.h>
#include <WiFiClient.h>

#include <WiFiManager.h>
#include <Wire.h>
#include <iostream>
#include <string>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <ArduinoOTA.h>






#include "IkeaVindriktningSerialCom.h"
#include "IkeaVindriktningTypes.h"
particleSensorState_t state;





#define WEBSERVER_PORT 80

//ESP8266 => D2
//ESP32 => D4
#define DHTPIN 4




#define DHTTYPE    DHT22     // DHT 22 (AM2302)

#define DEFAULT_MQTT_BROKER "192.168.178.89"
#define DEFAULT_MQTT_TOPIC "/iot"
#define DEFAULT_MQTT_BROKER_PORT "1883"
#define DEFAULT_ENABLE_PM25 "1"
#define DEFAULT_ENABLE_DHT "1"
#define MDNS_NAME "SMARTMETER" // set hostname
#define WEBSITE_TITLE "SMARTMETER Configuration" // name your device
#define VERSION "1.0"

#if defined(ESP8266)
const String BOARD_INFO = "SMARTMETER_FW_" + String(VERSION) + "_BOARD_" + "ESP8266";
#elif defined(ESP32)
const String BOARD_INFO = "SMARTMETER_FW_" + String(VERSION) + "_BOARD_" + "ESP32";
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
String enable_pm25 = "";
String enable_dht = "";
const char* file_mqtt_server = "/mqttbroker.txt";
const char* file_mqtt_topic = "/mqtttopic.txt";
const char* file_mqtt_broker_port = "/mqttbrokerport.txt";
const char* file_enable_pm25 = "/enablepm25.txt";
const char* file_enable_dht = "/enable_dht.txt";

WiFiClient espClient;
PubSubClient client(espClient);

#ifdef ESP8266
ESP8266WebServer server(WEBSERVER_PORT);
#endif

#ifdef ESP32
WebServer server(WEBSERVER_PORT);
#endif
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
  mqtt_broker_url = read_file(file_mqtt_server, DEFAULT_MQTT_BROKER);
  mqtt_topic = read_file(file_mqtt_topic, DEFAULT_MQTT_TOPIC);
  mqtt_broker_port = read_file(file_mqtt_broker_port, String(DEFAULT_MQTT_BROKER_PORT));
  enable_pm25 = read_file(file_enable_pm25, String(DEFAULT_ENABLE_PM25));
  enable_dht =  read_file(file_enable_dht, String(DEFAULT_ENABLE_DHT));
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

void save_values_to_eeprom() {
  write_file(file_mqtt_server, mqtt_broker_url);
  write_file(file_mqtt_topic, mqtt_topic);
  write_file(file_mqtt_broker_port, mqtt_broker_port);
  write_file(file_enable_pm25, enable_pm25);
  write_file(file_enable_dht, enable_dht);
}


void write_deffault_to_eeprom(bool _with_save = true) {
  mqtt_broker_url = DEFAULT_MQTT_BROKER;
  mqtt_broker_port = DEFAULT_MQTT_BROKER_PORT;
  mqtt_topic = DEFAULT_MQTT_TOPIC;
  enable_pm25 = DEFAULT_ENABLE_PM25;
  enable_dht = DEFAULT_ENABLE_DHT;
  if (_with_save) {
    save_values_to_eeprom();
  }

}

uint32_t get_esp_chip_id() {
#if defined(ESP8266)
  return ESP.getChipId();
#elif defined(ESP32)
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
#else
  return 0;
#endif

}
void setup() {
  Serial.begin(9600);

  write_deffault_to_eeprom(false);
  if (SPIFFS.begin(true)) {
    Serial.println("SPIFFS Initialisierung....OK");
  }
  else {
    Serial.println("SPIFFS Initialisierung...Fehler!");
    write_deffault_to_eeprom(true);
  }
  // LOAD SETTINGS
  restore_eeprom_values();

  setup_wifi();


  //WEBSERVER ROUTES
  delay(1000);
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.on("/index.html", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  //REGISTER MDNS
  if (MDNS.begin((MDNS_NAME + String(get_esp_chip_id())).c_str())) {
  }


  //START OTA LIB
  ArduinoOTA.setHostname((MDNS_NAME + String(get_esp_chip_id())).c_str());
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
  if (enable_dht.toInt()) {
    dht.begin();
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    dht.humidity().getSensor(&sensor);
  }


  //SETUP MQTT
  setup_mqtt_client();




  //SETUP PM25 IKRA VIDRIKNING SENSOR
  if (enable_pm25.toInt()) {
    IkeaVindriktningSerialCom::setup();
    for (int i = 0; i < 1000; i++) {
      IkeaVindriktningSerialCom::handleUart(state);
      delay(1);
    }
  }

  Serial.println("_setup_complete_");

}




String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3])  ;
}


void mqtt_reconnect() {
  // Loop until we're reconnected
  if (client.connected()) {
    return;
  }
  // Attempt to connect
  if (client.connect((MDNS_NAME + String(get_esp_chip_id())).c_str())) {
    last_error = "MQTT CLICNET CONNCTED";
  } else {
    last_error = "MQTT CLCIENT CONECT FAILED WITH" + String(client.state());
  }
}

void setup_mqtt_client() {
  if (mqtt_broker_url != "" && mqtt_broker_port != "") {
    client.setServer(mqtt_broker_url.c_str(), mqtt_broker_port.toInt());
    if (client.connected()) {

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

    if (server.argName(i) == "enable_pm25") {
      enable_pm25 = server.arg(i);
      enable_pm25 = "set enable_pm25 to" + enable_pm25;

    }


    if (server.argName(i) == "enable_dht") {
      enable_dht = server.arg(i);
      enable_dht = "set enable_dht to" + enable_dht;

    }


    // formats the filesystem= resets all settings
    if (server.argName(i) == "fsformat") {
      if (SPIFFS.format()) {
        last_error = "Datei-System formatiert";
      }
      else {
        last_error = "Datei-System formatiert Error";
      }

    }
    //LOAD CURRENT SAVED DATA
    if (server.argName(i) == "eepromread") {
      restore_eeprom_values();
    }
    //LOAD FACOTRY RESET
    if (server.argName(i) == "factreset") {
      write_deffault_to_eeprom();
    }
  }
  //SAVE THESE DATA
  save_values_to_eeprom();
  //SAVE MQTT STUFF
  setup_mqtt_client();

  server.send(300, "text/html", "<html><head><meta http-equiv='refresh' content='1; url=/' /></head><body>SAVE SETTINGS PLEASE WAIT</body></html>");
}


void handleRoot()
{

  String control_forms = "<hr><h2>DEVICE INFO</h2>";
  control_forms += "<h3>" + String(MDNS_NAME) + String(get_esp_chip_id()) + "<br><br>" + BOARD_INFO + "</h3><br>";


  control_forms +=
    "<br><h3> MQTT SETTINGS </h3>"
    "<form name='btn_offmq' action='/save' method='GET'>"
    "<input type='text' value='" + String(mqtt_broker_url) + "' name='mqtt_broker_url' required placeholder='broker.hivemq.com'/>"
    "<input type='submit' value='SAVE MQTT BROKER'/>"
    "</form>"
    "<form name='btn_off' action='/save' method='GET'>"
    "<input type='number' value='" + String(mqtt_broker_port) + "' name='mqtt_broker_port' min='1' max='65536' required placeholder='1883'/>"
    "<input type='submit' value='SET MQTT BROKER PORT'/>"
    "</form>"
    "<form name='btn_off' action='/save' method='GET'>"
    "<input type='text' value='" + String(mqtt_topic) + "' name='mqtt_topic' required placeholder='/iot'/>"
    "<input type='submit' value='SET MQTT BASE TOPIC'/>"
    "</form>"
    "<form name='btn_off' action='/save' method='GET'>"
    "<input type='number' value='" + String(enable_pm25) + "' name='enable_pm25' min='0' max='1' required placeholder='1'/>"
    "<input type='submit' value='SET PM25 SENSOR STATE'/>"
    "</form>"
    "<form name='btn_off' action='/save' method='GET'>"
    "<input type='number' value='" + String(enable_dht) + "' name='enable_dht' min='0' max='1' required placeholder='1'/>"
    "<input type='submit' value='SET DHT SENSOR STATE'/>"
    "</form>"




    "<br><h3> DEVICE SETTINGS </h3>"
    "<form name='btn_on' action='/save' method='GET' required >"
    "<input type='hidden' value='fsformat' name='fsformat' />"
    "<input type='submit' value='DELETE CONFIGURATION'/>"
    "</form><br>"
    "<form name='btn_on' action='/save' method='GET' required >"
    "<input type='hidden' value='factreset' name='factreset' />"
    "<input type='submit' value='FACTORY RESET'/>"
    "</form><br>"
    "<form name='btn_on' action='/save' method='GET' required >"
    "<input type='hidden' value='eepromread' name='eepromread' />"
    "<input type='submit' value='READ STORED CONFIG'/>"
    "</form><br>"
    "<br><hr><h3>LAST SYSTEM MESSAGE</h3><br>" + last_error;





  server.send(200, "text/html", phead_1 + WEBSITE_TITLE + phead_2 + pstart + control_forms + pend);
}


void handleNotFound()
{
  server.send(404, "text/html", "<html><head>header('location: /'); </head></html>");
}



void setup_wifi() {
  delay(10);
  Serial.println();


  // START WFIFIMANAGER FOR CAPTIVE PORTAL
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true);
  wifiManager.setTimeout(120);
  //TRY TO CONNECT
  // AND DISPLAY IP ON CLOCKS HOUR DISPLAY (FOR 2 DIGIT CLOCKS)
  if (wifiManager.autoConnect(("SmartmeterConfiguration_" + String(get_esp_chip_id())).c_str())) {
    String ip = IpAddress2String(WiFi.localIP());
    Serial.println(ip);

  } else {
    Serial.println("WIFI CONNECTION ERROR");
  }


  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  if (client.connected()) {
    return;
  }
  Serial.print("Reconnecting...");
  if (!client.connect(("smartmeter_" + String(get_esp_chip_id())).c_str())) {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" retrying in 5 seconds");

  }

}



void publish_values() {
  float Temperatur = 0;
  float Luftfeuchtigkeit = 0;
  char temp[100];
  char hum[100];
  char bar[100];



  sensors_event_t event;
  if (enable_dht.toInt()) {
    Serial.print("Publish message: ");
    Serial.println(msg);
    dht.temperature().getEvent(&event);

    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      Serial.print(F("Temperature: "));
      Temperatur = event.temperature;
      Serial.print(Temperatur);
      Serial.println(F("°C"));
      snprintf (temp, 50, "%f", Temperatur);
      client.publish((mqtt_topic + "/" + String(get_esp_chip_id()) + "/temperature/").c_str(), temp);
    }

    dht.humidity().getEvent(&event);

    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      Serial.print(F("Humidity: "));
      Luftfeuchtigkeit = event.relative_humidity;
      Serial.print(Luftfeuchtigkeit);
      Serial.println(F("%"));
      snprintf (hum, 50, "%f", Luftfeuchtigkeit);
      client.publish((mqtt_topic + "/" + String(get_esp_chip_id()) + "/humidity/").c_str(), hum);
    }
  }



  if (enable_pm25.toInt()) {
    char pm25[100];
    IkeaVindriktningSerialCom::handleUart(state);
    float PM25 = state.lastPM25 * 1.00;
    Serial.print(F("PM2.5: "));
    Serial.print(PM25);
    Serial.println(F(""));

    snprintf (pm25, 50, "%f", PM25);
    client.publish((mqtt_topic + "/" + String(get_esp_chip_id()) + "/pm25/").c_str(), pm25);
    Serial.print("Publish message: ");
    Serial.println(pm25);
  }
}


unsigned long timeNow = 0;
unsigned long timeLast = 0;

void loop() {
  Serial.println(".");
  //HANDLE SERVER
  server.handleClient();




  if (!client.connected()) {
    mqtt_reconnect();
  } else {


    client.loop();
    if ((millis() - timeLast) > 1000) {
      timeLast = millis();
      publish_values();
    }



  }


  //HANDLE OTA
  ArduinoOTA.handle();








  delay(70);


}
