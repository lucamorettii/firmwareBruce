/**
 * @file microel_logic.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-26
 */
#ifndef MICROEL_LOGIC_H
#define MICROEL_LOGIC_H

#include "modules/rfid/PN532.h"

struct Blocco {
    uint16_t numeroOperazione;
    uint16_t sommaTotaleCredito;
    uint8_t deposito;
    uint16_t credito;
    uint32_t dataTransazione;
    uint16_t puntiFedelta;
    uint16_t importoUltimaOperazione;
    uint8_t checkSum;
};

extern Blocco settore[3]; // settore[0] = B4, settore[1] = B5, settore[2] = B6

bool microel_read_tag(Blocco *settore, PN532 *nfc);
String microel_get_current_credit(Blocco *settore);
String microel_get_keys_string(Blocco *settore);
String microel_get_info_string(Blocco *settore);
bool microel_set_cents(Blocco *settore, uint16_t cents, uint8_t day, uint8_t month, uint8_t year);
bool microel_write_modified_blocks(Blocco *settore, PN532 *nfc);

#endif
