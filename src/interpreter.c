/**
 * @file interpreter.c
 * @author RealTigerCZ
 * @brief Brainf*ck interpreter implementation
 * @version 0.1
 * @date 2023-11-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/* TODO: Many things
 * - comment code
 * - refactor dumping, maybe executing to be more readable and consistent
 * - try to ditch IMES switches and casting types
 * - refactor loops (with bytecode)
 */


#define BF_INTERPRETER_IMPLEMENTATION
#include "interpreter.h"
#include "usefull_macros.h"

long long get_tape_value(void *tape, IMES imes, size_t cursor) {
    switch (imes) {
        case IMES_Byte:  return (long long) ((char *)      tape)[cursor];
        case IMES_Word:  return (long long) ((short *)     tape)[cursor];
        case IMES_DWord: return (long long) ((int *)       tape)[cursor];
        case IMES_QWord: return (long long) ((long long *) tape)[cursor];
    }
    eprint("Unreachable: Unknown IMES value");
    return 0;
}

void increment_tape_value(void *tape, IMES imes, size_t cursor) {
    switch (imes) {
        case IMES_Byte:  ((char *)      tape)[cursor]++; break;
        case IMES_Word:  ((short *)     tape)[cursor]++; break;
        case IMES_DWord: ((int *)       tape)[cursor]++; break;
        case IMES_QWord: ((long long *) tape)[cursor]++; break;
    }
}

void decrement_tape_value(void *tape, IMES imes, size_t cursor) {
    switch (imes) {
        case IMES_Byte:  ((char *)      tape)[cursor]--; break;
        case IMES_Word:  ((short *)     tape)[cursor]--; break;
        case IMES_DWord: ((int *)       tape)[cursor]--; break;
        case IMES_QWord: ((long long *) tape)[cursor]--; break;
    }
}

bool increment_cursor(IMOM imom, size_t *cursor, size_t tape_size) {
    switch (imom) {
        case IMOM_Default: (*cursor)++; break;
        case IMOM_Wrap:    *cursor = (*cursor + 1) % tape_size; break;
        case IMOM_Abort:   if ((*cursor)++ >= tape_size) return false;
    }
    return true;
}

bool decrement_cursor(IMOM imom, size_t *cursor, size_t tape_size) {
    switch (imom) {
        case IMOM_Default: (*cursor)--; break;
        case IMOM_Wrap:    *cursor = (*cursor + tape_size - 1) % tape_size; break;
        case IMOM_Abort:   if (*cursor == 0) return false;
    }
    return true;
}

Interpreter_Result run_interpreter(size_t tape_size, bool debug, IMOM imom, IMES imes, FILE *input, FILE *output) {
    if (tape_size == 0) {
        eprint("Tape size cannot be zero");
        return IR_Error;
    }

    if (input == NULL) {
        eprint("Input stream cannot be NULL");
        return IR_Error;
    }

    if (output == NULL) {
        eprint("Output stream cannot be NULL");
        return IR_Error;
    }

    size_t size_in_bytes = tape_size * (size_t) imes;

    if (size_in_bytes > BFI_MAX_TAPE_SIZE) {
        eprintf("Tape size too big. Max size: %u bytes (%u GiB) and requested size: %zu bytes (%zu GiB)", 
            BFI_MAX_TAPE_SIZE, BFI_MAX_TAPE_SIZE / 1024 / 1024 / 1024, size_in_bytes, size_in_bytes / 1024 / 1024 / 1024);
        return IR_Error;
    }

    void *tape = malloc(size_in_bytes);
    if (tape == NULL) {
        eprint("Failed to allocate memory for tape");
        return IR_Error;
    }

    memset(tape, 0, size_in_bytes);
    size_t cursor = 0;

    execute(tape, tape_size, imom, imes, input, output, &cursor, debug);
    return IR_Success;
}

