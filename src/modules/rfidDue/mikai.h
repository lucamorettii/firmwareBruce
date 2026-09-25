/**
 * @file mikai.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */

#ifndef __MIKAI_H__
#define __MIKAI_H__

#include "pn532_srix.h"
#include <Arduino.h>

class Mikai {
public:
    enum SRIX_State {
        IDLE_MODE,
        READ_TAG_MODE,
        SET_CREDIT_MODE,
        ADD_CREDIT_MODE,
        RESET_MODE,
        IMPORT_VENDOR_MODE,
        EXPORT_VENDOR_MODE
    };

    Mikai();
    ~Mikai();

    void setup();
    void loop();

private:
    Arduino_PN532_SRIX *nfc = nullptr;

    SRIX_State current_state;
    bool _screen_drawn = false;

    // RAM storage for 128 blocks (512 bytes)
    uint8_t _dump[128 * 4];

    void display_banner();
    void select_state();
    void set_state(SRIX_State state);
    void read_tag();
    void set_credit_tag();
    void add_credit_tag();
    void reset_tag();
    void import_vendor_tag();
    void export_vendor_tag();
    void show_main_menu();
};

void startMikai();

#endif
