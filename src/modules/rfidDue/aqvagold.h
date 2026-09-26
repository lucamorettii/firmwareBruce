/**
 * @file aqvagold.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */

#ifndef __AQVAGOLD_H__
#define __AQVAGOLD_H__

#include "modules/rfid/PN532.h"
#include <Arduino.h>

class AqvaGold {
public:
    enum AqvaGold_State { IDLE_MODE, READ_TAG_MODE, SET_CREDIT_MODE, ADD_CREDIT_MODE };

    AqvaGold();
    ~AqvaGold();

    void setup();
    void loop();

private:
    PN532 *nfc = nullptr;

    AqvaGold_State current_state;
    bool _screen_drawn = false;

    void display_banner();
    void select_state();
    void set_state(AqvaGold_State state);
    void read_tag();
    void set_credit_tag();
    void add_credit_tag();
    void show_main_menu();
};

void startAqvaGold();

#endif
