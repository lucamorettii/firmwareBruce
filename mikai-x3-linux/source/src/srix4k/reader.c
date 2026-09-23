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

/* Library to sleep during polling */
#ifdef _WIN32
    #include <Windows.h>
#else
    #include <unistd.h>
#endif

#include "reader.h"
#include "log.h"


/*
 * libnfc initialization
 */

int nfc_context_init(nfc_context **context) {
    /* Set libnfc context */
    nfc_init(context);

    /* If context set is a null pointer, exit with an error */
    if (*context == (void *) 0) {
        log_error("Unable to initialize libnfc context.\n");
        return -1;
    }

    /* Display libnfc version */
    log_operation("libnfc version: %s\n", nfc_version());
    return 0;
}

int nfc_reader_init(nfc_device **reader, nfc_context **context, nfc_connstring target) {
    /* Open selected reader */
    *reader = nfc_open(*context, target);
    if (*reader == (void *) 0) {
        log_error("Unable to open NFC reader.\n");
        return -1;
    }

    /* Initialize NFC device as initiator (reader) */
    if (nfc_initiator_init(*reader) != 0) {
        log_error("Error with libnfc nfc_initiator_init(). %s.\n", nfc_strerror(*reader));
        return -2;
    }

    return 0;
}

int nfc_srix4k_init(nfc_device *reader) {
    /* Array to save SRIX4k targets */
    nfc_target target_key[MAX_TARGET_COUNT];
    log_operation("Waiting for tag...\n");

    /*
     * (libnfc) To read ISO14443B2SR you have to initiate first ISO14443B to configure PN532 internal registers.
     * https://github.com/nfc-tools/libnfc/issues/436#issuecomment-326686914
     */
    nfc_initiator_list_passive_targets(reader, nfc_ISO14443B, target_key, MAX_TARGET_COUNT);

    /* Do tag polling. */
    while(true) {
        int targets = nfc_initiator_list_passive_targets(reader, nfc_ISO14443B2SR, target_key, MAX_TARGET_COUNT);

        if (targets < 0) {
            log_error("Error with nfc_initiator_list_passive_target(). %s.\n", nfc_strerror(reader));
            return -1;
        }

        if (targets > 0) {
            log_operation("SRIX tag found!\n");
            break;
        }

        sleep(0.5);
    }

    return 0;
}


/*
 * libnfc management
 */

size_t nfc_data_exchange(nfc_device *target, const uint8_t *restrict tx_data, size_t tx_size, uint8_t *restrict rx_data, size_t rx_size) {
    return nfc_initiator_transceive_bytes(target, tx_data, tx_size, rx_data, rx_size, SRIX_TIMEOUT_MS);
}

void nfc_close_current(nfc_device **reader, nfc_context **context) {
    if (*reader != (void *) 0) nfc_close(*reader);
    if (*context != (void *) 0) nfc_exit(*context);

    *reader = (void *) 0;
    *context = (void *) 0;
}