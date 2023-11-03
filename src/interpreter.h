/**
 * @file interpreter.h
 * @author RealTigerCZ
 * @brief Brainf*ck interpreter header file
 * @version 0.1
 * @date 2023-11-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// TODO: Comment code

#ifndef BF_INTERPRETER_H
#define BF_INTERPRETER_H

#include <stdio.h>
#include <stdbool.h>
#include "usefull_macros.h"


#define BFI_MAX_TAPE_SIZE 1024*1024*1024 // 1 GiB
#define BFI_DUMP_RADIUS 8

typedef enum interpreter_memory_overflow_mode_t {
    IMOM_Default = 0,
    IMOM_Wrap,
    IMOM_Abort
} IMOM;

typedef enum interpreter_memory_element_size_t {
    IMES_Byte  = 1,
    IMES_Word  = 2,
    IMES_DWord = 4,
    IMES_QWord = 8
} IMES;

typedef enum interpreter_result_t {
    IR_Success = 0,
    IR_Abort,
    IR_Error
} Interpreter_Result;


Interpreter_Result run_interpreter(size_t tape_size, bool debug, IMOM imom, IMES imes, FILE *input, FILE *output);

#ifdef BF_INTERPRETER_IMPLEMENTATION

void dump_tape(void *tape, IMES imes, size_t size, size_t cursor, FILE *output);
long long get_tape_value(void *tape, IMES imes, size_t cursor);
void increment_tape_value(void *tape, IMES imes, size_t cursor);
void decrement_tape_value(void *tape, IMES imes, size_t cursor);

bool increment_cursor(IMOM imom, size_t *cursor, size_t tape_size);
bool decrement_cursor(IMOM imom, size_t *cursor, size_t tape_size);

Interpreter_Result execute(void *tape, size_t tape_size, IMOM imom, IMES imes, FILE *input, FILE *output, size_t *cursor, bool debug);

#endif // BF_INTERPRETER_IMPLEMENTATION

#endif // BF_INTERPRETER_H