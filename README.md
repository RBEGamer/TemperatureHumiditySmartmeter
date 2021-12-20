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

![SCHEMATIC](documenation/schematic.png)

#### SOFTWARE

Flash the esp8266 or esp32 using the Arduino IDE.
Copy the `/src/smartmeter/required_libs`-folder into the `Arduino/libraries`-folder.
Open the `smartmeter.ino` file and select the right board under the tools tab.
If your board isnt present in the list, you may need to include the boards file.
To perform this task copy the following urls and insert these in the Arduino-IDE setting -> Additional Boards:

* `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json`
* `http://arduino.esp8266.com/stable/package_esp8266com_index.json`

Click upload, to compile and flash the program.



After resetting the node, scan the wifi for open netzworks and select  `SMARTMETER ConfigurationxxXXXxXX`, to set the wifi setting for your network.
If the webpage didnt open automatically, simply enter the following ip in your browser:

`http://192.168.4.1/`


After this step and an additional reset the node should be visiable in your network.
Open the ip adress of the node to configure you mqtt server and topic.


### INFLUXDB INSTALLATION

```bash
# INSTALL MQTT SERVER
sudo apt update
sudo apt upgrade -y
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto.service
sudo systemctl start mosquitto.service

# INSTALL INFLUX
curl -s https://repos.influxdata.com/influxdb2.key | gpg --import -
wget https://dl.influxdata.com/influxdb/releases/influxdb2-2.0.9-linux-arm.tar.gz.asc

```



### MQTT2INFLUX BRIDGE


```bash
# INSTALL PYTHON PACKAGES
sudo apt install python3-pip -y
sudo pip3 -H install paho-mqtt python-etcd
python3 -m pip install influxdb


# CLONE REPO
cd /home/pi
git clone git@github.com:RBEGamer/MSFHAC_Smartmeter.git ./MSFHAC_Smartmeter

# MODIFY SCRIPT:
nano /home/pi/MSFHAC_Smartmeter/src/mqtt2influx/bridge/mqtt_iot_influx_bridge.py

# 11 | INFLUXDB_ADDRESS = 'http://127.0.0.1:8086'
# 12 | INFLUXDB_ORG = 'iot'
# 13 | INFLUXDB_TOKEN = '7s4oE6wlSZld6VvsR9cz9lI-TmE3GuN-gAdji3Gqc_3a9MjbZs2B1dkIJ2gKZMJCs4mIxD4QRHDnjHWCzkL9nQ=='
# 14 | INFLUXDB_BUCKET = 'iot'

# 18 |  MQTT_PORT = 1883
# 19 | MQTT_ADDRESS = '127.0.0.1'

# MAKE SCRIP EXECUTABLE
chmod +x /home/pi/MSFHAC_Smartmeter/src/mqtt2influx/bridge/mqtt_iot_influx_bridge.py
# $ contab -e INSERT:
@reboot python3 /home/pi/MSFHAC_Smartmeter/src/mqtt2influx/bridge/mqtt_iot_influx_bridge.py
```





