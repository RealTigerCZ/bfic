/**
 * @file interpreter.h
 * @author RealTigerCZ
 * @brief Brainf*ck interpreter header file
 * @version 0.1.1
 * @date 2023-11-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// TODO: Comment code

#ifndef BF_INTERPRETER_H
#define BF_INTERPRETER_H

#include "bfic.h"

#define BFI_MAX_TAPE_SIZE 1024*1024*1024 // 1 GiB
#define BFI_DUMP_RADIUS 8

Return_Code run_interpreter(Config *config);

#ifdef BF_INTERPRETER_IMPLEMENTATION

void dump_tape(void *tape, CMES cmes, size_t size, size_t cursor, FILE *output);
long long get_tape_value(void *tape, CMES cmes, size_t cursor);
void increment_tape_value(void *tape, CMES cmes, size_t cursor);
void decrement_tape_value(void *tape, CMES CMES, size_t cursor);

bool increment_cursor(CMOM cmom, size_t *cursor, size_t tape_size);
bool decrement_cursor(CMOM cmom, size_t *cursor, size_t tape_size);

Return_Code execute(void *tape, Config *config, size_t *cursor);

#endif // BF_INTERPRETER_IMPLEMENTATION

#endif // BF_INTERPRETER_H