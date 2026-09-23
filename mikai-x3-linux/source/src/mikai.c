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

#include "mikai.h"
#include "reader.h"
#include "srix_private.h"

mykey *mykey_new() {
    /* Allocate mykey struct and check if there are errors */
    mykey *target = (mykey *) malloc(sizeof(mykey));
    if(target == (void *) 0) {
        return (void *) 0;
    }

    /* Allocate srix struct and check if there are errors */
    target->srix4k = (srix *) malloc(sizeof(srix));
    if(target->srix4k == (void *) 0) {
        return (void *) 0;
    }

    /* Set nfc context to null to avoid problems */
    target->srix4k->context = (void *) 0;
    target->srix4k->reader = (void *) 0;

    /* Reset mykey chip id (for the check if it's new) */
    target->srix4k->eeprom[0x07][0] = 0x00;
    target->srix4k->eeprom[0x07][1] = 0x00;
    target->srix4k->eeprom[0x07][2] = 0x00;
    target->srix4k->eeprom[0x07][3] = 0x00;

    return target;
}

mykey *mykey_nfc_init(mykey **target, char *reader) {
    /* Allocate struct memory if pointer isn't null */
    if(*target == (void *) 0) {
        *target = mykey_new();
        /* If there is an error when allocating return null */
        if(*target == (void *) 0) {
            return (void *) 0;
        }
    }

    /* Initialize srix from NFC */
    if(srix_nfc_init((*target)->srix4k, reader) < 0) {
        mykey_delete(*target);
        *target = (void *) 0;
        return (void *) 0;
    }

    /* Calculate mykey keys */
    mykey_init(*target);

    /* Return pointer */
    return *target;
}

mykey *mykey_memory_init(mykey **target, uint8_t dump[const static SRIX4K_BYTES], uint64_t uid) {
    /* Allocate struct memory if pointer isn't null */
    if(*target == (void *) 0) {
        *target = mykey_new();

        /* If there is an error when allocating return null */
        if(*target == (void *) 0) {
            return (void *) 0;
        }
    }

    /* Initialize srix from memory */
    srix_memory_init((*target)->srix4k, dump, uid);

    /* Calculate mykey keys */
    mykey_init(*target);

    /* Return pointer */
    return *target;
}

void mykey_delete(mykey *key) {
    /* If nfc was opened, close it. */
    if (key->srix4k->reader != (void *) 0) {
        nfc_close_current(&key->srix4k->reader, &key->srix4k->context);
    }
    /* Free memory */
    free(key->srix4k);
    free(key);
}

const char *mikai_version() {
    return MIKAI_VERSION;
}