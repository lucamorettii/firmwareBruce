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

#include "log.h"
#include "mykey.h"
#include "srix_flag.h"
#include "srix_private.h"


/*
 * Private functions
 */

/**
 * Encode or decode a mykey block
 * @param block data to manage
 */
static inline void encode_decode_block(uint8_t block[static 4]) {
    /* Copy block input to a temporary array */
    uint8_t input[4] = {block[0], block[1], block[2], block[3]};

    /*
     * Repeat for each of four bit groups
     * positions 0-5-10-15 are the same
     */

    /* first byte */
    block[0] = 0;
    block[0] |= input[0] & 0xC0; /* C0 = 11000000, get first two bits */
    block[0] |= (input[1] & 0xC0) >> 2;
    block[0] |= (input[2] & 0xC0) >> 4;
    block[0] |= (input[3] & 0xC0) >> 6;

    /* second byte */
    block[1] = 0;
    block[1] |= (input[0] & 0x30) << 2; /* 30 = 00110000, get second two bits */
    block[1] |= input[1] & 0x30;
    block[1] |= (input[2] & 0x30) >> 2;
    block[1] |= (input[3] & 0x30) >> 4;

    /* third byte */
    block[2] = 0;
    block[2] |= (input[0] & 0xC) << 4; /* C = 00001100, get third two bits */
    block[2] |= (input[1] & 0xC) << 2;
    block[2] |= input[2] & 0xC;
    block[2] |= (input[3] & 0xC) >> 2;

    /* fourth byte */
    block[3] = 0;
    block[3] |= (input[0] & 3) << 6; /* 3 = 00000011, get last two bits */
    block[3] |= (input[1] & 3) << 4;
    block[3] |= (input[2] & 3) << 2;
    block[3] |= input[3] & 3;
}

/**
 * Return a number between 0 and 7 that represent current transaction location
 * @param key pointer to mykey struct
 * @return transaction pointer
 */
static uint8_t get_current_transaction_offset(mykey *key) {
    if (key->srix4k->eeprom[0x3C][1] == 0xFF) {
        /* Set to 7 to start with the first transaction block */
        return 0x07;
    }

    uint8_t current[4];

    /* Decode transaction pointer */
    current[0] = key->srix4k->eeprom[0x3C][0];
    current[1] = key->srix4k->eeprom[0x3C][1] ^ key->srix4k->eeprom[0x07][1];
    current[2] = key->srix4k->eeprom[0x3C][2] ^ key->srix4k->eeprom[0x07][2];
    current[3] = key->srix4k->eeprom[0x3C][3] ^ key->srix4k->eeprom[0x07][3];
    encode_decode_block(current);

    /* Check transaction pointer */
    if(current[1] > 7) {
        log_operation("Wrong transaction pointer. Setting it to 7.\n");
        return 0x07;
    } else {
        /* Return result (0x00-0x07) */
        return current[1];
    }
}

/**
 * Print mykey production date
 * @param key pointer to mykey struct
 */
static void mykey_print_production_date(mykey *key) {
    log_operation("Production date: %02X/%02X/%X%X%02X [%02X%02X%02X%02X]\n", key->srix4k->eeprom[0x08][0],
                  key->srix4k->eeprom[0x08][1], key->srix4k->eeprom[0x08][3] & 0xF,
                  (key->srix4k->eeprom[0x08][3] >> 4) & 0xF,
                  key->srix4k->eeprom[0x08][2], key->srix4k->eeprom[0x08][0], key->srix4k->eeprom[0x08][1],
                  key->srix4k->eeprom[0x08][2], key->srix4k->eeprom[0x08][3]);
}

/**
 * Print 8 transactions history (sorted) of mykey
 * @param key pointer to mykey struct
 */
