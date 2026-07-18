/*
 * baregear - A programming language compiler
 * Copyright (C) 2026 First Person
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <iostream>
#include <string>

#define MAX_PREV_LINES              5

#define printMargin(col) do { \
    for (int k = 1; k < col; k++) \
        std::cout << " "; \
} while (0)

#define printNotice(row, col, colorAttr, message) do { \
    int i = 0; \
    int j = MAX_PREV_LINES; \
    while (row < j) \
        j--; \
    j = row > j && j >= 1 ? row - j : row; \
    \
    for(int k = 0; (j + k) < row; k++) \
        std::cout << getLine(j + k) << std::endl; \
    \
    printMargin(col); \
    std::cout << "\033[38;5;" << colorAttr << "m^]" << std::endl; \
    \
    printMargin(col); \
    std::cout << "\033[38;5;" << colorAttr << "m" << message << "\033[0m" << std::endl; \
} while (0)

extern std::string getLine(int index);
extern void error(std::string, int row, int col);
extern void warn(std::string, int row, int col);
extern void success();
extern void failure();
extern void bugDetected(std::string message);

#endif // DEFINITIONS_H
