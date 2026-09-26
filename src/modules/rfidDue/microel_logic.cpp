/**
 * @file microel_logic.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-26
 */
#include "microel_logic.h"
#include "core/mykeyboard.h"

uint8_t uid[8];
uint8_t uidLength;
uint8_t sumHex[6];
uint8_t chiaveA[6];
uint8_t chiaveB[6];
Blocco settore[3]; // settore[0] = B4, settore[1] = B5, settore[2] = B6

uint8_t calcolaChecksum(uint8_t dati[16]) {
    uint16_t somma = 0x21;
    for (int i = 0; i < 15; i++) somma += dati[i];
    return (uint8_t)(somma % 256);
}

void microel_encode_block(const Blocco &blocco, uint8_t out[16]) {
    out[0] = (uint8_t)(blocco.numeroOperazione & 0xFF);
    out[1] = (uint8_t)(blocco.numeroOperazione >> 8);
    out[2] = (uint8_t)(blocco.sommaTotaleCredito & 0xFF);
    out[3] = (uint8_t)(blocco.sommaTotaleCredito >> 8);
    out[4] = blocco.deposito;
    out[5] = (uint8_t)(blocco.credito & 0xFF);
    out[6] = (uint8_t)(blocco.credito >> 8);
    out[7] = (uint8_t)(blocco.dataTransazione & 0xFF);
    out[8] = (uint8_t)((blocco.dataTransazione >> 8) & 0xFF);
    out[9] = (uint8_t)((blocco.dataTransazione >> 16) & 0xFF);
    out[10] = (uint8_t)((blocco.dataTransazione >> 24) & 0xFF);
    out[11] = (uint8_t)(blocco.puntiFedelta & 0xFF);
    out[12] = (uint8_t)(blocco.puntiFedelta >> 8);
    out[13] = (uint8_t)(blocco.importoUltimaOperazione & 0xFF);
    out[14] = (uint8_t)(blocco.importoUltimaOperazione >> 8);
    out[15] = calcolaChecksum(out);
}

void calcolaSumHex() {
    // Chiave XOR fissa del protocollo Microel
    const uint8_t xorKey[6] = {0x01, 0x92, 0xA7, 0x75, 0x2B, 0xF9};

    // Somma i byte dell'UID e applica mod 256
    int somma = 0;
    for (size_t i = 0; i < uidLength; i++) somma += uid[i];
    int val = somma % 256;

    // Il protocollo richiede un valore pari; se dispari incrementa di 2
    if (val % 2 == 1) val += 2;

    // XOR con la chiave fissa per ottenere i 6 byte di sumHex
    for (size_t i = 0; i < 6; i++) sumHex[i] = (uint8_t)(val ^ xorKey[i]);
}

void generaChiaveA() {
    calcolaSumHex();

    // Il nibble alto del primo byte determina il secondo XOR
    uint8_t primoNibble = (sumHex[0] >> 4) & 0x0F;

    if (primoNibble == 0x2 || primoNibble == 0x3 || primoNibble == 0xA || primoNibble == 0xB) {
        // Variante 1: secondo XOR con 0x40
        for (size_t i = 0; i < 6; i++) chiaveA[i] = 0x40 ^ sumHex[i];
    } else if (primoNibble == 0x6 || primoNibble == 0x7 || primoNibble == 0xE || primoNibble == 0xF) {
        // Variante 2: secondo XOR con 0xC0
        for (size_t i = 0; i < 6; i++) chiaveA[i] = 0xC0 ^ sumHex[i];
    } else {
        // Variante 3: Key A coincide con sumHex (nessun XOR aggiuntivo)
        for (size_t i = 0; i < 6; i++) chiaveA[i] = sumHex[i];
    }
}

void generaChiaveB() {
    // Key B = NOT bit a bit di Key A → chiaveA XOR chiaveB = 0xFF×6
    for (size_t i = 0; i < 6; i++) chiaveB[i] = 0xFF ^ chiaveA[i];
}

void decodificaBlocco(Blocco &out, const uint8_t dati[16]) {
    out.numeroOperazione = (uint16_t)dati[0] | ((uint16_t)dati[1] << 8);
    out.sommaTotaleCredito = (uint16_t)dati[2] | ((uint16_t)dati[3] << 8);
    out.deposito = dati[4];
    out.credito = (uint16_t)dati[5] | ((uint16_t)dati[6] << 8);
    out.dataTransazione =
        (uint32_t)dati[7] | ((uint32_t)dati[8] << 8) | ((uint32_t)dati[9] << 16) | ((uint32_t)dati[10] << 24);
    out.puntiFedelta = (uint16_t)dati[11] | ((uint16_t)dati[12] << 8);
    out.importoUltimaOperazione = (uint16_t)dati[13] | ((uint16_t)dati[14] << 8);
    out.checkSum = dati[15];
}

