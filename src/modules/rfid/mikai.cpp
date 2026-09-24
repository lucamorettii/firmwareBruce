/**
 * @file mikai.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-24
 */
#include "mikai.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/settings.h"
#include "mikai_logic.h"
#include <cstring>
#include <vector>

Mikai::Mikai() {
    current_state = IDLE_MODE;
    setup();
}

Mikai::~Mikai() {
    delete nfc;
    releaseI2CBus();
}

void Mikai::setup() {
    drawMainBorderWithTitle("MIKAI");
    padprintln("");
    padprintln("Initializing I2C...");

    // Init I2C
    TwoWire *Wire = acquireI2CBus();
    Wire->setClock(100000);

    padprintln("Initializing PN532...");

#if defined(PN532_IRQ) && defined(PN532_RF_REST)
    nfc = new Arduino_PN532_SRIX(PN532_IRQ, PN532_RF_REST);
    padprintln("Hardware mode (IRQ + RST)");
#else
    nfc = new Arduino_PN532_SRIX(-1, -1);
    padprintln("I2C-only mode");
#endif
    nfc->setWire(Wire);

    if (!nfc->init()) {
        displayError("PN532 not found!", true);
        return;
    }

    padprintln("Init OK, testing retries...");

    // Configure for SRIX
    if (!nfc->setPassiveActivationRetries(0xFF)) {
        displayError("Retry config failed!", true);
        delay(500);
        return;
    }

    padprintln("Testing SRIX init...");
    if (!nfc->SRIX_init()) {
        displayError("SRIX init failed!", true);
        return;
    }
    uint32_t ver = nfc->getFirmwareVersion();
    if (ver) {
        uint8_t chip = (ver >> 24) & 0xFF;
        uint8_t fw_major = (ver >> 16) & 0xFF;
        uint8_t fw_minor = (ver >> 8) & 0xFF;

        padprintln("Chip: PN5" + String(chip, HEX));
        padprintln("FW: " + String(fw_major) + "." + String(fw_minor));
    }
    delay(1000);
    displaySuccess("PN532-SRIX ready!");
    delay(1000);

    set_state(IDLE_MODE);
    return loop();
}

void Mikai::loop() {
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

void Mikai::select_state() {
    options = {};

    options.emplace_back("Main Menu", [this]() { set_state(IDLE_MODE); });
    options.emplace_back("Read tag", [this]() { set_state(READ_TAG_MODE); });

    loopOptions(options);
}

void Mikai::set_state(SRIX_State state) {
    current_state = state;
    _screen_drawn = false;
    display_banner();
    delay(300);
}

void Mikai::display_banner() {
    drawMainBorderWithTitle("MIKAI");

    switch (current_state) {
        case READ_TAG_MODE: printSubtitle("READ TAG MODE"); break;
        case IDLE_MODE: printSubtitle("MAIN MENU"); break;
    }

    tft.setTextSize(FP);
    padprintln("");
}

void Mikai::show_main_menu() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FP);

    padprintln("Mikai version: 0.1");
    padprintln("");
    padprintln("Features:");
    padprintln("- Read info Mikai tag");
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] to open menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void Mikai::read_tag() {

    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Mikai tag on the reader.");
    padprintln("");

    if (!mikai_read_tag(&srixKey, nfc)) {
        displayError("Mikai tag read failed!");
        delay(2000);
        set_state(READ_TAG_MODE);
        return;
    }

    memcpy(_dump, srixKey.srix4k->eeprom, sizeof(_dump));

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    char info[1024];
    mikai_get_info_string(&srixKey, info, sizeof(info));
    for (char *line = strtok(info, "\n"); line != nullptr; line = strtok(nullptr, "\n")) {
        padprintln(String(line));
    }
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] for Main Menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void startMikai() { Mikai mikai_tool; }
