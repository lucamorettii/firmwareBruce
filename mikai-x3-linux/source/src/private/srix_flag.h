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


#ifndef MIKAI_SRIX_FLAG_H
#define MIKAI_SRIX_FLAG_H

#include <stdbool.h>
#include <stdint.h>
#include "flag.h"

/**
 * Initialize a srix_flag struct
 * @return an empty srix_flag struct
 */
static inline struct srix_flag srix_flag_init() {
    return (struct srix_flag) {{0, 0, 0, 0}};
}

/**
 * Add a "1" flag bit on block position
 * @param flag pointer to block flags struct
 * @param block block number to flag (0-127)
 */
void srix_flag_add(struct srix_flag flag[static 1], uint8_t block);

/**
 * Set flag bit on block position to "0"
 * @param flag pointer to block flags struct
 * @param block block number to remove flag (0-127)
 */
void srix_flag_remove(struct srix_flag flag[static 1], uint8_t block);

/**
 * Get flag bit value of a specified block
 * @param flag pointer to block flags struct
 * @param block block flag number to get (0-127)
 * @return boolean result
 */
bool srix_flag_get(struct srix_flag flag[static 1], uint8_t block);

/**
 * Check if at least one block has been modified
 * @param flag pointer to block flags struct
 * @return boolean result
 */
bool srix_flag_isModified(struct srix_flag flag[static 1]);

#endif /* MIKAI_SRIX_FLAG_H */