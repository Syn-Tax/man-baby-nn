#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <instructions.h>

#if DEBUG == 1
#define NORMAL_TERM
#define RED_TERM
#define YELLOW_TERM
#define dprintf printf
#else
#define NORMAL_TERM  "\x1B[0m"
#define RED_TERM  "\x1B[31m"
#define YELLOW_TERM  "\x1B[33m"
#define dprintf(...) 
#endif

#ifndef bzero
#define bzero(x, y) memset(x, '\0', y)
#endif

#define error(errormsg) fprintf(stderr, "Error in %s():%s:%d, \"%s\"\n", __func__, __FILE__, __LINE__, errormsg)


typedef struct {
	uint16_t operand : 12;
	uint8_t imm : 1;
	uint8_t opcode : 6;
	uint32_t pad : 12;
} __attribute__((packed)) babyinsn;

extern uint32_t registers[4]; /* Accumulator is 0, PC is 1, LR (extension) is 2, PI is 3 */
extern uint32_t* memory;
extern unsigned long memoryLen;

int loadFile(char* filename);
void printMemory();
void printDecode();

#endif
