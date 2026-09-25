// #include "microel.h"

// // Define
// #define LUNGHEZZA_CHIAVE 6 // Lunghezza in byte di Key A e Key B
// // Variabili
// uint8_t uid[7];
// uint8_t uidLength;
// DumpMicroel bloccoQuattro;
// DumpMicroel bloccoCinque;
// uint8_t sumHex[LUNGHEZZA_CHIAVE];
// uint8_t chiaveA[LUNGHEZZA_CHIAVE];
// uint8_t chiaveB[LUNGHEZZA_CHIAVE];

// uint8_t calcolaChecksum(uint8_t dati[16]) {
//     uint16_t somma = 0x21;
//     for (int i = 0; i < 15; i++) somma += dati[i];
//     return (uint8_t)(somma % 256);
// }

// void costruisciBlocco(DumpMicroel &in, uint8_t out[16]) {
//     out[0] = (uint8_t)(in.numeroOperazione & 0xFF);
//     out[1] = (uint8_t)(in.numeroOperazione >> 8);
//     out[2] = (uint8_t)(in.sommaTotaleCredito & 0xFF);
//     out[3] = (uint8_t)(in.sommaTotaleCredito >> 8);
//     out[4] = in.deposito;
//     out[5] = (uint8_t)(in.credito & 0xFF);
//     out[6] = (uint8_t)(in.credito >> 8);
//     out[7] = (uint8_t)(in.dataTransazione & 0xFF);
//     out[8] = (uint8_t)((in.dataTransazione >> 8) & 0xFF);
//     out[9] = (uint8_t)((in.dataTransazione >> 16) & 0xFF);
//     out[10] = (uint8_t)((in.dataTransazione >> 24) & 0xFF);
//     out[11] = (uint8_t)(in.puntiFedelta & 0xFF);
//     out[12] = (uint8_t)(in.puntiFedelta >> 8);
//     out[13] = (uint8_t)(in.importoUltimaOperazione & 0xFF);
//     out[14] = (uint8_t)(in.importoUltimaOperazione >> 8);
//     out[15] = calcolaChecksum(out);
// }

// void impostaCredito(uint16_t nuovoCredito) {
//     const uint32_t dataTransazione = 0xD25E501A;

//     uint16_t importo =
//         (nuovoCredito > bloccoQuattro.credito) ? (nuovoCredito - bloccoQuattro.credito) : nuovoCredito;

//     // Blocco 5: copia dello stato attuale (diventa il "precedente")
//     bloccoCinque = bloccoQuattro;

//     // Blocco 4: nuovo stato
//     bloccoQuattro.numeroOperazione++;
//     bloccoQuattro.sommaTotaleCredito += importo;
//     bloccoQuattro.credito = nuovoCredito;
//     bloccoQuattro.dataTransazione = dataTransazione;
//     bloccoQuattro.importoUltimaOperazione = importo;
// }

// void microelWrite() {
//     String info[6];
//     drawPopup("Avvicina il tag al lettore...", info, 0);

//     bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 5000);

//     if (!success) {
//         drawPopup("Nessun tag rilevato", info, 0);
//         return;
//     }

//     String uidStr = "";
//     for (uint8_t i = 0; i < uidLength; i++) {
//         if (uid[i] < 0x10) uidStr += "0";

//         uidStr += String(uid[i], HEX);

//         if (i < uidLength - 1) uidStr += " ";
//     }
//     uidStr.toUpperCase();

//     generaChiaveA();
//     generaChiaveB();

//     String keyAStr = "";
//     String keyBStr = "";

//     for (uint8_t i = 0; i < LUNGHEZZA_CHIAVE; i++) {

//         if (chiaveA[i] < 0x10) keyAStr += "0";
//         keyAStr += String(chiaveA[i], HEX);

//         if (i < LUNGHEZZA_CHIAVE - 1) keyAStr += " ";

//         if (chiaveB[i] < 0x10) keyBStr += "0";
//         keyBStr += String(chiaveB[i], HEX);

