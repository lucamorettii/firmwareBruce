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

#ifndef MIKAI_LIBRARY_H
#define MIKAI_LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "srix.h"
#include "mykey.h"

#define MIKAI_VERSION "X3"

/**
 * Allocate and initialize memory for a mykey struct. Chip id of an empty mykey is always 00 00 00 00.
 * @return pointer to mykey struct
 */
mykey *mykey_new();

/**
 * Initialize a mykey struct from NFC header
 * @param target pointer to mykey struct. If it isn't null, no space will be allocated in the heap but it is understood that it has already been initialized.
 * @param reader connection string from nfc_get_readers() or a manual reader
 * @return pointer received in input to mykey struct. If there is an error, target and returned pointer will be null.
 */
mykey *mykey_nfc_init(mykey **target, char *reader);

/**
 * Initialize a mykey struct from the memory (could be a dump file).
 * @param target pointer to mykey struct. If it isn't null, no space will be allocated in the heap but it is understood that it has already been initialized.
 * @param dump array that contains srix4k eeprom bytes.
 * @param uid srix4k uid as uint64.
 * @return pointer received in input to mykey struct. If there is an error, target and returned pointer will be null.
 */
mykey *mykey_memory_init(mykey **target, uint8_t dump[const static SRIX4K_BYTES], uint64_t uid);

/**
 * Free memory of an allocated mykey struct (to prevent memory leak).
 * @param key pointer to mykey struct to deallocate.
 */
void mykey_delete(mykey *key);

/**
 * Get current version of mikai as string
 * @return version as string
 */
const char *mikai_version();

#endif /* MIKAI_LIBRARY_H */