static void mykey_print_transactions_history(mykey *key) {
    /* Get current transaction pointer */
    uint8_t current = get_current_transaction_offset(key);
    log_operation("TRANSACTIONS HISTORY:\n");

    /* Current transaction values */
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint16_t credit;

    for(uint8_t i = 0; i < 8; i++) {
        /* Start with first transaction (the cycle of transactions iterate so the next) */
        if(current == 7) {
            current = 0;
        } else {
            current++;
        }

        if(key->srix4k->eeprom[0x34 + current][0] == 0xFF && key->srix4k->eeprom[0x34 + current][1] == 0xFF
        && key->srix4k->eeprom[0x34 + current][2] == 0xFF && key->srix4k->eeprom[0x34 + current][3] == 0xFF) {
            log_operation("[%" PRIu8 "] -> No transaction.\n", i);
            continue;
        }

        /* Decode data about current transaction */
        day = key->srix4k->eeprom[0x34 + current][0] >> 3;
        month = (((key->srix4k->eeprom[0x34 + current][0] & 0x07) << 1) | ((key->srix4k->eeprom[0x34 + current][1] & 0x80) >> 7));
        year = 2000 + (key->srix4k->eeprom[0x34 + current][1] & 0x7F);
        credit = (key->srix4k->eeprom[0x34 + current][2] << 8) | key->srix4k->eeprom[0x34 + current][3];

        /* Print result */
        log_operation("[%" PRIu8 "] -> %02" PRIu8 "/%02" PRIu8 "/%" PRIu16 " %.2f€ [%02X%02X%02X%02X]\n", i, day, month, year, credit/100.0,
                        key->srix4k->eeprom[0x34 + current][0], key->srix4k->eeprom[0x34 + current][1],
                        key->srix4k->eeprom[0x34 + current][2], key->srix4k->eeprom[0x34 + current][3]);
    }
}

/**
 * Calculate the encryption key and save the result in mykey struct.
 * @param key pointer to mykey data struct
 */
static inline void calculateEncryptionKey(mykey *key) {
    /* OTP calculation (reverse of block 6, incremental. 1,2,3, ecc.) */
    uint32_t otp = (((uint32_t)(0xFFu - key->srix4k->eeprom[0x06][3]) << 24) |
                   ((uint32_t)(0xFFu - key->srix4k->eeprom[0x06][2]) << 16) |
                   ((uint32_t)(0xFFu - key->srix4k->eeprom[0x06][1]) << 8) |
                   (uint32_t)(0xFFu - key->srix4k->eeprom[0x06][0])) + 1u;

    /* MasterKey calculation: based on vendor code and UID */
    uint8_t block18[4] = {key->srix4k->eeprom[0x18][0], key->srix4k->eeprom[0x18][1], key->srix4k->eeprom[0x18][2],
                          key->srix4k->eeprom[0x18][3]};
    uint8_t block19[4] = {key->srix4k->eeprom[0x19][0], key->srix4k->eeprom[0x19][1], key->srix4k->eeprom[0x19][2],
                          key->srix4k->eeprom[0x19][3]};
    encode_decode_block(block18);
    encode_decode_block(block19);

    uint32_t masterKey = key->srix4k->uid * ((((uint32_t) block18[2] << 24) | ((uint32_t) block18[3] << 16) |
                                            ((uint32_t) block19[2] << 8) | ((uint32_t) block19[3])) + 1u);

    /* Save encryption key (or SessionKey): MasterKey X OTP */
    key->encryptionKey = masterKey * otp;
}

/**
 * Calculate checksum of a generic block
 * @param block array to checksum (checksum saved on index 0)
 * @param blockNum number of the block(0-127)
 */
static inline void calculateBlockChecksum(uint8_t block[static 4], const uint8_t blockNum) {
    block[0] = 0xFF - blockNum - (block[3] & 0x0F) - (block[3] >> 4 & 0x0F) - (block[2] & 0x0F) - (block[2] >> 4 & 0x0F) - (block[1] & 0x0F) - (block[1] >> 4 & 0x0F);
}

/**
 * Return the number of days between 1/1/1995 and a specified date
 * @param day day of the second date
 * @param month month of the second date
 * @param year year of the second date
 * @return difference in days
 */
static uint32_t days_difference(int day, int month, int year) {
    if (month < 3) {
        year--;
        month += 12;
    }

    return year * 365 + year / 4 - year / 100 + year / 400 + (month * 153 + 3) / 5 + day - 728692;
}

