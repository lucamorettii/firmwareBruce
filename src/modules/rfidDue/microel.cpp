/**
 * @file microel.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */
#include "microel.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/settings.h"
#include "microel_logic.h"

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

    delay(1000);
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
            case SET_CREDIT_MODE: set_credit_tag(); break;
            case ADD_CREDIT_MODE: add_credit_tag(); break;
            case CALCULATE_KEYS_MODE: calculate_keys_tag(); break;
        }

        if (returnToMenu) break;
    }
}

void Microel::select_state() {
    options = {};

    options.emplace_back("Main Menu", [this]() { set_state(IDLE_MODE); });
    options.emplace_back("Read tag", [this]() { set_state(READ_TAG_MODE); });
    options.emplace_back("Set credit", [this]() { set_state(SET_CREDIT_MODE); });
    options.emplace_back("Add credit", [this]() { set_state(ADD_CREDIT_MODE); });
    options.emplace_back("Calculate Keys", [this]() { set_state(CALCULATE_KEYS_MODE); });

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
        case SET_CREDIT_MODE: printSubtitle("SET CREDIT MODE"); break;
        case ADD_CREDIT_MODE: printSubtitle("ADD CREDIT MODE"); break;
        case CALCULATE_KEYS_MODE: printSubtitle("CALCULATE KEYS MODE"); break;
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
    padprintln("- Set credit on Microel tag");
    padprintln("- Add credit to Microel tag");
    padprintln("- Calculate Keys on Microel tag");
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

    if (!microel_read_tag(settore, nfc)) {
        if (returnToMenu) return;
        displayError("Microel tag read failed!");
        delay(2000);
        set_state(READ_TAG_MODE);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    String info = microel_get_info_string(settore);
    int inizio = 0;
    while (inizio < info.length()) {
        int fine = info.indexOf('\n', inizio);
        if (fine < 0) fine = info.length();
        padprintln(info.substring(inizio, fine));
        inizio = fine + 1;
    }
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] for Main Menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void Microel::set_credit_tag() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Microel tag on the reader.");
    padprintln("");

    if (!microel_read_tag(settore, nfc)) {
        if (returnToMenu) return;
        displayError("Microel tag read failed!");
        delay(2000);
        set_state(SET_CREDIT_MODE);
        return;
    }

    uint8_t day = 15, month = 7, year = 26;
    String value = num_keyboard("", 5, "Credit in cents:");
    if (value == "\x1B") { // User pressed ESC
        set_state(SET_CREDIT_MODE);
        return;
    }
    display_banner();
    padprintln("Updating Microel tag...");
    padprintln("");
    long cents = value.toInt();
    if (value.isEmpty() || cents < 200 || cents > 5000) {
        displayError("Invalid credit!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    if (microel_set_cents(settore, (uint16_t)cents, day, month, year)) {
        displayError("Impossible to set credit!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    if (microel_write_modified_blocks(settore, nfc)) {
        displayError("Tag write failed!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    displaySuccess("Credit set successfully!");
    delay(1000);
    set_state(IDLE_MODE);
}

void Microel::add_credit_tag() {}

void Microel::calculate_keys_tag() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Microel tag on the reader.");
    padprintln("");

    if (!microel_read_tag(settore, nfc)) {
        if (returnToMenu) return;
        displayError("Microel tag read failed!");
        delay(2000);
        set_state(CALCULATE_KEYS_MODE);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    String info = microel_get_keys_string(settore);
    int inizio = 0;
    while (inizio < info.length()) {
        int fine = info.indexOf('\n', inizio);
        if (fine < 0) fine = info.length();
        padprintln(info.substring(inizio, fine));
        inizio = fine + 1;
    }
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] for Main Menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void startMicroel() { Microel microel_tool; }
