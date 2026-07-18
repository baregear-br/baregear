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

#ifndef RUNTIME_H
#define RUNTIME_H

extern "C" {
    #include <stdint.h>

    // Memory allocation using sys_mmap
    // RCX: Address hint, RDX: Length
    // Returns: Address of allocated memory
    void* falloc(void* address, size_t length);
    
    // Memory reallocation using sys_mremap
    // RBX: Old address, RSI: Old size, RDX: New size
    // Returns: New address of reallocated memory
    void* frealloc(void* old_address, size_t old_size, size_t new_size);
    
    // Memory deallocation using sys_munmap
    // RCX: Address, RDX: Length
    // Returns: 0 on success, negative error code on failure
    int ffree(void* address, size_t length);
}

#endif // RUNTIME_H