/*
 * mykey functions
 */

void mykey_init(mykey key[static 1]) {
    calculateEncryptionKey(key);
}

int checkLockID(mykey key[static 1]) {
    /* If there is lock id but block 21 checksum is right, don't block mikai */
    uint8_t creditCheck[4];

    /* Decode 0x21 with Sk */
    creditCheck[0] = key->srix4k->eeprom[0x21][0] ^ key->encryptionKey >> 24;
    creditCheck[1] = key->srix4k->eeprom[0x21][1] ^ key->encryptionKey >> 16;
    creditCheck[2] = key->srix4k->eeprom[0x21][2] ^ key->encryptionKey >> 8;
    creditCheck[3] = key->srix4k->eeprom[0x21][3] ^ key->encryptionKey;
    /* Reorder bits */
    encode_decode_block(creditCheck);
    /* Save checksum */
    uint8_t check = creditCheck[0];
    /* Calculate checksum */
    calculateBlockChecksum(creditCheck, 0x21);


    /* Check lock id and checksum */
    if (key->srix4k->eeprom[0x05][3] == 0x7F && check != creditCheck[0]) {
        return 1;
    }

    return 0;
}

bool mykey_isReset(mykey key[static 1]) {
    return key->srix4k->eeprom[0x18][0] == 0x8F && key->srix4k->eeprom[0x18][1] == 0xCD &&
           key->srix4k->eeprom[0x18][2] == 0x0F && key->srix4k->eeprom[0x18][3] == 0x48 &&
           key->srix4k->eeprom[0x19][0] == 0xC0 && key->srix4k->eeprom[0x19][1] == 0x82 &&
           key->srix4k->eeprom[0x19][2] == 0x00 && key->srix4k->eeprom[0x19][3] == 0x07;
}

void mykey_print_info(mykey key[static 1]) {
    log_operation("Lock ID: %s\n", checkLockID(key) > 0 ? "yes" : "no");

    if(checkLockID(key) <= 0) {
        if(!mykey_isReset(key)) {
            log_operation("Actual credit: %.2f€\n", mykey_get_current_credit(key) / 100.0);
        }
        log_operation("SK: %" PRIX32 "\n", key->encryptionKey);
        log_operation("IS BOUND: %s\n", !mykey_isReset(key) ? "yes" : "no");
    }

    mykey_print_production_date(key);
    mykey_print_transactions_history(key);
}


/*
 * EEPROM
 */

void mykey_import_vendor(mykey key[static 1], uint8_t block18[const static 4], uint8_t block19[const static 4]) {
    /* Set new block 18 and 19 */
    memcpy(key->srix4k->eeprom[0x18], block18, SRIX_BLOCK_LENGTH);
    srix_flag_add(&key->srix4k->srixFlag, 0x18);
    memcpy(key->srix4k->eeprom[0x19], block19, SRIX_BLOCK_LENGTH);
    srix_flag_add(&key->srix4k->srixFlag, 0x19);

    /* Decode blocks 21 and 25 with precedent vendor SK */
    key->srix4k->eeprom[0x21][0] ^= key->encryptionKey >> 24;
    key->srix4k->eeprom[0x21][1] ^= key->encryptionKey >> 16;
    key->srix4k->eeprom[0x21][2] ^= key->encryptionKey >> 8;
    key->srix4k->eeprom[0x21][3] ^= key->encryptionKey;

    key->srix4k->eeprom[0x25][0] ^= key->encryptionKey >> 24;
    key->srix4k->eeprom[0x25][1] ^= key->encryptionKey >> 16;
    key->srix4k->eeprom[0x25][2] ^= key->encryptionKey >> 8;
    key->srix4k->eeprom[0x25][3] ^= key->encryptionKey;

    /* Recalculate keys with new vendor */
    calculateEncryptionKey(key);

    /* Encode 21 and 25 with new vendor SK */
    key->srix4k->eeprom[0x21][0] ^= key->encryptionKey >> 24;
    key->srix4k->eeprom[0x21][1] ^= key->encryptionKey >> 16;
    key->srix4k->eeprom[0x21][2] ^= key->encryptionKey >> 8;
    key->srix4k->eeprom[0x21][3] ^= key->encryptionKey;
    srix_flag_add(&key->srix4k->srixFlag, 0x21);

    key->srix4k->eeprom[0x25][0] ^= key->encryptionKey >> 24;
    key->srix4k->eeprom[0x25][1] ^= key->encryptionKey >> 16;
    key->srix4k->eeprom[0x25][2] ^= key->encryptionKey >> 8;
    key->srix4k->eeprom[0x25][3] ^= key->encryptionKey;
    srix_flag_add(&key->srix4k->srixFlag, 0x25);

    /* Copy vendor blocks 18 and 19 to their copy 1C and 1D */
    memcpy(key->srix4k->eeprom[0x1C], key->srix4k->eeprom[0x18], SRIX_BLOCK_LENGTH);
    memcpy(key->srix4k->eeprom[0x1D], key->srix4k->eeprom[0x19], SRIX_BLOCK_LENGTH);
    srix_flag_add(&key->srix4k->srixFlag, 0x1C);
    srix_flag_add(&key->srix4k->srixFlag, 0x1D);

    /* Change checksum adapting to new blocks */
    encode_decode_block(key->srix4k->eeprom[0x1C]);
    encode_decode_block(key->srix4k->eeprom[0x1D]);

    calculateBlockChecksum(key->srix4k->eeprom[0x1C], 0x1C);
    calculateBlockChecksum(key->srix4k->eeprom[0x1D], 0x1D);

    encode_decode_block(key->srix4k->eeprom[0x1C]);
    encode_decode_block(key->srix4k->eeprom[0x1D]);
}

