/**
 * @file microel.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-24
 */

#ifndef __MICROEL_H__
#define __MICROEL_H__

#include "PN532.h"
#include <Arduino.h>

class Microel {
public:
    enum Microel_State { IDLE_MODE, READ_TAG_MODE };

    Microel();
    ~Microel();

    void setup();
    void loop();

private:
    PN532 *nfc = nullptr;

    Microel_State current_state;
    bool _screen_drawn = false;

    // RAM storage for 128 blocks (512 bytes)
    uint8_t _dump[128 * 4];

    void display_banner();
    void select_state();
    void set_state(Microel_State state);
    void read_tag();
    void show_main_menu();
};

void startMicroel();

#endif
