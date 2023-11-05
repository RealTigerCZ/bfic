/**
 * @file interpreter.c
 * @author RealTigerCZ
 * @brief Brainf*ck interpreter implementation
 * @version 0.1.1
 * @date 2023-11-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/* TODO: Many things
 * - comment code
 * - refactor dumping, maybe executing to be more readable and consistent
 * - try to ditch CMES switches and casting types
 * - refactor loops (with bytecode)
 */


#define BF_INTERPRETER_IMPLEMENTATION
#include "interpreter.h"


long long get_tape_value(void *tape, CMES imes, size_t cursor) {
    switch (imes) {
        case CMES_Byte:  return (long long) ((char *)      tape)[cursor];
        case CMES_Word:  return (long long) ((short *)     tape)[cursor];
        case CMES_DWord: return (long long) ((int *)       tape)[cursor];
        case CMES_QWord: return (long long) ((long long *) tape)[cursor];
    }
    eprint("Unreachable: Unknown CMES value");
    return 0;
}

void increment_tape_value(void *tape, CMES imes, size_t cursor) {
    switch (imes) {
        case CMES_Byte:  ((char *)      tape)[cursor]++; break;
        case CMES_Word:  ((short *)     tape)[cursor]++; break;
        case CMES_DWord: ((int *)       tape)[cursor]++; break;
        case CMES_QWord: ((long long *) tape)[cursor]++; break;
    }
}

void decrement_tape_value(void *tape, CMES imes, size_t cursor) {
    switch (imes) {
        case CMES_Byte:  ((char *)      tape)[cursor]--; break;
        case CMES_Word:  ((short *)     tape)[cursor]--; break;
        case CMES_DWord: ((int *)       tape)[cursor]--; break;
        case CMES_QWord: ((long long *) tape)[cursor]--; break;
    }
}

bool increment_cursor(CMOM cmom, size_t *cursor, size_t tape_size) {
    switch (cmom) {
        case CMOM_Default: (*cursor)++; break;
        case CMOM_Wrap:    *cursor = (*cursor + 1) % tape_size; break;
        case CMOM_Abort:   if ((*cursor)++ >= tape_size) return false;
    }
    return true;
}

bool decrement_cursor(CMOM cmom, size_t *cursor, size_t tape_size) {
    switch (cmom) {
        case CMOM_Default: (*cursor)--; break;
        case CMOM_Wrap:    *cursor = (*cursor + tape_size - 1) % tape_size; break;
        case CMOM_Abort:   if (*cursor == 0) return false;
    }
    return true;
}

Return_Code run_interpreter(Config *config) {
    if (config->tape_size == 0) {
        eprint("Tape size cannot be zero");
        return RC_Error;
    }

    if (config->input == NULL) {
        eprint("Input stream cannot be NULL");
        return RC_Error;
    }

    if (config->output == NULL) {
        eprint("Output stream cannot be NULL");
        return RC_Error;
    }

    size_t size_in_bytes = config->tape_size * (size_t) config->cmes;

    if (size_in_bytes > BFI_MAX_TAPE_SIZE) {
        eprintf("Tape size too big. Max size: %u bytes (%u GiB) and requested size: %zu bytes (%zu GiB)", 
            BFI_MAX_TAPE_SIZE, BFI_MAX_TAPE_SIZE / 1024 / 1024 / 1024, size_in_bytes, size_in_bytes / 1024 / 1024 / 1024);
        return RC_Error;
    }

    void *tape = malloc(size_in_bytes);
    if (tape == NULL) {
        eprint("Failed to allocate memory for tape");
        return RC_Error;
    }

    memset(tape, 0, size_in_bytes);
    size_t cursor = 0;

    execute(tape, config, &cursor);
    return RC_Success;
}

 Return_Code execute(void *tape, Config *config, size_t *cursor) {
    static long long level = 0;

    int c = fgetc(config->input);
    while (c != EOF) {
        switch (c) {
            case '>': if (!increment_cursor(config->cmom, cursor, config->tape_size)) return RC_Abort; break;
            case '<': if (!decrement_cursor(config->cmom, cursor, config->tape_size)) return RC_Abort; break;
            case '+': increment_tape_value(tape, config->cmes, *cursor); break;
            case '-': decrement_tape_value(tape, config->cmes, *cursor); break;
            case '.':
                long long value = get_tape_value(tape, config->cmes, *cursor);
                if (config->cmes == CMES_Byte) fputc((char) value, config->output);
                else fprintf(config->output, "%llu (0x%llX)\n", value, value);
                break;
            case ',': {
                long long value = 0;
                for (int i = 0; i < (int) config->cmes; i++) {
                    int ch = fgetc(config->input);
                    if (ch == EOF) {
                        eprint("Unexpected end of file when loading value");
                        return RC_Abort;
                    }
                    value <<= 8;
                    value += ch;
                }
                switch (config->cmes) {
                    case CMES_Byte:  ((char *)      tape)[*cursor] = (char)      value; break;
                    case CMES_Word:  ((short *)     tape)[*cursor] = (short)     value; break;
                    case CMES_DWord: ((int *)       tape)[*cursor] = (int)       value; break;
                    case CMES_QWord: ((long long *) tape)[*cursor] = (long long) value; break;
                }
            }
            break;
            case '[': 
                if (get_tape_value(tape, config->cmes, *cursor) == 0) {
                    c = fgetc(config->input);
                    while (c != ']') {
                        if (c == EOF) {
                            eprint("Unexpected end of file in loop");
                            return RC_Abort;
                        }
                        c = fgetc(config->input);
                    }
                } else {
                    size_t position = ftell(config->input);
                    level++;
                    Return_Code err = execute(tape, config, cursor);
                    if (err != RC_Success) return err;
                    fseek(config->input, position, SEEK_SET);
                    ungetc('[', config->input);
                }
                break;
            case ']':
                if (level == 0) {
                    eprint("Unexpected end of loop");
                    return RC_Abort;
                }

                level--;
                return RC_Success;
            case '#':
                if (config->debug) {
                    dump_tape(tape, config->cmes, config->tape_size, *cursor, config->output);
                }
                break;
        }
        c = fgetc(config->input);
    }
    
    if (level != 0) {
        eprint("Unexpected end of file -- missing ']'");
        return RC_Abort;
    }

    return RC_Success;
}

// CHECK: printing end of tape (end is == size - 1)
void dump_tape(void *tape, CMES imes, size_t size, size_t cursor, FILE *output) {
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