int mykey_export_vendor(mykey key[static 1], uint8_t buffer[static 8]) {
    /* Export vendor value to buffer */
    if(mykey_isReset(key)) {
        return -1;
    }

    memcpy(buffer, key->srix4k->eeprom[0x18], SRIX_BLOCK_LENGTH*2);

    return 0;
}

void mykey_reset(mykey key[static 1]) {
    for (uint8_t i = 0x10; i < SRIX4K_BLOCKS; i++) {
        /* current block new value */
        uint8_t block[4];

        switch (i) {
            case 0x10:
            case 0x14:
            case 0x3F:
            case 0x43:
                /* partial Key ID [first byte] + days elapsed from production */
                /* CHECK | ID | DAYS | DAYS */
                /* First byte of key id at byte 1 */
                block[1] = key->srix4k->eeprom[0x07][0];

                /* Decoding BCD (Binary Coded Decimal) date*/
                int day = ((key->srix4k->eeprom[0x08][0] & 0xF0u) >> 4) * 10 + (key->srix4k->eeprom[0x08][0] & 0x0Fu);
                int month = ((key->srix4k->eeprom[0x08][1] & 0xF0u) >> 4) * 10 + (key->srix4k->eeprom[0x08][1] & 0x0Fu);
                /* year: thousands, hundreds and units */
                int year = (key->srix4k->eeprom[0x08][3] & 0x0Fu) * 1000 +
                           ((key->srix4k->eeprom[0x08][3] & 0xF0u) >> 4) * 100
                           + ((key->srix4k->eeprom[0x08][2] & 0xF0u) >> 4) * 10 +
                           (key->srix4k->eeprom[0x08][2] & 0x0Fu);

                /* Calculate days difference */
                uint32_t elapsed = days_difference(day, month, year);

                /* Save difference into two bytes */
                block[2] = (elapsed / 1000 % 10) * 16 + (elapsed / 100 % 10);
                block[3] = (elapsed / 10 % 10) * 16 + (elapsed % 10);

                /* Calculate checksum */
                calculateBlockChecksum(block, i);
                break;

            case 0x11:
            case 0x15:
            case 0x40:
            case 0x44:
                /* Key ID [other three bytes] */
                block[1] = key->srix4k->eeprom[0x07][1];
                block[2] = key->srix4k->eeprom[0x07][2];
                block[3] = key->srix4k->eeprom[0x07][3];
                calculateBlockChecksum(block, i);
                break;

            case 0x22:
            case 0x26:
            case 0x51:
            case 0x55:
                /* production date (last three bytes) */
                block[1] = key->srix4k->eeprom[0x08][2];
                block[2] = key->srix4k->eeprom[0x08][1];
                block[3] = key->srix4k->eeprom[0x08][3];
                calculateBlockChecksum(block, i);
                encode_decode_block(block);
                break;

            case 0x12:
            case 0x16:
            case 0x41:
            case 0x45:
                /* Operations counter */
                block[1] = 0x00;
                block[2] = 0x00;
                block[3] = 0x01;
                calculateBlockChecksum(block, i);
                break;

            case 0x13:
            case 0x17:
            case 0x42:
            case 0x46:
                /* Generic blocks */
                block[1] = 0x04;
                block[2] = 0x00;
                block[3] = 0x13;
                calculateBlockChecksum(block, i);
                break;

            case 0x18:
            case 0x1C:
            case 0x47:
            case 0x4B:
                /* Generic blocks */
                block[1] = 0x00;
                block[2] = 0xFE;
                block[3] = 0xDC;
                calculateBlockChecksum(block, i);
                encode_decode_block(block);
                break;

            case 0x1D:
            case 0x48:
            case 0x4C:
                /* Generic blocks */
                block[1] = 0x00;
                block[2] = 0x01;
                block[3] = 0x23;
                calculateBlockChecksum(block, i);
                encode_decode_block(block);
                break;

            case 0x19:
                /* Generic blocks */
                block[1] = 0x00;
                block[2] = 0x01;
                block[3] = 0x23;
                calculateBlockChecksum(block, i);
                encode_decode_block(block);

                /* Calculate new keys (for block 21 and 25) */
                calculateEncryptionKey(key);
                break;

            case 0x21:
            case 0x25:
                /* Current credit (0,00€) */
                block[1] = 0x00;
                block[2] = 0x00;
                block[3] = 0x00;
                calculateBlockChecksum(block, i);
                encode_decode_block(block);
                block[0] ^= key->encryptionKey >> 24;
                block[1] ^= key->encryptionKey >> 16;
                block[2] ^= key->encryptionKey >> 8;
                block[3] ^= key->encryptionKey;
                break;

            case 0x20:
            case 0x24:
            case 0x4F:
            case 0x53:
                /* Generic blocks */
                block[1] = 0x01;
                block[2] = 0x00;
                block[3] = 0x00;
                calculateBlockChecksum(block, i);
                encode_decode_block(block);
                break;

            case 0x1A:
            case 0x1B:
            case 0x1E:
            case 0x1F:
            case 0x23:
            case 0x27:
            case 0x49:
            case 0x4A:
            case 0x4D:
            case 0x4E:
            case 0x50:
            case 0x52:
            case 0x54:
            case 0x56:
                /* Generic blocks */
                block[1] = 0x00;
                block[2] = 0x00;
                block[3] = 0x00;
                calculateBlockChecksum(block, i);
                encode_decode_block(block);
                break;

            default:
                /* Else, set to FF */
                block[0] = 0xFF;
                block[1] = 0xFF;
                block[2] = 0xFF;
                block[3] = 0xFF;
                break;
        }

        /* If this block has a different value than precedent, flag it. */
        if (block[0] != key->srix4k->eeprom[i][0] || block[1] != key->srix4k->eeprom[i][1] ||
            block[2] != key->srix4k->eeprom[i][2] || block[3] != key->srix4k->eeprom[i][3]) {
            log_operation("Resetting block %02X...\n", i);

            key->srix4k->eeprom[i][0] = block[0];
            key->srix4k->eeprom[i][1] = block[1];
            key->srix4k->eeprom[i][2] = block[2];
            key->srix4k->eeprom[i][3] = block[3];

            srix_flag_add(&key->srix4k->srixFlag, i);
        }
    }

    /* Log end of operation */
    log_operation("MyKey reset successfully!\n");
}

