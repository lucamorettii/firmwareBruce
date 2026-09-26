#include "RFIDMenuDue.h"
#include "core/display.h"
#include "core/settings.h"
#include "core/utils.h"
#include "modules/rfidDue/YBB.h"
#include "modules/rfidDue/aqvagold.h"
#include "modules/rfidDue/microel.h"
#include "modules/rfidDue/mikai.h"
#include "modules/rfidDue/stoBene.h"

void RFIDMenuDue::optionsMenu() {
    options = {
#if !defined(REMOVE_RFID_HW_INTERFACE)
#ifndef LITE_VERSION
        {"Mikai",    [=]() { startMikai(); }   },
        {"Microel",  [=]() { startMicroel(); } },
        {"YBB",      [=]() { startYBB(); }     },
        {"Sto&Bene", [=]() { startStoBene(); } },
        {"Aqvagold", [=]() { startAqvaGold(); }},
        {"Config",   [=]() { configMenu(); }   },
#endif
#endif
    };

    addOptionToMainMenu();

    vTaskDelay(pdMS_TO_TICKS(200));

    String txt = "RFID";
    if (bruceConfigPins.rfidModule == M5_RFID2_MODULE) txt += " (RFID2)";
#ifdef M5STICK
    else if (bruceConfigPins.rfidModule == PN532_I2C_MODULE) txt += " (PN532-G33)";
    else if (bruceConfigPins.rfidModule == PN532_I2C_SPI_MODULE) txt += " (PN532-G36)";
#else
    else if (bruceConfigPins.rfidModule == PN532_I2C_MODULE) txt += " (PN532-I2C)";
#endif
    else if (bruceConfigPins.rfidModule == PN532_SPI_MODULE) txt += " (PN532-SPI)";
    else if (bruceConfigPins.rfidModule == RC522_SPI_MODULE) txt += " (RC522-SPI)";
#if !defined(LITE_VERSION)
    else if (bruceConfigPins.rfidModule == ST25R3916_SPI_MODULE) txt += " (ST25R-SPI)";
    else if (bruceConfigPins.rfidModule == ST25R3916_I2C_MODULE) txt += " (ST25R-I2C)";
#endif
    loopOptions(options, MENU_TYPE_SUBMENU, txt.c_str());
}

void RFIDMenuDue::configMenu() {
    options = {
#if !defined(REMOVE_RFID_HW_INTERFACE)  // Remove Hardware interface menu due to lack of external GPIO
        {"RFID Module", setRFIDModuleMenu          },
#endif
        {"Add MIF Key", addMifareKeyMenu           },
        {"Back",        [this]() { optionsMenu(); }},
    };

    loopOptions(options, MENU_TYPE_SUBMENU, "RFID Config");
}

void RFIDMenuDue::drawIcon(float scale) {
    clearIconArea();
    int iconSize = scale * 70;
    int iconRadius = scale * 7;
    int deltaRadius = scale * 10;

    if (iconSize % 2 != 0) iconSize++;

    tft.drawRoundRect(
        iconCenterX - iconSize / 2,
        iconCenterY - iconSize / 2,
        iconSize,
        iconSize,
        iconRadius,
        bruceConfig.priColor
    );
    tft.fillRect(iconCenterX - iconSize / 2, iconCenterY, iconSize / 2, iconSize / 2, bruceConfig.bgColor);

    tft.drawCircle(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        iconRadius,
        bruceConfig.priColor
    );

    tft.drawArc(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        2.5 * iconRadius,
        2 * iconRadius,
        180,
        270,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        2.5 * iconRadius + deltaRadius,
        2 * iconRadius + deltaRadius,
        180,
        270,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
    tft.drawArc(
        iconCenterX - iconSize / 2 + deltaRadius,
        iconCenterY + iconSize / 2 - deltaRadius,
        2.5 * iconRadius + 2 * deltaRadius,
        2 * iconRadius + 2 * deltaRadius,
        180,
        270,
        bruceConfig.priColor,
        bruceConfig.bgColor
    );
}
