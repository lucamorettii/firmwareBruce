/**
 * @file stoBene.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */

#ifndef __STOBENE_H__
#define __STOBENE_H__

#include "modules/rfid/PN532.h"
#include <Arduino.h>

class StoBene {
public:
    enum StoBene_State { IDLE_MODE, READ_TAG_MODE, SET_CREDIT_MODE, ADD_CREDIT_MODE };

    StoBene();
    ~StoBene();

    void setup();
    void loop();

private:
    PN532 *nfc = nullptr;

    StoBene_State current_state;
    bool _screen_drawn = false;

    void display_banner();
    void select_state();
    void set_state(StoBene_State state);
    void read_tag();
    void set_credit_tag();
    void add_credit_tag();
    void show_main_menu();
};

void startStoBene();

#endif
