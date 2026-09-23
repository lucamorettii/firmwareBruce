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

#ifndef MIKAI_READER_H
#define MIKAI_READER_H

#include <inttypes.h>
#include <nfc/nfc.h>
#include <stdint.h>
#include <stdlib.h>

/* libnfc settings */
#define SRIX_TIMEOUT_MS  0
#define MAX_DEVICE_COUNT 8
#define MAX_TARGET_COUNT 1

/* Modulation constant structs */
static const nfc_modulation nfc_ISO14443B = {
        .nmt = NMT_ISO14443B,
        .nbr = NBR_106,
};

static const nfc_modulation nfc_ISO14443B2SR = {
        .nmt = NMT_ISO14443B2SR,
        .nbr = NBR_106,
};


/* libnfc initialization functions (used only in srix.c) */

/**
 * Initialize libnfc context.
 * @param context pointer to save initialized context pointer
 * @return negative int on failure
 */
int nfc_context_init(nfc_context **context);

/**
 * Initialize a device as libnfc reader
 * @param reader pointer to a null libnfc device pointer to save initialized device
 * @param context pointer to a libnfc context pointer
 * @param target string that represent a found reader
 * @return negative int on failure
 */
int nfc_reader_init(nfc_device **reader, nfc_context **context, nfc_connstring target);

/**
 * Polling to search a valid SRIX4K tag to initialize
 * @param reader pointer to a libnfc device
 * @return negative int on failure
 */
int nfc_srix4k_init(nfc_device *reader);


/* libnfc data management */

/**
 * Transmit bytes to the srix tag and receive the response
 * @param target device pointer
 * @param tx_data array of uint8_t to transmit
 * @param tx_size size of tx_data
 * @param rx_data array of uint8_t to save the response
 * @param rx_size size of rx_data
 * @return response length in bytes
 */
size_t nfc_data_exchange(nfc_device *target, const uint8_t *restrict tx_data, size_t tx_size, uint8_t *restrict rx_data, size_t rx_size);

/**
 * Function that closes current LibNFC reader instance
 * @param NFC_reader pointer to LibNFC reader
 * @param NFC_context pointer to LibNFC context
 */
void nfc_close_current(nfc_device **reader, nfc_context **context);

#endif /* MIKAI_READER_H */