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

#ifndef MIKAI_MYKEY_H
#define MIKAI_MYKEY_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "srix.h"

/**
 * Init mykey fields (MK, OTP, SK)
 * @param key pointer to mykey struct
 */
void mykey_init(mykey key[static 1]);

/**
 * Return a number greater than 0 if mykey has lock id
 * @param key pointer to mykey struct
 * @return result
 */
int checkLockID(mykey key[static 1]);

/**
 * Check if a mykey is reset (it hasn't an associated vendor code)
 * @param key pointer to mykey struct
 * @return boolean value (true = there isn't a vendor bound)
 */
bool mykey_isReset(mykey key[static 1]);

/**
 * Print info about a mykey
 * @param key pointer to mykey struct
 */
void mykey_print_info(mykey key[static 1]);


/* EEPROM */

/**
 * Import vendor code on the srix4k eeprom memory and recalculate MK/SK
 * @param key pointer to mykey struct
 * @param block18 block18 value as 4 bytes array
 * @param block19 block19 value as 4 bytes array
 */
void mykey_import_vendor(mykey key[static 1], uint8_t block18[const static 4], uint8_t block19[const static 4]);

/**
 * Export vendor code to the buffer (length = 8 byte)
 * @param key pointer to mykey struct
 * @param buffer array to save vendor code data
 * @return If mykey is reset will be returned a negative value
 */
int mykey_export_vendor(mykey key[static 1], uint8_t buffer[static 8]);

/**
 * Reset a mykey (make it virgin)
 * @param key pointer to mykey struct
 */
void mykey_reset(mykey key[static 1]);

/**
 * Reset OTP bytes and recalculate keys with new block 6
 * @param key pointer to mykey struct
 */
void mykey_reset_otp(mykey key[static 1]);


/* credit */

/**
 * Return current credit from block 21
 * @return cents of credit
 */
uint16_t mykey_get_current_credit(mykey key[static 1]);

/**
 * Add n cents to mykey
 * @param key pointer to mykey data struct
 * @param cents cents to charge (add)
 * @param day today number (0-31)
 * @param month this month (1-12)
 * @param year this year from 2000 (0-127)
 * @return negative value if there is an error
 */
int mykey_add_cents(mykey key[static 1], uint16_t cents, uint8_t day, uint8_t month, uint8_t year);

/**
* Reset credit history and charge n cents
* @param key pointer to mykey data struct
* @param cents cents to write
* @param day today number (0-31)
* @param month this month (1-12)
* @param year this year from 2000 (0-127)
* @return negative value if there is an error
*/
int mykey_set_cents(mykey key[static 1], uint16_t cents, uint8_t day, uint8_t month, uint8_t year);

#endif /* MIKAI_MYKEY_H */