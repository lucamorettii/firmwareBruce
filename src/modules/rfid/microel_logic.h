// #ifndef MICROEL_H
// #define MICROEL_H

// #include "drivers/pn532/pn532.h"
// #include "components/popup.h"
// #include "components/menu.h"
// #include "components/menu_touch.h"

// struct DumpMicroel {
//     uint16_t numeroOperazione;
//     uint16_t sommaTotaleCredito;
//     uint8_t deposito;
//     uint16_t credito;
//     uint32_t dataTransazione;
//     uint16_t puntiFedelta;
//     uint16_t importoUltimaOperazione;
//     uint8_t checkSum;
// };

// uint8_t calcolaChecksum(uint8_t dati[16]);
// void costruisciBlocco(DumpMicroel &in, uint8_t out[16]);
// void impostaCredito(uint16_t nuovoCredito);
// void microelWrite();
// void microelRead();
// void generaChiaveA();
// void generaChiaveB();
// void decodificaBlocco(DumpMicroel &dump, uint8_t blocco[16]);
// String creditoAttuale();
// String creditoPrecedente();

// #endif
