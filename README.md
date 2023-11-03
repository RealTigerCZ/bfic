# bfic
Brainf*ck interpreter and compiler written in C (it's WIP and not intended for production use)

## Quick start 
```console
$ make
$ ./bfic [options]
```

## Runing options
### Default settings
 - file input stream: `stdin`
 - file output stream: `stdout` if interpreting, `a.out` if compiling
 - tape size: `1024 elements`
 - debug mode: `off`
 - output mode: `compile`
 - memory overflow mode: `Do nothing`
 - memory element size: `byte`
 - verbose output: `off`

### Arguments
 - `-[h|?]` or `--help` - print help
 - `FILE` - set input stream to `FILE`
 - `-o FILE` - set output stream to `FILE`
 - `-v` or `--verbose` - enable verbose output
 - `-d` or `--debug` - enable debug mode
 - `-i` or `--interpret` - set output mode to `interpret`
 - `--mem-overflow=[wrap|abort]` or `--mem-overflow [wrap|abort]` - set memory overflow mode
 - `--mem-element-size=TYPE` or `--mem-elelement-size TYPE` - set memory element size, where TYPE can be:
   - `byte`  - 1 byte  (char or int8_t)
   - `word`  - 2 bytes (int16_t)
   - `dword` - 4 bytes (int32_t)
   - `qword` - 8 bytes (int64_t)
 - `--tape-size=SIZE` or `--tape-size SIZE` - set tape size to SIZE
 - `--version` - print version
 - `--count` - Count the number of instructions executed (slow)
 - `--end=char` or `--end char` - set the end of the input stream to `char` 
 - `--escape-input` - escape the input stream (so you can read bytes like this `,\x0A)

## Instruction set

| Instruction | Description |
|----------|----------|
| '>' | Increment the data pointer by one (to point to the next element to the right). |
| '<' | Decrement the data pointer by one (to point to the next element to the left). |
| '+' | Increment the element at the data pointer by one. |
| '-' | Decrement the element at the data pointer by one. |
| '.' | Print the element at the data pointer. (if memory element size is byte prints as ascii otherwise prints as decimal ) |
| ',' | Reads incoming bytes (number of bytes depends on memory element size) and stores them in the element at the data pointer.|
| '[' | If the element at the data pointer is zero, then instead of moving the instruction pointer forward to the next command, jump it forward to the command after the matching ] command. |
| ']' | If the element at the data pointer is nonzero, then instead of moving the instruction pointer forward to the next command, jump it back to the command after the matching '[' command.|
| '#' | If the debug mode is enabled, print the current pointer position and dump the tape. |

Any other character is ignored (unless the --end option is set).

## Examples

```console
$ ./bfic.out -o hello.out hello_world.bf
$ ./hello.out
Hello, World!
```
*hello_world.bf is provided in the `examples` directory*

```console
$ ./bfic.out -i hello_world.bf
Hello, World!
```

## Goals
 - [x] Basic prototype (unoptimized interpreter)
 - [ ] Better options parser
 - [ ] Basic compiler
 - [ ] Bytecode generator and interpreter (optimized)
 - [ ] Optimized compiler
 - [ ] 100% Up to spec + good documentation (never will be)



Inspired by [this great online bf interpreter](https://copy.sh/brainfuck/).
Learn more about BF on [Wikipedia](https://en.wikipedia.org/wiki/Brainfuck).
