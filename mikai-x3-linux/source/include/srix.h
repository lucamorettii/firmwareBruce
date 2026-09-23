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

#ifndef MIKAI_SRIX_H
#define MIKAI_SRIX_H

#include <inttypes.h>
#include <nfc/nfc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "flag.h"

/**
 * Struct that represent a generic SRIX4K tag
 */
struct srix {
    uint64_t uid;
    uint8_t eeprom[128][4];
    struct srix_flag srixFlag;
    nfc_context *context;
    nfc_device *reader;
};
typedef struct srix srix;

/**
 * Struct that represents a mykey
 */
struct mykey {
    uint32_t encryptionKey;
    srix *srix4k;
};
typedef struct mykey mykey;

/* SRIX4K EEPROM */
#define SRIX4K_BLOCKS     128
#define SRIX4K_BYTES      512

/* SRIX4K Commands */
#define SRIX_GET_UID      0x0B
#define SRIX_READ_BLOCK   0x08
#define SRIX_WRITE_BLOCK  0x09

/* SRIX4K Constants */
#define SRIX_UID_LENGTH   8
#define SRIX_BLOCK_LENGTH 4

/**
 * Function that search for available readers and return number of available readers.
 * @param key pointer to mykey struct
 * @return number of readers found. zero if there is an error or there aren't readers.
 */
size_t nfc_get_readers_count(mykey key[static 1]);

/**
 * Function that return specified nfc reader connection string
 * @param reader index of reader (0 = first, 1 = second, ecc.)
 * @return reader connstring at specified index. NULL if index is invalid.
 */
char *nfc_get_reader(mykey key[static 1], uint8_t reader);

/**
 * Save eeprom bytes and uid to variable pointers as parameters
 * @param target pointer to mykey struct
 * @param uid pointer to uid variable
 * @param eeprom pointer to eeprom array to save bytes
 */
void srix_export_dump(mykey key[static 1], uint64_t uid[static 1], uint8_t eeprom[static SRIX4K_BYTES]);

/**
 * Modify manually a block between 16 and 127 (dangerous)
 * @param target pointer to mykey struct
 * @param blockNum block to write
 * @param block value to write
 */
void srix_modify_block(mykey key[static 1], uint8_t block[static 4], const uint8_t blockNum);

/**
 * Write all modified blocks of target to SRIX4K
 * @param target pointer to mykey struct
 * @return negative value if there is an error
 */
int srix_write_blocks(mykey key[static 1]);

/**
 * Check if eeprom has been modified
 * @param target pointer to mykey struct
 * @return Boolean response, True if nfc is initialized and eeprom has been modified.
 */
bool srix_is_modified(mykey key[static 1]);

/**
 * Print content of srix eeprom
 * @param target pointer to mykey struct
 */
void srix_print_eeprom(mykey key[static 1]);

#endif /* MIKAI_SRIX_H */