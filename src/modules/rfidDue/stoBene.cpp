/**
 * @file stoBene.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-26
 */
#include "stoBene.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/settings.h"

#define STOBENE_BLOCCO_CREDITO_PRECEDENTE 50
#define STOBENE_BLOCCO_CREDITO 54

StoBene::StoBene() {
    current_state = IDLE_MODE;
    setup();
}

StoBene::~StoBene() {
    delete nfc;
    releaseI2CBus();
}

void StoBene::setup() {
    drawMainBorderWithTitle("Sto&Bene");
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

void StoBene::loop() {
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

void StoBene::select_state() {
    options = {};

    options.emplace_back("Main Menu", [this]() { set_state(IDLE_MODE); });
    options.emplace_back("Read tag", [this]() { set_state(READ_TAG_MODE); });
    options.emplace_back("Set credit", [this]() { set_state(SET_CREDIT_MODE); });
    options.emplace_back("Add credit", [this]() { set_state(ADD_CREDIT_MODE); });

    loopOptions(options);
}

void StoBene::set_state(StoBene_State state) {
    current_state = state;
    _screen_drawn = false;
    display_banner();
    delay(300);
}

void StoBene::display_banner() {
    drawMainBorderWithTitle("Sto&Bene");

    switch (current_state) {
        case READ_TAG_MODE: printSubtitle("READ TAG MODE"); break;
        case SET_CREDIT_MODE: printSubtitle("SET CREDIT MODE"); break;
        case ADD_CREDIT_MODE: printSubtitle("ADD CREDIT MODE"); break;
        case IDLE_MODE: printSubtitle("MAIN MENU"); break;
    }

    tft.setTextSize(FP);
    padprintln("");
}

void StoBene::show_main_menu() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
    tft.setTextSize(FP);

    padprintln("Sto&Bene version: 0.1");
    padprintln("");
    padprintln("Features:");
    padprintln("- Read YBB tag");
    padprintln("- Set credit on YBB tag");
    padprintln("- Add credit to YBB tag");
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] to open menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void StoBene::read_tag() {

    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Sto&Bene tag on the reader.");
    padprintln("");

    /* Aggiungo le chiavi Mifare */
    bruceConfig.ensureMifareKeysLoaded();
    if (setupSdCard()) {
        if (!SD.exists("/BruceRFID/StoBene.keys")) {
            displayError("File StoBene.keys not found", true);
        } else {
            File keysFile = SD.open("/BruceRFID/StoBene.keys");
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
    byte previousBuffer[18];
    bool readSuccess = false;
    bool previousReadSuccess = false;
    const uint32_t readStart = millis();
    while (millis() - readStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            break;
        }

        if (nfc->readMifareBlock(STOBENE_BLOCCO_CREDITO, buffer) == 0) {
            readSuccess = true;
            break;
        }
        delay(10);
    }

    if (returnToMenu) { return; }

    const uint32_t previousReadStart = millis();
    while (readSuccess && millis() - previousReadStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            break;
        }

        if (nfc->readMifareBlock(STOBENE_BLOCCO_CREDITO_PRECEDENTE, previousBuffer) == 0) {
            previousReadSuccess = true;
            break;
        }
        delay(10);
    }

    if (returnToMenu) { return; }

    if (!readSuccess || !previousReadSuccess) {
        displayError("Sto&Bene tag read failed!");
        delay(2000);
        set_state(READ_TAG_MODE);
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    /* Stampa UID, Tipo e Credito */
    padprintln("UID: " + nfc->printableUID.uid);
    padprintln("Tipo: " + nfc->printableUID.picc_type);

    uint16_t credit = (uint16_t)((buffer[1] << 8) | buffer[2]);
    uint16_t previousCredit = (uint16_t)((previousBuffer[1] << 8) | previousBuffer[2]);
    uint16_t euro = credit / 100;
    uint16_t cent = credit % 100;
    String creditoStr = String(euro) + ".";
    if (cent < 10) creditoStr += "0";
    creditoStr += String(cent) + " euro";
    euro = previousCredit / 100;
    cent = previousCredit % 100;
    String previousCreditStr = String(euro) + ".";
    if (cent < 10) previousCreditStr += "0";
    previousCreditStr += String(cent) + " euro";
    padprintln("Credito: " + creditoStr);
    padprintln("Credito precedente: " + previousCreditStr);
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] for Main Menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void StoBene::set_credit_tag() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Sto&Bene tag on the reader.");
    padprintln("");

    bruceConfig.ensureMifareKeysLoaded();
    byte buffer[18];
    byte previousBuffer[18];
    bool readSuccess = false;
    bool previousReadSuccess = false;
    const uint32_t readStart = millis();
    while (millis() - readStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            return;
        }
        if (nfc->readMifareBlock(STOBENE_BLOCCO_CREDITO, buffer) == 0) {
            readSuccess = true;
            break;
        }
        delay(10);
    }

    const uint32_t previousReadStart = millis();
    while (readSuccess && millis() - previousReadStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            return;
        }
        if (nfc->readMifareBlock(STOBENE_BLOCCO_CREDITO_PRECEDENTE, previousBuffer) == 0) {
            previousReadSuccess = true;
            break;
        }
        delay(10);
    }

    if (returnToMenu) { return; }
    if (!readSuccess || !previousReadSuccess) {
        displayError("Sto&Bene tag read failed!");
        delay(2000);
        set_state(SET_CREDIT_MODE);
        return;
    }

    uint16_t currentCredit = (uint16_t)((buffer[1] << 8) | buffer[2]);
    String value = num_keyboard("", 5, "Credit in cents (" + String(currentCredit) + "):");
    if (value == "\x1B") {
        set_state(SET_CREDIT_MODE);
        return;
    }

    long credit = value.toInt();
    if (value.isEmpty() || credit < 100 || credit > 5000) {
        displayError("Invalid credit!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    buffer[1] = (uint8_t)(credit >> 8);
    buffer[2] = (uint8_t)credit;
    previousBuffer[1] = (uint8_t)((credit - 100) >> 8);
    previousBuffer[2] = (uint8_t)(credit - 100);

    display_banner();
    padprintln("Updating Sto&Bene tag...");
    padprintln("");
    if (nfc->writeMifareBlock(STOBENE_BLOCCO_CREDITO_PRECEDENTE, previousBuffer) != 0 ||
        nfc->writeMifareBlock(STOBENE_BLOCCO_CREDITO, buffer) != 0) {
        displayError("Sto&Bene tag write failed!", true);
        set_state(SET_CREDIT_MODE);
        return;
    }

    displaySuccess("Credit set successfully!");
    delay(1000);
    set_state(IDLE_MODE);
}

void StoBene::add_credit_tag() {
    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Sto&Bene tag on the reader.");
    padprintln("");

    bruceConfig.ensureMifareKeysLoaded();
    byte buffer[18];
    byte previousBuffer[18];
    bool readSuccess = false;
    bool previousReadSuccess = false;
    const uint32_t readStart = millis();
    while (millis() - readStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            return;
        }
        if (nfc->readMifareBlock(STOBENE_BLOCCO_CREDITO, buffer) == 0) {
            readSuccess = true;
            break;
        }
        delay(10);
    }

    const uint32_t previousReadStart = millis();
    while (readSuccess && millis() - previousReadStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            return;
        }
        if (nfc->readMifareBlock(STOBENE_BLOCCO_CREDITO_PRECEDENTE, previousBuffer) == 0) {
            previousReadSuccess = true;
            break;
        }
        delay(10);
    }

    if (returnToMenu) { return; }
    if (!readSuccess || !previousReadSuccess) {
        displayError("Sto&Bene tag read failed!");
        delay(2000);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    uint16_t currentCredit = (uint16_t)((buffer[1] << 8) | buffer[2]);
    String value = num_keyboard("", 5, "Add cents (" + String(currentCredit) + "):");
    if (value == "\x1B") {
        set_state(ADD_CREDIT_MODE);
        return;
    }

    long amount = value.toInt();
    long newCredit = (long)currentCredit + amount;
    if (value.isEmpty() || amount < 5 || newCredit < 100 || newCredit > 5000) {
        displayError("Invalid credit!", true);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    buffer[1] = (uint8_t)(newCredit >> 8);
    buffer[2] = (uint8_t)newCredit;
    previousBuffer[1] = (uint8_t)((newCredit - 100) >> 8);
    previousBuffer[2] = (uint8_t)(newCredit - 100);

    display_banner();
    padprintln("Updating Sto&Bene tag...");
    padprintln("");
    if (nfc->writeMifareBlock(STOBENE_BLOCCO_CREDITO_PRECEDENTE, previousBuffer) != 0 ||
        nfc->writeMifareBlock(STOBENE_BLOCCO_CREDITO, buffer) != 0) {
        displayError("Sto&Bene tag write failed!", true);
        set_state(ADD_CREDIT_MODE);
        return;
    }

    displaySuccess("Credit added successfully!");
    delay(1000);
    set_state(IDLE_MODE);
}

void startStoBene() { StoBene stoBene_tool; }
