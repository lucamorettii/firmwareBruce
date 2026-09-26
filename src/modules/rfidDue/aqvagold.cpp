/**
 * @file aqvagold.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */
#include "aqvagold.h"
#include "core/bus_HAL.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/sd_functions.h"
#include "core/settings.h"

#define AQVAGOLD_BLOCCO_CREDITO 6

AqvaGold::AqvaGold() {
    current_state = IDLE_MODE;
    setup();
}

AqvaGold::~AqvaGold() {
    delete nfc;
    releaseI2CBus();
}

void AqvaGold::setup() {
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

void AqvaGold::loop() {
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

void AqvaGold::select_state() {
    options = {};

    options.emplace_back("Main Menu", [this]() { set_state(IDLE_MODE); });
    options.emplace_back("Read tag", [this]() { set_state(READ_TAG_MODE); });
    options.emplace_back("Set credit", [this]() { set_state(SET_CREDIT_MODE); });
    options.emplace_back("Add credit", [this]() { set_state(ADD_CREDIT_MODE); });

    loopOptions(options);
}

void AqvaGold::set_state(AqvaGold_State state) {
    current_state = state;
    _screen_drawn = false;
    display_banner();
    delay(300);
}

void AqvaGold::display_banner() {
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

void AqvaGold::show_main_menu() {
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

void AqvaGold::read_tag() {

    if (_screen_drawn) {
        delay(50);
        return;
    }

    display_banner();
    padprintln("Place a Aqvagold tag on the reader.");
    padprintln("");

    /* Lettura tag */
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

void AqvaGold::set_credit_tag() {}

void AqvaGold::add_credit_tag() {}

void startAqvaGold() { AqvaGold aqvagold_tool; }
