/*
 * This file is part of LibMIKAI.
 *
 * LibMIKAI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LibMIKAI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with LibMIKAI.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * @author      Lilz0C <https://telegram.me/Lilz0C>
 * @copyright   2019-2020 Lilz0C <https://telegram.me/Lilz0C>
 * @license     https://opensource.org/licenses/LGPL-3.0 LGPLv3
 */

#include "srix.h"
#include "log.h"
#include "reader.h"
#include "srix_private.h"
#include "srix_flag.h"

/*
 * SRIX4K datasheet
 *
 * Blocks:
 * 0-4 Resettable OTP bits
 * 5-6 Count down counter
 * 7-15 Lockable EEPROM
 * 16-127 EEPROM
 * 255 System OTP bits
 */


/**
 * Get UID from SRIX4K
 * @param target pointer to srix struct to save uid
 */
static inline int srix_get_uid(srix target[static 1]) {
    /* Array to save UID byte response */
    uint8_t uid_bytes[SRIX_UID_LENGTH];

    /* Send command (length = 1) and check length */
    if (nfc_data_exchange(target->reader, (uint8_t[]) {SRIX_GET_UID}, 1, uid_bytes, SRIX_UID_LENGTH) != SRIX_UID_LENGTH) {
        log_error("Invalid UID length.");
        return -1;
    }

    /* Manufacturer code from datasheet (SRIX4K or ST25TB04K) */
    if(uid_bytes[7] != 0xD0 || uid_bytes[6] != 0x02) {
        log_error("Invalid UID header. Different from D0 02.");
        return -2;
    }

    /* Reverse UID bytes and convert to uint64 */
    target->uid = (uint64_t) uid_bytes[7] << 56u | (uint64_t) uid_bytes[6] << 48u | (uint64_t) uid_bytes[5] << 40u |
                  (uint64_t) uid_bytes[4] << 32u | (uint64_t) uid_bytes[3] << 24u | (uint64_t) uid_bytes[2] << 16u |
                  (uint64_t) uid_bytes[1] << 8u | (uint64_t) uid_bytes[0];

    return 0;
}

/**
 * Decrease block 6 counter of toDecrease value
 * @param target pointer to srix struct
 * @param toDecrease value that decrease block 6
 * @return boolean success
 */
static inline bool srix_decrease_block6(srix target[static 1], uint32_t toDecrease) {
    /* Do nothing if toDecrease = 0 */
    if (toDecrease == 0) return true;

    /* Reverse bytes */
    uint32_t block6 = ((uint32_t) target->eeprom[0x06][3] << 24) + ((uint32_t) target->eeprom[0x06][2] << 16) +
                      ((uint32_t) target->eeprom[0x06][1] << 8) + (uint32_t) target->eeprom[0x06][0];

    if (block6 < toDecrease) return false;

    /* If result will be more than 0, subtract. */
    block6 -= toDecrease;

    /* Save result */
    target->eeprom[0x06][0] = block6;
    target->eeprom[0x06][1] = block6 >> 8;
    target->eeprom[0x06][2] = block6 >> 16;
    target->eeprom[0x06][3] = block6 >> 24;

    /* Flag block 6 */
    srix_flag_add(&target->srixFlag, 0x06);

    return true;
}

/**
 * Read a specified block from SRIX4K to rx_data array
 * @param target nfc reader to send command
 * @param rx_data array to save result
 * @param blockNum block to read from srix
 */
static inline void read_block(nfc_device *target, uint8_t rx_data[static 4], const uint8_t blockNum) {
    /* Read while read block length is different than expected */
    while(nfc_data_exchange(target, (uint8_t[]) {SRIX_READ_BLOCK, blockNum}, 2, rx_data, SRIX_BLOCK_LENGTH) != SRIX_BLOCK_LENGTH) {
        /* Show error to the user */
        log_error("The SRIX4K tag has been removed. Reposition it correctly.");
        /* tag reselect */
        nfc_target target_key[MAX_TARGET_COUNT];
        nfc_initiator_select_passive_target(target, nfc_ISO14443B2SR, (void *) 0, 0, target_key);
    }
}

/**
 * Read a specified block from SRIX4K
 * @param target nfc reader to send command
 * @return negative value if there is an error
 */
