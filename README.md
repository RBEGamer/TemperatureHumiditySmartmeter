# MSFHAC_Smartmeter

This is a rework project of the FH Aachen Makerspace Smartmeter project.
Its a simple mqtt based dht22 sensor node with an lasercut case for students.

The sensor uses a esp8266 or esp32 development board with an DHT22 sensor.

This repositoy contains a rewritten version of the original software, to make the project compatible with:

* ESP8266 and ESP32 Support
* easy wifi configuration
* mqtt2influx db converter script
* influx db 2.0 support



## BUILD INSTRUCTIONS

### SENSOR NODE

### INFLUXDB INSTALLATION

### MQTT2INFLUX BRIDGE


```bash
# CLONE REPO
cd /home/pi
git clone git@github.com:RBEGamer/MSFHAC_Smartmeter.git ./MSFHAC_Smartmeter

# MODIFY SCRIPT:
nano /home/pi/MSFHAC_Smartmeter/src/mqtt2influx/bridge/mqtt_iot_influx_bridge.py

# MAKE SCRIP EXECUTABLE
chmod +x /home/pi/MSFHAC_Smartmeter/src/mqtt2influx/bridge/mqtt_iot_influx_bridge.py
# $ contab -e INSERT:
@reboot python3 /home/pi/MSFHAC_Smartmeter/src/mqtt2influx/bridge/mqtt_iot_influx_bridge.py
```





