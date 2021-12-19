import re
import random
from typing import NamedTuple
import re

import paho.mqtt.client as mqtt

from influxdb_client import InfluxDBClient, Point, WriteOptions
from influxdb_client.client.write_api import SYNCHRONOUS

INFLUXDB_ADDRESS = 'http://127.0.0.1:8086'
INFLUXDB_ORG = 'prodevmo'
INFLUXDB_TOKEN = '7s4oE6wlSZld6VvsR9cz9lI-TmE3GuN-gAdji3Gqc_3a9MjbZs2B1dkIJ2gKZMJCs4mIxD4QRHDnjHWCzkL9nQ=='
INFLUXDB_BUCKET = 'iot'

INFLUX_DISABLE_STRING_DATA_WRITE = True

MQTT_PORT = 1883
MQTT_ADDRESS = '192.168.178.89'
MQTT_TOPIC = '#'
MQTT_TOPCI_REGEX = r'(\/|){0,1}iot\/(.)*$' # None or r'^$'
MQTT_CLIENT_ID = 'MQTTInfluxDBBridge'
MQTT_USER = None
MQTT_PW = None

influxdb_client = InfluxDBClient(url=INFLUXDB_ADDRESS, org=INFLUXDB_ORG, token=INFLUXDB_TOKEN)
influx_write_client = influxdb_client.write_api(
    write_options=WriteOptions(batch_size=500, flush_interval=10_000, jitter_interval=2_000, retry_interval=5_000))

def isfloat(value):
  try:
    float(value)
    return True
  except ValueError:
    return False


class SensorData():
    measurement: str
    value: str
    type: str
    topic: str


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def _send_sensor_data_to_influxdb(sensor_data):


    value = None

    if re.match(r'^-?\d+(?:\.\d+)$', sensor_data.value) is not None:

        value = float(sensor_data.value)
    elif re.match(r'^[-+]?[0-9]+$', sensor_data.value) is not None:
        value = int(sensor_data.value)
    else:
        if INFLUX_DISABLE_STRING_DATA_WRITE:
            return
        value = str(sensor_data.value)

    print(value, type(value))
    data_point = {
        "measurement": sensor_data.topic,
        "tags": {

        },
        "fields": {
            "value": value
        },

    }

    tpsp = sensor_data.topic.split('/')
    #data_point['tags']

    influx_write_client.write(INFLUXDB_BUCKET, INFLUXDB_ORG, [data_point])
    ##influxdb_client.write_points(json_body)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))

    if MQTT_TOPCI_REGEX is not None:
        if re.match(MQTT_TOPCI_REGEX, msg.topic) is None:
            return

    payload = msg.payload.decode('utf-8')
    sensor_data = SensorData()
    sensor_data.topic = msg.topic
    sensor_data.value = payload
    sensor_data.type = type(payload)
    sensor_data.measurement = msg.topic

    if sensor_data is not None:
        _send_sensor_data_to_influxdb(sensor_data)


def _init_influxdb_database():
    pass
    # databases = influxdb_client.get_list_database()
    # if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
    #    influxdb_client.create_database(INFLUXDB_DATABASE)
    # influxdb_client.switch_database(INFLUXDB_DATABASE)


def main():
    _init_influxdb_database()

    mqtt_client = mqtt.Client()
    if MQTT_USER is not None:
        mqttpw = ""
        if MQTT_PW is not None:
            mqttpw = MQTT_PW
        mqtt_client.username_pw_set(MQTT_USER, mqttpw)

    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, MQTT_PORT)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    while 1:
        try:
            main()
        except KeyboardInterrupt:
            print("Ende")
            break
        except Exception as e:
            print(e)

# See PyCharm help at https://www.jetbrains.com/help/pycharm/