static inline int srix_read_blocks(srix target[static 1]) {
    if (target->reader == (void *) 0) {
        log_error("NFC reader hasn't been initialized.");
        return -1;
    }

    for (uint8_t i = 0; i < SRIX4K_BLOCKS; i++) {
        read_block(target->reader, target->eeprom[i], i);
    }

    return 0;
}

/**
 * Write block to SRIX4K
 * @param target nfc reader to send command
 * @param blockNum block to write to srix
 */
static inline void write_block(srix target[static 1], const uint8_t blockNum) {
    log_operation("Writing block %02X...\n", blockNum);

    /* While data aren't right, rewrite them. */
    while (true) {
        /* Write data */
        nfc_data_exchange(target->reader, (uint8_t[]) {SRIX_WRITE_BLOCK, blockNum, target->eeprom[blockNum][0], target->eeprom[blockNum][1],
                                                       target->eeprom[blockNum][2], target->eeprom[blockNum][3]}, 6, (void *) 0, 0);

        /* Read to check */
        uint8_t check[4];
        read_block(target->reader, check, blockNum);

        /* Check the read block */
        if (target->eeprom[blockNum][0] == check[0] && target->eeprom[blockNum][1] == check[1] &&
            target->eeprom[blockNum][2] == check[2] && target->eeprom[blockNum][3] == check[3]) {
            /* Success */
            break;
        }

        /* If different, reselect tag */
        log_error("The SRIX4K tag has been removed. Reposition it correctly.");
        nfc_target target_key[MAX_TARGET_COUNT];
        nfc_initiator_select_passive_target(target->reader, nfc_ISO14443B2SR, (void *) 0, 0, target_key);
    }
}


/* Functions used by other files in this library */

int srix_nfc_init(srix target[static 1], char *reader) {
    /* Init srix flag */
    target->srixFlag = srix_flag_init();

    /* Init context */
    if(target->context == (void *) 0) {
        if (nfc_context_init(&target->context) < 0) {
            return -1;
        }
    }

    /* Init reader */
    if(nfc_reader_init(&target->reader, &target->context, reader) < 0) {
        nfc_close_current(&target->reader, &target->context);
        return -2;
    }

    /* Init srix4k */
    if(nfc_srix4k_init(target->reader) < 0) {
        nfc_close_current(&target->reader, &target->context);
        return -3;
    }

    /* Get SRIX4K UID */
    log_operation("Reading UID...\n");
    if(srix_get_uid(target) < 0) {
        nfc_close_current(&target->reader, &target->context);
        return -4;
    }
    log_operation("UID: %" PRIX64 "\n", target->uid);

    /* Read SRIX4K EEPROM */
    log_operation("Reading %u SRIX4K blocks...\n", SRIX4K_BLOCKS);
    srix_read_blocks(target);

    return 0;
}

void srix_memory_init(srix target[static 1], uint8_t eeprom[const static SRIX4K_BYTES], uint64_t uid) {
    /* If memory is already initialized (key id not null), skip read of first 9 blocks */
    if(target->eeprom[0x07][0] != 0x00 && target->eeprom[0x07][1] != 0x00
    && target->eeprom[0x07][2] != 0x00 && target->eeprom[0x07][3] != 0x00) {
        memcpy(target->eeprom[0] + 9*SRIX_BLOCK_LENGTH, eeprom, SRIX4K_BYTES - 9*SRIX_BLOCK_LENGTH);

        /* If already initialized, flag all blocks by default (from 0x09) */
        target->srixFlag.memory[0] = 0xFFFFFE00;
        target->srixFlag.memory[1] = 0xFFFFFFFF;
        target->srixFlag.memory[2] = 0xFFFFFFFF;
        target->srixFlag.memory[3] = 0xFFFFFFFF;
    } else {
        memcpy(target->eeprom[0], eeprom, SRIX4K_BYTES);
    }

    /* Read UID from dump file */
    target->uid = uid;
}

