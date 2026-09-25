/**
 * @file mikai_logic.h
 * @author Luca Moretti
 * @brief
 * @version 0.1
 * @date 2026-09-25
 */
#ifndef MIKAI_LOGIC_H
#define MIKAI_LOGIC_H

#include "pn532_srix.h"

// Funzione                         Descrizione
// mikai_read_tag                   Legge UID + 128 blocchi, calcola chiave
// mikai_is_reset                   true se tag in stato di fabbrica
// mikai_check_lock_id              true se Lock-ID attivo (sola lettura)
// mikai_get_current_credit         Decifra e restituisce credito in centesimi
// mikai_get_info_string            Compone stringa info (UID, Lock ID, credito, SK, data prod, transazioni)
// mikai_add_cents                  Aggiunge centesimi con step 5/10/20/50/100/200
// mikai_set_cents                  Azzera storico e imposta credito esatto
// mikai_import_vendor              Imposta nuovo vendor e ricalcola chiave
// mikai_export_vendor              Copia blocchi vendor (0x18-0x19) in buffer
// mikai_reset_key                  Reset completo del tag (tutti i blocchi 0x10-0x7F)
// mikai_write_modified_blocks      Scrive blocchi modificati sul tag fisico
// mikai_has_pending_writes         true se ci sono blocchi da scrivere
// mikai_export_dump                Copia UID + EEPROM per salvataggio
// mikai_modify_block               Modifica blocco arbitrario (0x10-0x7F)
// mikai_reset_otp                  Reset contatori OTP (blocchi 0x00-0x04)

#define SRIX4K_BLOCKS 128
#define SRIX_BLOCK_LENGTH 4
#define SRIX4K_BYTES (SRIX4K_BLOCKS * SRIX_BLOCK_LENGTH)

struct srix_flag {
    uint32_t memory[4];
};

static inline struct srix_flag srix_flag_init() {
    struct srix_flag f;
    f.memory[0] = f.memory[1] = f.memory[2] = f.memory[3] = 0;
    return f;
}

static inline void srix_flag_add(struct srix_flag *f, uint8_t b) {
    if (b < 128) f->memory[b / 32] |= 1u << (b % 32);
}

static inline void srix_flag_remove(struct srix_flag *f, uint8_t b) {
    if (b < 128) f->memory[b / 32] &= ~(1u << (b % 32));
}

static inline bool srix_flag_get(struct srix_flag *f, uint8_t b) {
    return b < 128 && ((f->memory[b / 32] >> (b % 32)) & 1u);
}

static inline bool srix_flag_isModified(struct srix_flag *f) {
    return (f->memory[0] | f->memory[1] | f->memory[2] | f->memory[3]) > 0;
}

struct srix_t {
    uint8_t eeprom[SRIX4K_BLOCKS][SRIX_BLOCK_LENGTH];
    uint64_t uid;
    struct srix_flag srixFlag;
};

struct mykey_t {
    struct srix_t *srix4k;
    uint32_t encryptionKey;
};

extern Arduino_PN532_SRIX nfcSrix;
extern struct srix_t srix;
extern struct mykey_t srixKey;

bool mikai_read_tag(struct mykey_t *key, Arduino_PN532_SRIX *nfc);
bool mikai_is_reset(struct mykey_t *key);
bool mikai_check_lock_id(struct mykey_t *key);
uint16_t mikai_get_current_credit(struct mykey_t *key);
void mikai_get_info_string(struct mykey_t *key, char *out, size_t outLen);
int mikai_add_cents(struct mykey_t *key, uint16_t cents, uint8_t day, uint8_t month, uint8_t year);
int mikai_set_cents(struct mykey_t *key, uint16_t cents, uint8_t day, uint8_t month, uint8_t year);
void mikai_import_vendor(struct mykey_t *key, const uint8_t block18[4], const uint8_t block19[4]);
int mikai_export_vendor(struct mykey_t *key, uint8_t buffer[8]);
void mikai_reset_key(struct mykey_t *key);
int mikai_write_modified_blocks(struct mykey_t *key, Arduino_PN532_SRIX *nfc);
bool mikai_has_pending_writes(struct mykey_t *key);
void mikai_export_dump(struct mykey_t *key, uint64_t *uid_out, uint8_t eeprom_out[SRIX4K_BYTES]);
void mikai_modify_block(struct mykey_t *key, const uint8_t block[4], uint8_t blockNum);
void mikai_reset_otp(struct mykey_t *key);

#endif // MIKAI_LOGIC_H
