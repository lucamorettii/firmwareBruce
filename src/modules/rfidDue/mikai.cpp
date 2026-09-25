/**
 * @file mikai.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */
#include "mikai.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/settings.h"
#include "mikai_logic.h"
#include <cstring>

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
        if (returnToMenu) break;

        switch (current_state) {
            case IDLE_MODE: show_main_menu(); break;
            case READ_TAG_MODE: read_tag(); break;
            case SET_CREDIT_MODE: set_credit_tag(); break;
            case ADD_CREDIT_MODE: add_credit_tag(); break;
            case RESET_MODE: reset_tag(); break;
            case IMPORT_VENDOR_MODE: import_vendor_tag(); break;
            case EXPORT_VENDOR_MODE: export_vendor_tag(); break;
        }
    }
}

void Mikai::select_state() {
    options = {};

    options.emplace_back("Main Menu", [this]() { set_state(IDLE_MODE); });
    options.emplace_back("Read tag", [this]() { set_state(READ_TAG_MODE); });
    options.emplace_back("Set credit", [this]() { set_state(SET_CREDIT_MODE); });
    options.emplace_back("Add credit", [this]() { set_state(ADD_CREDIT_MODE); });
    options.emplace_back("Reset tag", [this]() { set_state(RESET_MODE); });
    options.emplace_back("Import vendor tag", [this]() { set_state(IMPORT_VENDOR_MODE); });
    options.emplace_back("Export vendor tag", [this]() { set_state(EXPORT_VENDOR_MODE); });

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
        case SET_CREDIT_MODE: printSubtitle("SET CREDIT MODE"); break;
        case ADD_CREDIT_MODE: printSubtitle("ADD CREDIT MODE"); break;
        case RESET_MODE: printSubtitle("RESET MODE"); break;
        case IMPORT_VENDOR_MODE: printSubtitle("IMPORT VENDOR MODE"); break;
        case EXPORT_VENDOR_MODE: printSubtitle("EXPORT VENDOR MODE"); break;
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
    padprintln("- Read Mikai tag");
    padprintln("- Set credit on Mikai tag");
    padprintln("- Add credit to Mikai tag");
    padprintln("- Reset Mikai tag");
    padprintln("- Import vendor data from Mikai tag");
    padprintln("- Export vendor data from Mikai tag");

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
        if (returnToMenu) return;
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

void Mikai::set_credit_tag() {

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
        set_state(SET_CREDIT_MODE);
        return;
    }

    memcpy(_dump, srixKey.srix4k->eeprom, sizeof(_dump));

    uint8_t day = 15, month = 7, year = 26;
    String value = num_keyboard("", 5, "Credit in cents:");
    if (value == "\x1B") { // User pressed ESC
        set_state(SET_CREDIT_MODE);
        return;
    }
    display_banner();
    padprintln("Updating Mikai tag...");
    padprintln("");
    long cents = value.toInt();
    if (value.isEmpty() || cents < 5 || cents > 5000) {
        displayError("Invalid credit!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    int result = mikai_set_cents(&srixKey, (uint16_t)cents, day, month, year);
    if (result < 0 || !mikai_has_pending_writes(&srixKey)) {
        displayError("Impossible to set credit!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    if (mikai_write_modified_blocks(&srixKey, nfc) != 0) {
        displayError("Tag write failed!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    displaySuccess("Credit set successfully!");
    delay(1000);
    set_state(IDLE_MODE);
}

void Mikai::add_credit_tag() {

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
        set_state(ADD_CREDIT_MODE);
        return;
    }

    memcpy(_dump, srixKey.srix4k->eeprom, sizeof(_dump));

    uint8_t day = 15, month = 7, year = 26;
    String value = num_keyboard("", 5, "Add cents:");
    if (value == "\x1B") { // User pressed ESC
        set_state(ADD_CREDIT_MODE);
        return;
    }
    display_banner();
    padprintln("Updating Mikai tag...");
    padprintln("");
    long cents = value.toInt();
    if (value.isEmpty() || cents < 5 || cents > 5000) {
        displayError("Invalid credit!", true);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    int result = mikai_add_cents(&srixKey, (uint16_t)cents, day, month, year);
    if (result < 0 || !mikai_has_pending_writes(&srixKey)) {
        displayError("Unable to add credit!", true);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    if (mikai_write_modified_blocks(&srixKey, nfc) != 0) {
        displayError("Tag write failed!", true);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    displaySuccess("Additional credit added!");
    delay(1000);
    set_state(IDLE_MODE);
}

void Mikai::reset_tag() {

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
        set_state(RESET_MODE);
        return;
    }

    memcpy(_dump, srixKey.srix4k->eeprom, sizeof(_dump));

    mikai_reset_key(&srixKey);
    if (!mikai_has_pending_writes(&srixKey)) {
        displayError("No changes to write!", true);
        set_state(RESET_MODE);
        return;
    }

    if (mikai_write_modified_blocks(&srixKey, nfc) != 0) {
        displayError("Reset write failed!", true);
        set_state(RESET_MODE);
        return;
    }

    displaySuccess("Tag reset successfully!");
    delay(1000);
    set_state(IDLE_MODE);
}

void Mikai::import_vendor_tag() {}

void Mikai::export_vendor_tag() {}

void startMikai() { Mikai mikai_tool; }