int srix_reset_otp(srix target[static 1]) {
    /* If at least one OTP block is different than 0xFFFFFFFF, reset OTP writing 0xFFFFFFFF. */
    unsigned char reset[5*SRIX_BLOCK_LENGTH];
    memset(reset, 0xFF, sizeof(reset));

    if(memcmp(target->eeprom[0x00], reset, sizeof(reset)) != 0) {
        /* If decrease is done, proceed, else print an error */
        if (srix_decrease_block6(target, 0x00200000)) {
            /* Set 0x00-0x04 blocks to 0xFFFFFFFF */
            memset(target->eeprom[0x00], 0xFF, 5*SRIX_BLOCK_LENGTH);

            /* Flag the blocks */
            srix_flag_add(&target->srixFlag, 0x00);
            srix_flag_add(&target->srixFlag, 0x01);
            srix_flag_add(&target->srixFlag, 0x02);
            srix_flag_add(&target->srixFlag, 0x03);
            srix_flag_add(&target->srixFlag, 0x04);
        } else {
            log_error("Unable to decrease block 0x06 when resetting OTP.\n");
            return -1;
        }
    }

    return 0;
}


/* Public API */

size_t nfc_get_readers_count(mykey key[static 1]) {
    /* Init context */
    if(key->srix4k->context == (void *) 0) {
        if (nfc_context_init(&key->srix4k->context) < 0) {
            return 0;
        }
    }

    nfc_connstring readers[MAX_DEVICE_COUNT];

    /* Search for readers */
    return nfc_list_devices(key->srix4k->context, readers, MAX_DEVICE_COUNT);
}

char *nfc_get_reader(mykey key[static 1], uint8_t reader) {
    /* Init context */
    if(key->srix4k->context == (void *) 0) {
        if (nfc_context_init(&key->srix4k->context) < 0) {
            return 0;
        }
    }

    /* Check if reader will be out of range (with known max length) */
    if(reader >= MAX_DEVICE_COUNT) {
        return (void *) 0;
    }

    static nfc_connstring readers[MAX_DEVICE_COUNT];

    /* Check if reader is out of range (if found readers number is less than index) */
    if(nfc_list_devices(key->srix4k->context, readers, MAX_DEVICE_COUNT) <= reader) {
        return (void *) 0;
    }

    return readers[reader];
}

void srix_export_dump(mykey key[static 1], uint64_t uid[static 1], uint8_t eeprom[static SRIX4K_BYTES]) {
    /* Write eeprom blocks to array pointer param */
    memcpy(eeprom, key->srix4k->eeprom, SRIX4K_BYTES);
    /* Write UID to uid pointer param */
    *uid = key->srix4k->uid;
}

void srix_modify_block(mykey key[static 1], uint8_t block[static 4], const uint8_t blockNum) {
    /* Skip dangerous blocks (OTP + resettable OTP) and set limit to block 127 */
    if (blockNum < 0x10 || blockNum > 0x7F) {
        return;
    }

    key->srix4k->eeprom[blockNum][0] = block[0];
    key->srix4k->eeprom[blockNum][1] = block[1];
    key->srix4k->eeprom[blockNum][2] = block[2];
    key->srix4k->eeprom[blockNum][3] = block[3];
    srix_flag_add(&key->srix4k->srixFlag, blockNum);
}

int srix_write_blocks(mykey key[static 1]) {
    if (key->srix4k->reader == (void *) 0) {
        log_error("NFC reader hasn't been initialized.");
        return -1;
    }

    /* Write block 6 as first block if modified (OTP reset compliant) */
    if (srix_flag_get(&key->srix4k->srixFlag, 0x06)) {
        /* Write block */
        write_block(key->srix4k, 0x06);
        /* Remove flag */
        srix_flag_remove(&key->srix4k->srixFlag, 0x06);
    }

    for (uint8_t i = 0; i < SRIX4K_BLOCKS; i++) {
        /* If current block is flagged (modified), write it. */
        if (srix_flag_get(&key->srix4k->srixFlag, i)) {
            write_block(key->srix4k, i);
        }
    }

    /* Reset flags for next write */
    key->srix4k->srixFlag = srix_flag_init();

    return 0;
}

bool srix_is_modified(mykey key[static 1]) {
    /* If target has been modified and nfc device is initialized, return a positive response. */
    return srix_flag_isModified(&key->srix4k->srixFlag) && key->srix4k->reader != (void *) 0;
}

void srix_print_eeprom(mykey key[static 1]) {
    for (uint8_t i = 0; i < SRIX4K_BLOCKS; i++) {
        log_operation("[%02X] %02X%02X%02X%02X\n", i, key->srix4k->eeprom[i][0], key->srix4k->eeprom[i][1], key->srix4k->eeprom[i][2], key->srix4k->eeprom[i][3]);
    }
}