bool microel_read_tag(Blocco *settore, PN532 *nfc) {
    if (nfc == nullptr || settore == nullptr) {
        uidLength = 0;
        return false;
    }

    uidLength = 0;
    const uint32_t readStart = millis();
    while (millis() - readStart < 5000) {
        if (check(EscPress)) {
            returnToMenu = true;
            return false;
        }

        if (nfc->nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100)) break;
        delay(10);
    }

    if (uidLength == 0) return false;

    if (uidLength == 0 || uidLength > sizeof(uid)) {
        uidLength = 0;
        return false;
    }

    generaChiaveA();
    generaChiaveB();

    bool autenticato = nfc->nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, chiaveA);
    if (!autenticato) {
        autenticato = nfc->nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 1, chiaveB);
    }
    if (!autenticato) return false;

    uint8_t primoBlocco = 4;
    uint8_t dati[16];
    for (uint8_t indice = 0; indice < 3; ++indice) {
        if (!nfc->nfc.mifareclassic_ReadDataBlock(primoBlocco + indice, dati)) { return false; }
        decodificaBlocco(settore[indice], dati);
    }

    return true;
}

String microel_get_current_credit(Blocco *settore) {
    if (settore == nullptr) return "No data available";

    const uint16_t centesimi = settore[0].credito;
    String credito = String(centesimi / 100) + ".";
    if (centesimi % 100 < 10) credito += "0";
    credito += String(centesimi % 100) + " euro";
    return credito;
}

String microel_get_info_string(Blocco *settore) {
    if (settore == nullptr) return "No data available";

    String uidString;
    for (uint8_t i = 0; i < uidLength; ++i) {
        if (uid[i] < 0x10) uidString += "0";
        uidString += String(uid[i], HEX);
        if (i + 1 < uidLength) uidString += " ";
    }
    uidString.toUpperCase();

    String keyAString;
    String keyBString;
    for (uint8_t i = 0; i < 6; ++i) {
        if (chiaveA[i] < 0x10) keyAString += "0";
        keyAString += String(chiaveA[i], HEX);
        if (i + 1 < 6) keyAString += " ";

        if (chiaveB[i] < 0x10) keyBString += "0";
        keyBString += String(chiaveB[i], HEX);
        if (i + 1 < 6) keyBString += " ";
    }
    keyAString.toUpperCase();
    keyBString.toUpperCase();

    return "UID (" + String(uidLength) + " byte): " + uidString + "\nKey A: " + keyAString +
           "\nKey B: " + keyBString + "\nCredito corrente: " + microel_get_current_credit(settore) +
           "\nCredito precedente: " + String(settore[1].credito / 100) + "." +
           (settore[1].credito % 100 < 10 ? "0" : "") + String(settore[1].credito % 100) + " euro";
}

String microel_get_keys_string(Blocco *settore) {
    if (settore == nullptr) return "No data available";

    String uidString;
    for (uint8_t i = 0; i < uidLength; ++i) {
        if (uid[i] < 0x10) uidString += "0";
        uidString += String(uid[i], HEX);
        if (i + 1 < uidLength) uidString += " ";
    }
    uidString.toUpperCase();

    String keyAString;
    String keyBString;
    for (uint8_t i = 0; i < 6; ++i) {
        if (chiaveA[i] < 0x10) keyAString += "0";
        keyAString += String(chiaveA[i], HEX);
        if (i + 1 < 6) keyAString += " ";

        if (chiaveB[i] < 0x10) keyBString += "0";
        keyBString += String(chiaveB[i], HEX);
        if (i + 1 < 6) keyBString += " ";
    }
    keyAString.toUpperCase();
    keyBString.toUpperCase();

    return "UID (" + String(uidLength) + " byte): " + uidString + "\nKey A: " + keyAString +
           "\nKey B: " + keyBString;
}

bool microel_set_cents(Blocco *settore, uint16_t cents, uint8_t day, uint8_t month, uint8_t year) {
    if (settore == nullptr) return false;
    if (cents < 200 || cents > 5000) return false;

    settore[0].numeroOperazione = 51;
    settore[0].sommaTotaleCredito = 2000 + cents;
    settore[0].credito = cents;
    settore[0].dataTransazione = (uint32_t)day | ((uint32_t)month << 8) | ((uint32_t)year << 16);
    settore[0].importoUltimaOperazione = cents;

    settore[1].numeroOperazione = 50;
    settore[1].sommaTotaleCredito = 2000;
    settore[1].credito = cents - 100;
    settore[1].dataTransazione = (uint32_t)day | ((uint32_t)month << 8) | ((uint32_t)year << 16);
    settore[1].importoUltimaOperazione = cents - 200;

    settore[2] = settore[0];

    return true;
}

bool microel_write_modified_blocks(Blocco *settore, PN532 *nfc) {
    if (nfc == nullptr || settore == nullptr) return false;

    uint8_t bloccoDati[16];
    uint8_t blocchi[] = {4, 5, 6};

    for (int i = 0; i < 3; i++) {
        microel_encode_block(settore[i], bloccoDati);
        if (!nfc->nfc.mifareclassic_WriteDataBlock(blocchi[i], bloccoDati)) return false;
    }

    return true;
}
