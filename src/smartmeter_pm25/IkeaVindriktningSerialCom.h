#pragma once

//ORIGINAL VERSION: github.com/Hypfer/esp8266-vindriktning-particle-sensor
//MODIFIED by Marcel Ochsendorf marcelochsendorf.com
// # 02.10.2021
// added state.lastPM25
// rework Serial.printf



#include "IkeaVindriktningTypes.h"
#define RXD2 16
#define TXD2 17
namespace IkeaVindriktningSerialCom {
    constexpr static const uint8_t PIN_UART_RX = 8; // D2 on Wemos D1 Mini
    constexpr static const uint8_t PIN_UART_TX = 9; // UNUSED

    

    uint8_t serialRxBuf[255];
    uint8_t rxBufIdx = 0;

    void setup() {
        //sensorSerial.begin(9600);
        Serial2.begin(9600, SERIAL_8N1, RXD2,TXD2);
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
            Serial.print("NEW PM 2.5 READING:");
            Serial.println(state.avgPM25);
        }

        clearRxBuf();
    }

    bool isValidHeader() {
        bool headerValid = serialRxBuf[0] == 0x16 && serialRxBuf[1] == 0x11 && serialRxBuf[2] == 0x0B;

        if (!headerValid) {
            Serial.println("Received message with invalid header.");
        }

        return headerValid;
    }

    bool isValidChecksum() {
        uint8_t checksum = 0;

        for (uint8_t i = 0; i < 20; i++) {
            checksum += serialRxBuf[i];
        }

        if (checksum != 0) {
            Serial.print("Received message with invalid checksum. Expected: 0. Actual: \n");
            Serial.println(checksum);
        }

        return checksum == 0;
    }

    void handleUart(particleSensorState_t& state) {
        if (!Serial2.available()) {
            return;
        }

        //Serial.print("Receiving:");
        while (Serial2.available()) {
            serialRxBuf[rxBufIdx++] = Serial2.read();
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
            /*
            Serial.printf(
                "Current measurements: %d, %d, %d, %d, %d\n",

                state.measurements[0],
                state.measurements[1],
                state.measurements[2],
                state.measurements[3],
                state.measurements[4]
            );
            */
        } else {
            clearRxBuf();
        }
    }
} // namespace SerialCom
