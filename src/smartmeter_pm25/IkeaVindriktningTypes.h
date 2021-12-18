#pragma once

//ORIGINAL VERSION: github.com/Hypfer/esp8266-vindriktning-particle-sensor
//MODIFIED by Marcel Ochsendorf marcelochsendorf.com
// # 02.10.2021
// added state.lastPM25
// rework Serial.printf

struct particleSensorState_t {
    uint16_t avgPM25 = 0;
    uint16_t lastPM25 = 0;
    uint16_t measurements[5] = {0, 0, 0, 0, 0};
    uint8_t measurementIdx = 0;
    boolean valid = false;
};