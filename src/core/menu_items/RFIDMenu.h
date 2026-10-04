/**
 * @file RFIDMenu.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-26
 */
#ifndef __RFID_MENU_H__
#define __RFID_MENU_H__

#include <MenuItemInterface.h>

class RFIDMenu : public MenuItemInterface {
public:
    RFIDMenu() : MenuItemInterface("RFID") {}

    void optionsMenu(void);
    void drawIcon(float scale);
    bool hasTheme() { return bruceConfig.theme.rfid; }
    const String &themePath() override { return bruceConfig.theme.paths.rfid; }

private:
    void configMenu(void);
};

#endif
