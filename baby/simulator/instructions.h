#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define NUM_BASIC_INSTRUCTIONS 8
#define NUM_INSTRUCTIONS 36

typedef int (*operator)(uint16_t operand, uint8_t imm);
extern operator instructionSet[NUM_INSTRUCTIONS];

#endif
