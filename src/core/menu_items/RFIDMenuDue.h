/**
 * @file RFIDMenuDue.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-26
 */
#ifndef __RFID_MENU_DUE_H__
#define __RFID_MENU_DUE_H__

#include <MenuItemInterface.h>

class RFIDMenuDue : public MenuItemInterface {
public:
    RFIDMenuDue() : MenuItemInterface("RFID 2.0") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return bruceConfig.theme.rfid; }
    const String &themePath() override { return bruceConfig.theme.paths.rfid; }

private:
    void configMenu(void);
};

#endif