//         if (i < LUNGHEZZA_CHIAVE - 1) keyBStr += " ";
//     }

//     keyAStr.toUpperCase();
//     keyBStr.toUpperCase();

//     uint8_t bloccoTmp[16];

//     bool authOk = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, chiaveA);

//     if (!authOk) {
//         success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 2500);

//         if (success) { authOk = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 1, chiaveB); }
//     }

//     if (!authOk) {
//         drawPopup("Autenticazione fallita", info, 0);
//         return;
//     }

//     nfc.mifareclassic_ReadDataBlock(4, bloccoTmp);
//     decodificaBlocco(bloccoQuattro, bloccoTmp);

//     nfc.mifareclassic_ReadDataBlock(5, bloccoTmp);
//     decodificaBlocco(bloccoCinque, bloccoTmp);

//     // Mostra il credito attuale prima di scegliere
//     drawPopup("Credito attuale " + creditoAttuale(), info, 0);
//     delay(2500);

//     // Menu scelta credito
//     int dimArrayCredito = 9;
//     String sceltaCredito[dimArrayCredito] = {
//         "5.00 euro",
//         "10.00 euro",
//         "15.00 euro",
//         "20.00 euro",
//         "25.00 euro",
//         "30.00 euro",
//         "35.00 euro",
//         "40.00 euro",
//         "Indietro"
//     };
//     uint16_t valoriCredito[dimArrayCredito] = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 0};
//     Menu credito("Scegli credito", dimArrayCredito, sceltaCredito, nullptr, nullptr);
//     credito.drawMenu();

//     delay(100);
//     while (ts.getPoint().z >= 300) delay(10);

//     uint16_t creditoSelezionato = 0;
//     bool selezionato = false;
//     bool lastTouch = false;

//     while (!selezionato) {
//         TS_Point p = ts.getPoint();
//         bool touched = (p.z >= 300);

//         if (touched && !lastTouch) {
//             int tx = menuTouchX(p.x, p.y);
//             int ty = menuTouchY(p.x, p.y);

//             if (!isMenuButtonTouch(tx, ty)) {
//                 lastTouch = touched;
//                 delay(50);
//                 continue;
//             }

//             if (ty < 109) {
//                 credito.prev();
//             } else if (ty >= 213) {
//                 credito.next();
//             } else {
//                 if (sceltaCredito[credito.getIndex()] == "Indietro") { return; }
//                 creditoSelezionato = valoriCredito[credito.getIndex()];
//                 selezionato = true;
//             }

//             credito.drawMenu();
//         }
//         lastTouch = touched;
//         delay(50);
//     }

//     // Debug
//     // Serial.print("Credito selezionato: ");
//     // Serial.print(creditoSelezionato / 100);
//     // Serial.print(".");
//     // if (creditoSelezionato % 100 < 10)
//     //     Serial.print("0");
//     // Serial.print(creditoSelezionato % 100);
//     // Serial.println(" euro");

//     // Blocco 4 = credito nuovo (corrente)
//     // Blocco 5 = credito precedente (copia di B4 prima della modifica)
//     // Blocco 6 = copia identica del blocco 4

//     impostaCredito(creditoSelezionato);

//     drawPopup("Scrittura in corso...", info, 0);

//     // Riseleziona il tag (l'autenticazione è persa dopo il menu)
//     uint8_t reUid[7];
//     uint8_t reUidLength;
//     success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, reUid, &reUidLength, 5000);
//     if (!success) {
//         drawPopup("Nessun tag rilevato", info, 0);
//         return;
//     }

//     // Verifica che sia lo stesso tag
//     if (reUidLength != uidLength || memcmp(reUid, uid, uidLength) != 0) {
//         drawPopup("Tag diverso, operazione annullata", info, 0);
//         return;
//     }

//     // Riautentica per il settore 1 (blocchi 4-7) con le chiavi già generate
//     bool writeAuthOk = nfc.mifareclassic_AuthenticateBlock(reUid, reUidLength, 4, 0, chiaveA);
//     if (!writeAuthOk) {
//         writeAuthOk = nfc.mifareclassic_AuthenticateBlock(reUid, reUidLength, 4, 1, chiaveB);
//     }

