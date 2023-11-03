/**
 * @file usefull_macros.h
 * @author RealTigerCZ
 * @brief Macros for error and debug printing and other useful things 
 * @version 0.1
 * @date 2023-11-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef USEFULL_MACROS_H
#define USEFULL_MACROS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

// Prints error message with file name and line number to stderr
#define eprint(s) fprintf(stderr, "[ERROR] "__FILE__":%u: %s\n", __LINE__, s)

// Prints and formats error message with file name and line number to stderr
#define eprintf(s, ...) fprintf(stderr, "[ERROR] "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)


#ifndef NDEBUG
/* Macros in here are only used in debug mode (compiled with -DNDEBUG) and are required to not be used in release mode. */
/* If any of these are used in release mode, it will result in a compilation error. */

    // Prints debug message with file name and line number to stout
    #define dprint(s) printf("[DEBUG] "__FILE__":%u: %s\n", __LINE__, s)

    // Prints and formats debug message with file name and line number to stout
    #define dprintf(s, ...) printf("[DEBUG] "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

    // Debug print of integer
    #define dprint_int(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

    // Debug print of double
    #define dprint_double(d) printf(" - " __FILE__ ":%u: " #d " = %f\n", __LINE__, d)

    // Debug print of boolean
    #define dprint_bool(b) printf(" - " __FILE__ ":%u: " #b " = %s\n", __LINE__, b ? "true" : "false")

#endif // !NDEBUG 

#endif // USEFULL_MACROS_H