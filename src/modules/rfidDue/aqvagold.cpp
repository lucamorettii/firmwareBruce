/**
 * @file aqvagold.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-26
 */
#include "aqvagold.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/settings.h"

#define AQVAGOLD_BLOCCO_CREDITO 12

Aqvagold::Aqvagold() {
    current_state = IDLE_MODE;
    setup();
}

Aqvagold::~Aqvagold() {
    delete nfc;
    releaseI2CBus();
}

void Aqvagold::setup() {
    drawMainBorderWithTitle("AQVAGOLD");
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

void Aqvagold::loop() {
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
        }

        if (returnToMenu) break;
    }
}

void Aqvagold::select_state() {
    options = {};

    options.emplace_back("Main Menu", [this]() { set_state(IDLE_MODE); });
    options.emplace_back("Read tag", [this]() { set_state(READ_TAG_MODE); });
    options.emplace_back("Set credit", [this]() { set_state(SET_CREDIT_MODE); });
    options.emplace_back("Add credit", [this]() { set_state(ADD_CREDIT_MODE); });

    loopOptions(options);
}

void Aqvagold::set_state(Aqvagold_State state) {
    current_state = state;
    _screen_drawn = false;
    display_banner();
    delay(300);
}

void Aqvagold::display_banner() {
    drawMainBorderWithTitle("AQVAGOLD");

    switch (current_state) {
        case READ_TAG_MODE: printSubtitle("READ TAG MODE"); break;
        case SET_CREDIT_MODE: printSubtitle("SET CREDIT MODE"); break;
        case ADD_CREDIT_MODE: printSubtitle("ADD CREDIT MODE"); break;
        case IDLE_MODE: printSubtitle("MAIN MENU"); break;
    }

    tft.setTextSize(FP);
    padprintln("");
}

void Aqvagold::show_main_menu() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FP);

    padprintln("AQVAGOLD version: 0.1");
    padprintln("");
    padprintln("Features:");
    padprintln("- Read Aqvagold tag");
    padprintln("- Set credit on Aqvagold tag");
    padprintln("- Add credit to Aqvagold tag");
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] to open menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void Aqvagold::read_tag() {

    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Aqvagold tag on the reader.");
    padprintln("");

    /* Aggiungo le chiavi Mifare */
    bruceConfig.ensureMifareKeysLoaded();
    if (setupSdCard()) {
        if (!SD.exists("/BruceRFID/Aqvagold.keys")) {
            displayError("File Aqvagold.keys not found", true);
        } else {
            File keysFile = SD.open("/BruceRFID/Aqvagold.keys");
            if (keysFile) {
                while (keysFile.available()) {
                    if (check(EscPress)) {
                        returnToMenu = true;
                        return;
                    }
                    String line = keysFile.readStringUntil('\n');
                    line.trim();
                    line.toUpperCase();
                    if (line.length() == 12) { bruceConfig.mifareKeys.insert(line); }
                }
                keysFile.close();
            }
        }
    }

    /* Lettura tag */
    byte buffer[18];
    bool readSuccess = false;
    const uint32_t readStart = millis();
    while (millis() - readStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            break;
        }

        if (nfc->readMifareBlock(AQVAGOLD_BLOCCO_CREDITO, buffer) == 0) {
            readSuccess = true;
            break;
        }
        delay(10);
    }

    if (returnToMenu) { return; }

    if (!readSuccess) {
        displayError("Aqvagold tag read failed!");
        delay(2000);
        set_state(READ_TAG_MODE);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    /* Stampa UID, Tipo e Credito */
    padprintln("UID: " + nfc->printableUID.uid);
    padprintln("Tipo: " + nfc->printableUID.picc_type);

    uint16_t credit = (uint16_t)(buffer[0] | buffer[1] << 8);
    uint16_t euro = credit / 1000;
    uint16_t mill = credit % 1000;
    String creditoStr = String(euro) + ".";
    if (mill < 100) creditoStr += "0";
    if (mill < 10) creditoStr += "0";
    creditoStr += String(mill) + " euro";
    padprintln("Credito: " + creditoStr);
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] for Main Menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void Aqvagold::set_credit_tag() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Aqvagold tag on the reader.");
    padprintln("");

    bruceConfig.ensureMifareKeysLoaded();
    byte buffer[18];
    bool readSuccess = false;
    const uint32_t readStart = millis();
    while (millis() - readStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            return;
        }
        if (nfc->readMifareBlock(AQVAGOLD_BLOCCO_CREDITO, buffer) == 0) {
            readSuccess = true;
            break;
        }
        delay(10);
    }

    if (!readSuccess) {
        displayError("Aqvagold tag read failed!");
        delay(2000);
        set_state(SET_CREDIT_MODE);
        return;
    }

    uint16_t storedCredit = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
    uint16_t currentCredit = storedCredit / 10;
    String value = num_keyboard("", 5, "Credit in cents (" + String(currentCredit) + "):");
    if (value == "\x1B") {
        set_state(SET_CREDIT_MODE);
        return;
    }

    long credit = value.toInt();
    if (value.isEmpty() || credit < 100 || credit > 1000) {
        displayError("Invalid credit!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    long storedCreditValue = credit * 10;
    buffer[0] = (uint8_t)storedCreditValue;
    buffer[1] = (uint8_t)(storedCreditValue >> 8);

    display_banner();
    padprintln("Updating Aqvagold tag...");
    padprintln("");
    if (nfc->writeMifareBlock(AQVAGOLD_BLOCCO_CREDITO, buffer) != 0) {
        displayError("Aqvagold tag write failed!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    displaySuccess("Credit set successfully!");
    delay(1000);
    set_state(IDLE_MODE);
}

void Aqvagold::add_credit_tag() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Aqvagold tag on the reader.");
    padprintln("");

    bruceConfig.ensureMifareKeysLoaded();
    byte buffer[18];
    bool readSuccess = false;
    const uint32_t readStart = millis();
    while (millis() - readStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            return;
        }
        if (nfc->readMifareBlock(AQVAGOLD_BLOCCO_CREDITO, buffer) == 0) {
            readSuccess = true;
            break;
        }
        delay(10);
    }

    if (!readSuccess) {
        displayError("Aqvagold tag read failed!");
        delay(2000);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    uint16_t storedCredit = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
    uint16_t currentCredit = storedCredit / 10;
    String value = num_keyboard("", 5, "Add cents (" + String(currentCredit) + "):");
    if (value == "\x1B") {
        set_state(ADD_CREDIT_MODE);
        return;
    }

    long amount = value.toInt();
    long newCredit = (long)currentCredit + amount;
    if (value.isEmpty() || amount < 100 || newCredit > 1000) {
        displayError("Invalid credit!", true);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    long storedCreditValue = newCredit * 10;
    buffer[0] = (uint8_t)storedCreditValue;
    buffer[1] = (uint8_t)(storedCreditValue >> 8);

    display_banner();
    padprintln("Updating Aqvagold tag...");
    padprintln("");
    if (nfc->writeMifareBlock(AQVAGOLD_BLOCCO_CREDITO, buffer) != 0) {
        displayError("Aqvagold tag write failed!", true);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    displaySuccess("Credit added successfully!");
    delay(1000);
    set_state(IDLE_MODE);
}

void startAqvagold() { Aqvagold aqvagold_tool; }