void mykey_reset_otp(mykey key[static 1]) {
    if(srix_reset_otp(key->srix4k) >= 0) {
        calculateEncryptionKey(key);
        log_operation("Mykey reset OTP successfully!\n");
    }
}


/*
 * credit functions
 */

uint16_t mykey_get_current_credit(mykey key[static 1]) {
    uint8_t current_credit[4];

    /* XOR with sk to decode */
    current_credit[0] = key->srix4k->eeprom[0x21][0] ^ (key->encryptionKey >> 24u);
    current_credit[1] = key->srix4k->eeprom[0x21][1] ^ (key->encryptionKey >> 16u);
    current_credit[2] = key->srix4k->eeprom[0x21][2] ^ (key->encryptionKey >> 8u);
    current_credit[3] = key->srix4k->eeprom[0x21][3] ^ key->encryptionKey;

    /* Change bit order to decode */
    encode_decode_block(current_credit);

    /* Return actual credit */
    return current_credit[2] << 8 | current_credit[3];
}

int mykey_add_cents(mykey key[static 1], uint16_t cents, uint8_t day, uint8_t month, uint8_t year) {
    /* Check lock id */
    if (checkLockID(key) > 0) {
        log_error("Your key has an unknown protection (Lock id). Unable to charge money.");
        return -1;
    }

    /* Check reset key */
    if (mykey_isReset(key)) {
        log_error("Your mykey isn't associated with any vendor.");
        return -2;
    }

    /* Check recharge amount */
    if (cents < 5) {
        log_error("Unable to recharge. Try with a larger credit.");
        return -3;
    }

    /* Calculate current credit */
    uint16_t precedent_credit;
    uint16_t actual_credit = mykey_get_current_credit(key);

    /* Get current transaction position */
    uint8_t current = get_current_transaction_offset(key);

    /* Split credit into multiple transaction. Stop at less than 5 cent. */
    while (cents >= 5) {
        /* Save current credit */
        precedent_credit = actual_credit;

        if (cents / 200 > 0) {
            /* 2€ */
            cents -= 200;
            actual_credit += 200;
        } else if (cents / 100 > 0) {
            /* 1€ */
            cents -= 100;
            actual_credit += 100;
        } else if (cents / 50 > 0) {
            /* 0,50€ */
            cents -= 50;
            actual_credit += 50;
        } else if (cents / 20 > 0) {
            /* 0,20€ */
            cents -= 20;
            actual_credit += 20;
        } else if (cents / 10 > 0) {
            /* 0,10€ */
            cents -= 10;
            actual_credit += 10;
        } else if (cents / 5 > 0) {
            /* 0,05€ */
            cents -= 5;
            actual_credit += 5;
        }

        /* Point to new charge position */
        if(current == 7){
            current = 0;
        } else {
            current++;
        }

        /* Save new credit to history blocks */
        key->srix4k->eeprom[0x34 + current][0] = day << 3 | (month & 0x0E) >> 1;
        key->srix4k->eeprom[0x34 + current][1] = month << 7 | (year & 0x7F);
        key->srix4k->eeprom[0x34 + current][2] = actual_credit >> 8;
        key->srix4k->eeprom[0x34 + current][3] = actual_credit;
        srix_flag_add(&key->srix4k->srixFlag, 0x34 + current);
    }

    /* Save new credit to 21 and 25 */
    key->srix4k->eeprom[0x21][1] = 0x00;
    key->srix4k->eeprom[0x21][2] = actual_credit >> 8;
    key->srix4k->eeprom[0x21][3] = actual_credit;
    calculateBlockChecksum(key->srix4k->eeprom[0x21], 0x21);
    encode_decode_block(key->srix4k->eeprom[0x21]);
    key->srix4k->eeprom[0x21][0] ^= key->encryptionKey >> 24u;
    key->srix4k->eeprom[0x21][1] ^= key->encryptionKey >> 16u;
    key->srix4k->eeprom[0x21][2] ^= key->encryptionKey >> 8u;
    key->srix4k->eeprom[0x21][3] ^= key->encryptionKey;
    srix_flag_add(&key->srix4k->srixFlag, 0x21);

    key->srix4k->eeprom[0x25][1] = 0x00;
    key->srix4k->eeprom[0x25][2] = actual_credit >> 8;
    key->srix4k->eeprom[0x25][3] = actual_credit;
    calculateBlockChecksum(key->srix4k->eeprom[0x25], 0x25);
    encode_decode_block(key->srix4k->eeprom[0x25]);
    key->srix4k->eeprom[0x25][0] ^= key->encryptionKey >> 24u;
    key->srix4k->eeprom[0x25][1] ^= key->encryptionKey >> 16u;
    key->srix4k->eeprom[0x25][2] ^= key->encryptionKey >> 8u;
    key->srix4k->eeprom[0x25][3] ^= key->encryptionKey;
    srix_flag_add(&key->srix4k->srixFlag, 0x25);

    /* Save precedent credit to 23 and 27 */
    key->srix4k->eeprom[0x23][1] = 0x00;
    key->srix4k->eeprom[0x23][2] = precedent_credit >> 8;
    key->srix4k->eeprom[0x23][3] = precedent_credit;
    calculateBlockChecksum(key->srix4k->eeprom[0x23], 0x23);
    encode_decode_block(key->srix4k->eeprom[0x23]);
    srix_flag_add(&key->srix4k->srixFlag, 0x23);

    key->srix4k->eeprom[0x27][1] = 0x00;
    key->srix4k->eeprom[0x27][2] = precedent_credit >> 8;
    key->srix4k->eeprom[0x27][3] = precedent_credit;
    calculateBlockChecksum(key->srix4k->eeprom[0x27], 0x27);
    encode_decode_block(key->srix4k->eeprom[0x27]);
    srix_flag_add(&key->srix4k->srixFlag, 0x27);

    /* Save transaction pointer to block 3C */
    key->srix4k->eeprom[0x3C][1] = current;
    key->srix4k->eeprom[0x3C][2] = 0;
    key->srix4k->eeprom[0x3C][3] = 0;
    calculateBlockChecksum(key->srix4k->eeprom[0x3C], 0x3C);
    encode_decode_block(key->srix4k->eeprom[0x3C]);
    key->srix4k->eeprom[0x3C][1] ^= key->srix4k->eeprom[0x07][1];
    key->srix4k->eeprom[0x3C][2] ^= key->srix4k->eeprom[0x07][2];
    key->srix4k->eeprom[0x3C][3] ^= key->srix4k->eeprom[0x07][3];
    srix_flag_add(&key->srix4k->srixFlag, 0x3C);

    return 0;
}

