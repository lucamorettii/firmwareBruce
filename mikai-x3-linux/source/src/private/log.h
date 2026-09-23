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

#ifndef MIKAI_UTILS_H
#define MIKAI_UTILS_H

#include <stdarg.h>
#include <stdio.h>

/* Text style constants */
#define ANSI_COLOR_RESET    "\x1b[0m"
#define ANSI_BOLD           "\x1b[1m"
#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"

/* Logging */
#define LOG_STREAM stderr
#define OUT_STREAM stdout

/**
* Print an error (stderr)
* @param format printf() format
* @param ... optional parameters
*/
static inline void log_error(char const *const format, ...) {
    fprintf(LOG_STREAM, ANSI_BOLD ANSI_COLOR_RED "ERROR: ");

    /* variable fprintf() (runtime parameters validation) */
    va_list arguments;
    va_start(arguments, format);
    vfprintf(LOG_STREAM, format, arguments);
    va_end(arguments);

    fprintf(LOG_STREAM, ANSI_COLOR_RESET "\n");
}

/**
 * Print a generic operation (stdout)
 * @param format printf() format
 * @param ... optional parameters
 */
static inline void log_operation(char const *const format, ...) {
    fprintf(OUT_STREAM, ANSI_COLOR_GREEN);

    /* variable fprintf() (runtime parameters validation) */
    va_list arguments;
    va_start(arguments, format);
    vfprintf(OUT_STREAM, format, arguments);
    va_end(arguments);

    fprintf(OUT_STREAM, ANSI_COLOR_RESET);
}

#endif /* MIKAI_LOG_H */