//     if (!writeAuthOk) {
//         drawPopup("Autenticazione fallita per scrittura", info, 0);
//         return;
//     }

//     // Scrivi blocco 4 (credito corrente)
//     uint8_t bloccoWrite[16];
//     costruisciBlocco(bloccoQuattro, bloccoWrite);
//     if (!nfc.mifareclassic_WriteDataBlock(4, bloccoWrite)) {
//         drawPopup("Errore scrittura blocco 4", info, 0);
//         // Serial.println("Errore scrittura blocco 4");
//         return;
//     }

//     // Scrivi blocco 5 (credito precedente)
//     costruisciBlocco(bloccoCinque, bloccoWrite);
//     if (!nfc.mifareclassic_WriteDataBlock(5, bloccoWrite)) {
//         drawPopup("Errore scrittura blocco 5", info, 0);
//         // Serial.println("Errore scrittura blocco 5");
//         return;
//     }

//     // Scrivi blocco 6 (copia di blocco 4)
//     costruisciBlocco(bloccoQuattro, bloccoWrite);
//     if (!nfc.mifareclassic_WriteDataBlock(6, bloccoWrite)) {
//         drawPopup("Errore scrittura blocco 6", info, 0);
//         // Serial.println("Errore scrittura blocco 6");
//         return;
//     }

//     info[0] = "UID: " + uidStr;
//     info[1] = "Lunghezza: " + String(uidLength) + " byte";
//     info[2] = "Key A: " + keyAStr;
//     info[3] = "Key B: " + keyBStr;
//     info[4] = "Credito attuale: " + creditoAttuale();
//     info[5] = "Credito precedente: " + creditoPrecedente();

//     drawPopup("Scrittura completata", info, 6);
// }

// void microelRead() {
//     String info[6];
//     drawPopup("Avvicina il tag al lettore...", info, 0);

//     bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 5000);

//     if (!success) {
//         drawPopup("Nessun tag rilevato", info, 0);
//         return;
//     }

//     String uidStr = "";
//     for (uint8_t i = 0; i < uidLength; i++) {
//         if (uid[i] < 0x10) uidStr += "0";

//         uidStr += String(uid[i], HEX);

//         if (i < uidLength - 1) uidStr += " ";
//     }
//     uidStr.toUpperCase();

//     generaChiaveA();
//     generaChiaveB();

//     String keyAStr = "";
//     String keyBStr = "";

//     for (uint8_t i = 0; i < LUNGHEZZA_CHIAVE; i++) {

//         if (chiaveA[i] < 0x10) keyAStr += "0";
//         keyAStr += String(chiaveA[i], HEX);

//         if (i < LUNGHEZZA_CHIAVE - 1) keyAStr += " ";

//         if (chiaveB[i] < 0x10) keyBStr += "0";
//         keyBStr += String(chiaveB[i], HEX);

//         if (i < LUNGHEZZA_CHIAVE - 1) keyBStr += " ";
//     }

//     keyAStr.toUpperCase();
//     keyBStr.toUpperCase();

//     uint8_t bloccoTmp[16];

//     bool authOk = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, chiaveA);

//     if (!authOk) {
//         success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 2500);

//         if (success) { authOk = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 1, chiaveB); }
//     }

//     if (!authOk) {
//         drawPopup("Autenticazione fallita", info, 0);
//         return;
//     }

//     nfc.mifareclassic_ReadDataBlock(4, bloccoTmp);
//     decodificaBlocco(bloccoQuattro, bloccoTmp);

//     nfc.mifareclassic_ReadDataBlock(5, bloccoTmp);
//     decodificaBlocco(bloccoCinque, bloccoTmp);

