/**
 * @file microel.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-24
 */
#include "microel.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/settings.h"
#include "microel_logic.h"
#include <cstring>
#include <vector>

Microel::Microel() {
    current_state = IDLE_MODE;
    setup();
}

Microel::~Microel() {
    delete nfc;
    releaseI2CBus();
}

void Microel::setup() {
    drawMainBorderWithTitle("MICROEL");
    padprintln("");
    padprintln("Initializing I2C...");

    padprintln("Initializing PN532...");

    nfc = new PN532(PN532::CONNECTION_TYPE::I2C);
    if (!nfc->begin()) {
        displayError("PN532 not found!", true);
        return;
    }

    padprintln("PN532 ready for MIFARE.");

    set_state(IDLE_MODE);
    return loop();
}

void Microel::loop() {
    while (1) {
        if (check(EscPress)) {
            returnToMenu = true;
            break;
        }

        if (check(SelPress)) { select_state(); }

        switch (current_state) {
            case IDLE_MODE: show_main_menu(); break;
            case READ_TAG_MODE: read_tag(); break;
        }
    }
}

void Microel::select_state() {
    options = {};

    options.emplace_back("Main Menu", [this]() { set_state(IDLE_MODE); });
    options.emplace_back("Read tag", [this]() { set_state(READ_TAG_MODE); });

    loopOptions(options);
}

void Microel::set_state(Microel_State state) {
    current_state = state;
    _screen_drawn = false;
    display_banner();
    delay(300);
}

void Microel::display_banner() {
    drawMainBorderWithTitle("MICROEL");

    switch (current_state) {
        case READ_TAG_MODE: printSubtitle("READ TAG MODE"); break;
        case IDLE_MODE: printSubtitle("MAIN MENU"); break;
    }

    tft.setTextSize(FP);
    padprintln("");
}

void Microel::show_main_menu() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FP);

    padprintln("Microel version: 0.1");
    padprintln("");
    padprintln("Features:");
    padprintln("- Read Microel tag");
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] to open menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void Microel::read_tag() {

    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Microel tag on the reader.");
    padprintln("");

    if (nfc->read(PN532_MIFARE_ISO14443A) == RFIDInterface::TAG_NOT_PRESENT) {
        displayError("MIFARE tag not found!");
        delay(2000);
        set_state(READ_TAG_MODE);
        return;
    }

    display_banner();
    padprintln("MIFARE tag read successfully.");
    padprintln("");
    padprintln("Press [OK] for Main Menu");
    _screen_drawn = true;
}

void startMicroel() { Microel microel_tool; }
