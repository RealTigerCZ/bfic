/**
 * @file bfic.c
 * @author RealTigerCZ
 * @brief Brainf*ck interpreter and compiler main file (arguments handling)
 * @version 0.1.1
 * @date 2023-11-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#define safe_return(expr) ret_val = (expr); goto safe_return;

#include "src/interpreter.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* TODO: Many things
 * - comment code
 * - add usage
 * - add support for all options specified in README.md
 * - refactor options parser
 * - escaping input support
 * - bytecode support
*/

const char *USAGE = "   bfic [-o output_file] [-i] [input_file] []";
const size_t DEF_TAPE_SIZE = 1024;

int main(int argc, char *argv[]) {
    int ret_val;

    Config config = {
        .cmom = CMOM_Default,
        .cmes = CMES_Byte,
        .tape_size = 0,
        .debug = false,
        .verbose = false,
        .count = false,
        .end_char = 0,
        .input = stdin,
        .output = stdout
    };


    bool imes_set = false;
    bool interpret = false;


    if (argc < 2) {
        printf("[INFO] Reading input from stdin...\n");
    } else {
        for (int i = 1; i < argc; i++) {
            if (argv[i][0] == '-') {
                switch (argv[i][1]) {
                    case 'o':
                        if (config.output != stdout) {
                            eprint("Multiple output files specified");
                            safe_return(1);
                        }

                        if (i + 1 >= argc) {
                            eprint("Missing output file for option '-o'");
                            safe_return(1);
                        }

                        if ((config.output = fopen(argv[i + 1], "w")) == NULL) {
                            eprintf("Failed to open output file: %s, %s", argv[i + 1], strerror(errno));
                            safe_return(1);
                        };
                        break;
                    case 'i': interpret = true; break;
                    case 'h':
                    case '?':
                        // TODO: Add help
                        eprint("Help not implemented yet");
                        safe_return(0);

                    case 'v':
                        // TODO: Add vebose mode
                        eprint("Verbose mode not implemented yet");
                        safe_return(0);

                    case 'd': config.debug = true; break;

                    case '-':
                        if (strncmp(argv[i], "--help", 7) == 0) {
                            // TODO: Add help
                            eprint("Help not implemented yet");
                            safe_return(0);
                        }

                        if (strncmp(argv[i], "--version", 10) == 0) {
                            // TODO: Add version
                            eprint("Version not implemented yet");
                            safe_return(0);
                        }

                        if (strncmp(argv[i], "--debug", 8) == 0) {
                            config.debug = true;
                            break;
                        }

                        if (strncmp(argv[i], "--interpret", 12) == 0) {
                            interpret = true;
                            break;
                        }

                        if (strncmp(argv[i], "--tape-size", 11) == 0) {
                            printf("[INFO] Setting tape size to %s\n", argv[i] + 12);
                            if (argv[i][11] == '=') {
                                // TODO: Add tape size
                                eprint("Tape size not implemented yet");
                                safe_return(0);
                            } else if (i + 1 >= argc) {
                                eprint("Missing tape size for option '--tape-size'");
                                safe_return(1);
                            }

                            // TODO: Add tape size
                            eprint("Tape size not implemented yet");
                            safe_return(0);
                        }

                        if (strncmp(argv[i], "--mem-overflow", 14) == 0) {
                            if (config.cmom != CMOM_Default) {
                                eprint("Multiple memory overflow modes specified");
                                safe_return(1);
                            }

                            if (argv[i][15] == '=') {
                                if (strncmp(argv[i] + 16, "wrap", 5) == 0) {
                                    config.cmom = CMOM_Wrap;
                                } else if (strncmp(argv[i] + 16, "abort", 6) == 0) {
                                    config.cmom = CMOM_Abort;
                                } else {
                                    eprintf("Unknown memory overflow mode '%s'", argv[i] + 16);
                                    fprintf(stderr, "[INFO] possible memory overflow modes: wrap, abort\n");
                                    safe_return(1);
                                }

                            } else if (i + 1 >= argc) {
                                i++;
                                if (strncmp(argv[i], "wrap", 5) == 0) {
                                    config.cmom = CMOM_Wrap;
                                } else if (strncmp(argv[i], "abort", 6) == 0) {
                                    config.cmom = CMOM_Abort;
                                } else {
                                    eprintf("Unknown memory overflow mode '%s'", argv[i]);
                                    fprintf(stderr, "[INFO] possible memory overflow modes: wrap, abort\n");
                                    safe_return(1);
                                }
                            } else {
                                eprint("Missing memory overflow mode for option '--mem-overflow'");
                                safe_return(1);
                            }
                        }
                        if (strncmp(argv[i], "--mem-element-size", 18) == 0) {
                            if (imes_set) {
                                eprint("Multiple memory element sizes specified");
                                safe_return(1);
                            }
                            // FIXME: does not support "--mem-element-size type"
                            if (argv[i][18] == '=') {
                                if (strncmp(argv[i] + 20, "byte", 5) == 0) {
                                    config.cmes = CMES_Byte;
                                } else if (strncmp(argv[i] + 20, "word", 5) == 0) {
                                    config.cmes = CMES_Word;
                                } else if (strncmp(argv[i] + 20, "dword", 6) == 0) {
                                    config.cmes = CMES_DWord;
                                } else if (strncmp(argv[i] + 20, "qword", 6) == 0) {
                                    config.cmes = CMES_QWord;
                                } else {
                                    eprintf("Unknown memory element size '%s'", argv[i] + 20);
                                    fprintf(stderr, "[INFO] possible memory element sizes: byte, word, dword, qword\n");
                                    safe_return(1);
                                }
                                imes_set = true;
                                break;
                            }
                        }

                        eprintf("Unknown option '%s'", argv[i]);
                        safe_return(1);
                        
                    default:
                        eprintf("Unknown option '-%c'", argv[i][1]);
                        safe_return(1);
                }
            } else {
                if (config.input != stdin) {
                    eprint("Multiple input files specified");
                    safe_return(1);
                }

                if ((config.input = fopen(argv[i], "r")) == NULL) {
                    eprintf("Failed to open input file: %s, %s", argv[i], strerror(errno));
                    safe_return(1);
                }
            } 

        }
    }

    config.tape_size = config.tape_size ? config.tape_size : DEF_TAPE_SIZE;
    if (interpret) {
        safe_return(run_interpreter(&config) != RC_Success);
    }

    eprint("Compiler not implemented yet");
    safe_return(1);

safe_return:
    if (config.input != stdin) fclose(config.input);
    if (config.output != stdout) fclose(config.output);
    return ret_val;
}