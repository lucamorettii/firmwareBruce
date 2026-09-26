/**
 * @file stoBene.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */
#include "stoBene.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/settings.h"

#define STO_BENE_BLOCCO_CREDITO_PRECEDENTE 50
#define STO_BENE_BLOCCO_CREDITO 54

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
    padprintln("- Read Sto&Bene tag");
    padprintln("- Set credit on Sto&Bene tag");
    padprintln("- Add credit to Sto&Bene tag");
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

    /* Lettura tag */
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

    byte buffer[18];
    while (true) {
        if (check(EscPress)) {
            returnToMenu = true;
            break;
        }
        /* TODO: if con lettura */
        delay(200);
    }

    if (returnToMenu) {
        _screen_drawn = true;
        return;
    }

    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    /* Stampa UID, Tipo e Credito */
    // padprintln("UID: " + nfc->printableUID.uid);
    // padprintln("Tipo: " + nfc->printableUID.picc_type);

    // uint16_t credit = (uint16_t)((buffer[1] << 8) | buffer[2]);
    // uint16_t euro = credit / 100;
    // uint16_t cent = credit % 100;
    // String creditoStr = String(euro) + ".";
    // if (cent < 10) creditoStr += "0";
    // creditoStr += String(cent) + " euro";
    // padprintln("Credito: " + creditoStr);
    padprintln("");

    tft.setTextColor(getColorVariation(bruceConfig.priColor), bruceConfig.bgColor);
    padprintln("Press [OK] for Main Menu");
    tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);

    _screen_drawn = true;
}

void StoBene::set_credit_tag() {}

void StoBene::add_credit_tag() {}

void startStoBene() { StoBene stoBene_tool; }
