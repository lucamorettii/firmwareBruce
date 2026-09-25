/**
 * @file microel_logic.cpp
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-24
 */
#include "microel_logic.h"

uint8_t uid[8];
uint8_t uidLength;
uint8_t sumHex[6];
uint8_t chiaveA[6];
uint8_t chiaveB[6];
Blocco settore[3]; // settore[0] = B4, settore[1] = B5, settore[2] = B6

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
    if (!nfc->nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 5000)) { return false; }

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
