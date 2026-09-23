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

#include "srix_flag.h"

void srix_flag_add(struct srix_flag flag[static 1], uint8_t block) {
    /* Get index on array (0 -> 0-31, 1-> 32-63, 2-> 64-95, 3-> 96-128)
     * Make an OR bitwise to flag position in this block (between 0 and 31)
     * 32 = uint32_t length
     */
    if (block < 128) flag->memory[block / 32] |= 1u << (block % 32);
}

void srix_flag_remove(struct srix_flag flag[static 1], uint8_t block) {
    /*
     * Inverse operation of flag_add (& specific bit with 0)
     */
    if (block < 128) flag->memory[block / 32] &= 0xFFFFFFFF - (1u << (block % 32));
}

bool srix_flag_get(struct srix_flag flag[static 1], uint8_t block) {
    /* If position is out of range, return false.
     * Else make a bitwise AND between last uint memory value and 1 and
     * check if it's different from 0 (so if it has been flagged before).
     * current_position +=1 after the return (for the next call to this function).
     */
    if (block < 128) {
        return (flag->memory[block / 32] >> (block % 32)) & 1u;
    } else {
        return false;
    }
}

bool srix_flag_isModified(struct srix_flag flag[static 1]) {
    return (flag->memory[0] | flag->memory[1] | flag->memory[2] | flag->memory[3]) > 0;
}