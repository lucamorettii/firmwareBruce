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

#ifndef MIKAI_SRIX_INIT_H
#define MIKAI_SRIX_INIT_H

/**
 * Initialize libnfc and read SRIX4K EEPROM
 * @param target pointer to srix struct
 * @param reader libnfc reader connection string
 * @return negative value if there is an error
 */
int srix_nfc_init(srix target[static 1], char *reader);

/**
 * Read dump from file and initialize srix
 * @param target pointer to srix struct
 * @param eeprom pointer to eeprom array to import
 * @param uid uid value to import
 */
void srix_memory_init(srix target[static 1], uint8_t eeprom[const static SRIX4K_BYTES], uint64_t uid);

/**
 * Reset SRIX4K OTP blocks
 * @param target pointer to srix struct
 * @return negative value if there is an error
 */
int srix_reset_otp(srix target[static 1]);

#endif /* MIKAI_SRIX_INIT_H */
