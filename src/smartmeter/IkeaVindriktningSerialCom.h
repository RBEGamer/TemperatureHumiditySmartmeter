#pragma once

//ORIGINAL VERSION: github.com/Hypfer/esp8266-vindriktning-particle-sensor
//MODIFIED by Marcel Ochsendorf marcelochsendorf.com
// # 02.10.2021
// added state.lastPM25
// rework Serial.printf

//USE THIS FLAG ON ARDUINO UNO/NANO WITH ETHERNET SHIELD
//DONT SET FOR ESP32 OR ESP8266
//#define USE_SOFTWARE_SERIAL


#define DEBUG_SERIAL Serial



#ifdef USE_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
SoftwareSerial sensorSerial(05, -1); // RX,
#define SERIAL_VK sensorSerial
#else

//ON ESP32 WE USE THE BUILD IN SERIAL 2 WITH DIFFERENT PINS
#if defined(ESP32)
  #define RXD2 16
  #define TXD2 17
  #define SERIAL_VK Serial2
  
#elif defined(ESP8266)
  #define SERIAL_VK Serial1
#else
  #define SERIAL_VK Serial2
#endif

#endif



#include "IkeaVindriktningTypes.h"

namespace IkeaVindriktningSerialCom {
    

    

    uint8_t serialRxBuf[255];
    uint8_t rxBufIdx = 0;

    void setup() {

        #ifdef USE_SOFTWARE_SERIAL
          SERIAL_VK.begin(9600);
        #else

          #ifdef ESP32
          SERIAL_VK.begin(9600, SERIAL_8N1, RXD2,-1);
          #else
            SERIAL_VK.begin(9600);
          #endif
        #endif
    }

    void clearRxBuf() {
        // Clear everything for the next message
        memset(serialRxBuf, 0, sizeof(serialRxBuf));
        rxBufIdx = 0;
    }

    void parseState(particleSensorState_t& state) {
        /**
         *         MSB  DF 3     DF 4  LSB
         * uint16_t = xxxxxxxx xxxxxxxx
         */
        const uint16_t pm25 = (serialRxBuf[5] << 8) | serialRxBuf[6];
        state.lastPM25 = pm25;
 
        //Serial.print("Received RAW PM 2.5 reading:");
        //Serial.println(pm25);

        state.measurements[state.measurementIdx] = pm25;

        state.measurementIdx = (state.measurementIdx + 1) % 5;

        if (state.measurementIdx == 0) {
            float avgPM25 = 0.0f;

            for (uint8_t i = 0; i < 5; ++i) {
                avgPM25 += state.measurements[i] / 5.0f;
            }

            state.avgPM25 = avgPM25;
            state.valid = true;
            DEBUG_SERIAL.print("NEW PM 2.5 READING:");
            DEBUG_SERIAL.println(state.avgPM25);
        }

        clearRxBuf();
    }

    bool isValidHeader() {
        bool headerValid = serialRxBuf[0] == 0x16 && serialRxBuf[1] == 0x11 && serialRxBuf[2] == 0x0B;

        if (!headerValid) {
            DEBUG_SERIAL.println("Received message with invalid header.");
        }

        return headerValid;
    }

    bool isValidChecksum() {
        uint8_t checksum = 0;

        for (uint8_t i = 0; i < 20; i++) {
            checksum += serialRxBuf[i];
        }

        if (checksum != 0) {
            DEBUG_SERIAL.print("Received message with invalid checksum. Expected: 0. Actual: \n");
            DEBUG_SERIAL.println(checksum);
        }

        return checksum == 0;
    }

    void handleUart(particleSensorState_t& state) {
        if (!SERIAL_VK.available()) {
            return;
        }

        //Serial.print("Receiving:");
        while (SERIAL_VK.available()) {
            serialRxBuf[rxBufIdx++] = SERIAL_VK.read();
            //Serial.print(".");

            // Without this delay, receiving data breaks for reasons that are beyond me
            delay(15);

            if (rxBufIdx >= 64) {
                clearRxBuf();
            }
        }
        //Serial.println("Done.");

        if (isValidHeader() && isValidChecksum()) {
            parseState(state);
        } else {
            clearRxBuf();
        }
    }
} // namespace SerialCom