//     info[0] = "UID: " + uidStr;
//     info[1] = "Lunghezza: " + String(uidLength) + " byte";
//     info[2] = "Key A: " + keyAStr;
//     info[3] = "Key B: " + keyBStr;
//     info[4] = "Credito attuale: " + creditoAttuale();
//     info[5] = "Credito precedente: " + creditoPrecedente();

//     drawPopup("Informazioni", info, 6);
// }

// void calcolaSumHex() {
//     // Chiave XOR fissa del protocollo Microel
//     const uint8_t xorKey[LUNGHEZZA_CHIAVE] = {0x01, 0x92, 0xA7, 0x75, 0x2B, 0xF9};

//     // Somma i byte dell'UID e applica mod 256
//     int somma = 0;
//     for (size_t i = 0; i < uidLength; i++) somma += uid[i];
//     int val = somma % 256;

//     // Il protocollo richiede un valore pari; se dispari incrementa di 2
//     if (val % 2 == 1) val += 2;

//     // XOR con la chiave fissa per ottenere i 6 byte di sumHex
//     for (size_t i = 0; i < LUNGHEZZA_CHIAVE; i++) sumHex[i] = (uint8_t)(val ^ xorKey[i]);
// }

// void generaChiaveA() {
//     calcolaSumHex();

//     // Il nibble alto del primo byte determina il secondo XOR
//     uint8_t primoNibble = (sumHex[0] >> 4) & 0x0F;

//     if (primoNibble == 0x2 || primoNibble == 0x3 || primoNibble == 0xA || primoNibble == 0xB) {
//         // Variante 1: secondo XOR con 0x40
//         for (size_t i = 0; i < LUNGHEZZA_CHIAVE; i++) chiaveA[i] = 0x40 ^ sumHex[i];
//     } else if (primoNibble == 0x6 || primoNibble == 0x7 || primoNibble == 0xE || primoNibble == 0xF) {
//         // Variante 2: secondo XOR con 0xC0
//         for (size_t i = 0; i < LUNGHEZZA_CHIAVE; i++) chiaveA[i] = 0xC0 ^ sumHex[i];
//     } else {
//         // Variante 3: Key A coincide con sumHex (nessun XOR aggiuntivo)
//         for (size_t i = 0; i < LUNGHEZZA_CHIAVE; i++) chiaveA[i] = sumHex[i];
//     }
// }

// void generaChiaveB() {
//     // Key B = NOT bit a bit di Key A → chiaveA XOR chiaveB = 0xFF×6
//     for (size_t i = 0; i < LUNGHEZZA_CHIAVE; i++) chiaveB[i] = 0xFF ^ chiaveA[i];
// }

// void decodificaBlocco(DumpMicroel &dump, uint8_t blocco[16]) {
//     dump.numeroOperazione = (uint16_t)(blocco[0] | (blocco[1] << 8));
//     dump.sommaTotaleCredito = (uint16_t)(blocco[2] | (blocco[3] << 8));
//     dump.deposito = blocco[4];
//     dump.credito = (uint16_t)(blocco[5] | (blocco[6] << 8));
//     dump.dataTransazione = (uint32_t)(blocco[7] | (blocco[8] << 8) | (blocco[9] << 16) | (blocco[10] <<
//     24)); dump.puntiFedelta = (uint16_t)(blocco[11] | (blocco[12] << 8)); dump.importoUltimaOperazione =
//     (uint16_t)(blocco[13] | (blocco[14] << 8)); dump.checkSum = blocco[15];
// }

// String creditoAttuale() {
//     uint16_t cents = bloccoQuattro.credito;
//     uint16_t euro = cents / 100;
//     uint8_t cent = cents % 100;
//     String res = String(euro) + ".";
//     if (cent < 10) res += "0";
//     res += String(cent) + " euro";
//     return res;
// }

// String creditoPrecedente() {
//     uint16_t cents = bloccoCinque.credito;
//     uint16_t euro = cents / 100;
//     uint8_t cent = cents % 100;
//     String res = String(euro) + ".";
//     if (cent < 10) res += "0";
//     res += String(cent) + " euro";
//     return res;
// }