int mykey_set_cents(mykey key[static 1], uint16_t cents, uint8_t day, uint8_t month, uint8_t year) {
    /* dump precedent blocks (in case of failure) */
    uint8_t dump[10][4];
    memcpy(dump[0], key->srix4k->eeprom[0x21], SRIX_BLOCK_LENGTH);
    memcpy(dump[1], key->srix4k->eeprom[0x34], 9*SRIX_BLOCK_LENGTH);

    /* Set block 21 to calculated credit 0 (checksum + 0x00 + 0x00 + 0x00 encoded) */
    key->srix4k->eeprom[0x21][0] = 0xC0;
    key->srix4k->eeprom[0x21][1] = 0x40;
    key->srix4k->eeprom[0x21][2] = 0xC0;
    key->srix4k->eeprom[0x21][3] = 0x80;
    key->srix4k->eeprom[0x21][0] ^= key->encryptionKey >> 24u;
    key->srix4k->eeprom[0x21][1] ^= key->encryptionKey >> 16u;
    key->srix4k->eeprom[0x21][2] ^= key->encryptionKey >> 8u;
    key->srix4k->eeprom[0x21][3] ^= key->encryptionKey;

    /* Reset transaction history and pointer (0x24-0x3C) */
    memset(key->srix4k->eeprom[0x34], 0xFF, 9*SRIX_BLOCK_LENGTH);

    /* If there is an error, restore precedent dump */
    if(mykey_add_cents(key, cents, day, month, year) < 0) {
        memcpy(key->srix4k->eeprom[0x21], dump[0], SRIX_BLOCK_LENGTH);
        memcpy(key->srix4k->eeprom[0x34], dump[1], 9*SRIX_BLOCK_LENGTH);
        return -1;
    }

    return 0;
}