Interpreter_Result execute(void *tape, size_t tape_size, IMOM imom, IMES imes, FILE *input, FILE *output, size_t *cursor, bool debug) {
    static long long level = 0;

    int c = fgetc(input);
    while (c != EOF) {
        switch (c) {
            case '>': if (!increment_cursor(imom, cursor, tape_size)) return IR_Abort; break;
            case '<': if (!decrement_cursor(imom, cursor, tape_size)) return IR_Abort; break;
            case '+': increment_tape_value(tape, imes, *cursor); break;
            case '-': decrement_tape_value(tape, imes, *cursor); break;
            case '.':
                long long   value = get_tape_value(tape, imes, *cursor); 
                if (imes == IMES_Byte) fputc((char) value, output);
                else fprintf(output, "%llu (0x%llX)\n", value, value);
                break;
            case ',': {
                long long value = 0;
                for (int i = 0; i < (int) imes; i++) {
                    int ch = fgetc(input);
                    if (ch == EOF) {
                        eprint("Unexpected end of file when loading value");
                        return IR_Abort;
                    }
                    value <<= 8;
                    value += ch;
                }
                switch (imes) {
                    case IMES_Byte:  ((char *)      tape)[*cursor] = (char)      value; break;
                    case IMES_Word:  ((short *)     tape)[*cursor] = (short)     value; break;
                    case IMES_DWord: ((int *)       tape)[*cursor] = (int)       value; break;
                    case IMES_QWord: ((long long *) tape)[*cursor] = (long long) value; break;
                }
            }
            break;
            case '[': 
                if (get_tape_value(tape, imes, *cursor) == 0) {
                    c = fgetc(input);
                    while (c != ']') {
                        if (c == EOF) {
                            eprint("Unexpected end of file in loop");
                            return IR_Abort;
                        }
                        c = fgetc(input);
                    }
                } else {
                    size_t position = ftell(input);
                    level++;
                    Interpreter_Result err = execute(tape, tape_size, imom, imes, input, output, cursor, debug);
                    if (err != IR_Success) return err;
                    fseek(input, position, SEEK_SET);
                    ungetc('[', input);
                }
                break;
            case ']':
                if (level == 0) {
                    eprint("Unexpected end of loop");
                    return IR_Abort;
                }

                level--;
                return IR_Success;
            case '#':
                if (debug) {
                    dump_tape(tape, imes, tape_size, *cursor, output);
                }
                break;
        }
        c = fgetc(input);
    }
    
    if (level != 0) {
        eprint("Unexpected end of file -- missing ']'");
        return IR_Abort;
    }

    return IR_Success;
}

// CHECK: printing end of tape (end is == size - 1)
void dump_tape(void *tape, IMES imes, size_t size, size_t cursor, FILE *output) {
    size_t start = cursor < BFI_DUMP_RADIUS ? 0 : cursor - BFI_DUMP_RADIUS;
    size_t end = cursor + BFI_DUMP_RADIUS > size ? size : cursor + BFI_DUMP_RADIUS;

    fprintf(output, "Tape size: %zu, cursor: %zu\n", size, cursor);
    fprintf(output, "Dumping tape: [");

     
    for (int i = 0; i < (BFI_DUMP_RADIUS / 2) * (start != 0); i++) {
        long long value = get_tape_value(tape, imes, i);
        fprintf(output, "%llu (0x%llX), ", value, value);
    }

    if (start > (BFI_DUMP_RADIUS / 2) * (start != 0)) {
        fprintf(output, "..., ");
    } else {
        start = (BFI_DUMP_RADIUS / 2) * (start != 0);
    }

    for (size_t i = start; i < cursor; i++) {
        long long value = get_tape_value(tape, imes, i);
        fprintf(output, "%llu (0x%llX), ", value, value);
    }

    fprintf(output, ">>> %llu (0x%llX) <<<, ", get_tape_value(tape, imes, cursor), get_tape_value(tape, imes, cursor));

    for (size_t i = cursor + 1; i < end; i++) {
        long long value = get_tape_value(tape, imes, i);
        fprintf(output, "%llu (0x%llX), ", value, value);
    }

    if (end < size - 2 - BFI_DUMP_RADIUS / 2) {
        fprintf(output, "..., ");
        end = size - 2 - BFI_DUMP_RADIUS / 2;
    } else {
        end++;
    }

    for (size_t i = end; i < size - 2; i++) {
        long long value = get_tape_value(tape, imes, i);
        fprintf(output, "%llu (0x%llX), ", value,value);
    }

    fprintf(output, "%llu (0x%llX)]\n", get_tape_value(tape, imes, size - 1) , get_tape_value(tape, imes, size - 1));
}