/**
 * @file microel.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-24
 */

#ifndef __MICROEL_H__
#define __MICROEL_H__

#include "modules/rfid/PN532.h"
#include <Arduino.h>

class Microel {
public:
    enum Microel_State { IDLE_MODE, READ_TAG_MODE, SET_CREDIT_MODE, ADD_CREDIT_MODE, CALCULATE_KEYS_MODE };

    Microel();
    ~Microel();

    void setup();
    void loop();

private:
    PN532 *nfc = nullptr;

    Microel_State current_state;
    bool _screen_drawn = false;

    void display_banner();
    void select_state();
    void set_state(Microel_State state);
    void read_tag();
    void set_credit_tag();
    void add_credit_tag();
    void calculate_keys_tag();
    void show_main_menu();
};

void startMicroel();

#endif
