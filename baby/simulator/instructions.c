#include <common.h>
#include <instructions.h>
#include <stack.h>

static uint32_t flags; /* bit 0 for ==, bit 1 for lt, bit 2 for gt */

static int jmp(uint16_t operand, uint8_t imm) {
	/* JMP doesn't allow for IMM addressing */
	if(imm) {
		error("JMP doesn't support immidiate addressing");
		return -1;
	}
	/* Set the pc to the operand provided */
	registers[2] = registers[1];
	registers[1] = operand - 1;
	dprintf("JMP %d\n", operand);

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}
	
	return 0;
}

static int jrp(uint16_t operand, uint8_t imm) {
	/* JRP doesn't allow for IMM addressing */
	if(imm) {
		error("JRP doesn't support immidiate addressing");
		return -1;
	}
	/* Increase the pc by the operand provided */
	registers[2] = registers[1];
	registers[1] = registers[1] + operand - 1;

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	dprintf("JRP %d\n", operand);
	return 0;
}

static int ldn(uint16_t operand, uint8_t imm) {
	if(imm) {
		registers[0] = -operand;
		dprintf("LDN %d\n", operand);
		return 0;
	}
	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Load into the accumulator from the negated the memory location requested by our operand */
	registers[0] = -memory[operand];
	dprintf("LDN %d\n", operand);
	return 0;
}

static int sto(uint16_t operand, uint8_t imm) {
	/* STO doesn't allow for IMM addressing */
	if(imm) {
		error("STO doesn't support immidiate addressing");
		return -1;
	}
	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Store the accumulator in the memory location requested by our operand */
	memory[operand] = registers[0];
	dprintf("STO %d\n", operand);
	return 0;
}

static int sub(uint16_t operand, uint8_t imm) {
	if(imm) {
		registers[0] -= operand;
		dprintf("SUB %d\n", operand);
		return 0;
	}
	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* subtract what's found in the memory location requested by our operand from the accumulator */
	registers[0] -= memory[operand];
	dprintf("SUB %d\n", operand);
	return 0;
}

static int cmp(uint16_t operand, uint8_t imm) {
	/* CMP doesn't allow for IMM addressing */
	if(imm) {
		error("CMP doesn't support immidiate addressing");
		return -1;
	}
	/* cmp must have a 0 operand */
	if(operand != 0) {
		error("CMP must have 0 operand");
		return -1;
	}
	if((int32_t)registers[0] < 0) {
		registers[1] += 1;
	}
	dprintf("CMP %d\n", operand);
	return 0;
}

static int stp(uint16_t operand, uint8_t imm) {

	/* STP doesn't allow for IMM addressing */
	if(imm) {
		error("STP doesn't support immidiate addressing");
		return -1;
	}

	/* stp must have a 0 operand */
	if(operand != 0) {
		error("STP must have 0 operand");
		return -1;
	}
	dprintf("STP %d\n", operand);
	/* We return 1 because we've hardcoded a return value of 1 to instruct the program to return */
	return 1;
}

static int stoacc(uint16_t operand, uint8_t imm) {

	if(registers[0] >= memoryLen) {
		error("Program out of bounds");
		return -1;
	}
	if(!imm) {
		if(operand >= memoryLen) {
			error("Program out of bounds");
			return -1;
		}
		memory[registers[0]] = memory[operand];
	}
	else {
		memory[registers[0]] = operand;
	}
	dprintf("STOACC %d\n", operand);
	return 0;
}

static int add(uint16_t operand, uint8_t imm) {
	if(imm) {
		registers[0] += operand;
		dprintf("ADD %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* add what's found in the memory location requested by our operand from the accumulator */
	registers[0] += memory[operand];
	dprintf("ADD %d\n", operand);
	return 0;
}

static int mul(uint16_t operand, uint8_t imm) {
	if(imm) {
		registers[0] *= operand;
		dprintf("MUL %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* multiply what's found in the memory location requested by our operand from the accumulator */
	registers[0] *= memory[operand];
	dprintf("MUL %d\n", operand);
	return 0;
}

static int div_insn(uint16_t operand, uint8_t imm) {
	if(imm) {
		registers[0] /= operand;
		dprintf("DIV %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}
	/* Verify we're not going to divide by zero */
	if(memory[operand] == 0) {
 		error("Divide by zero error.");
 		return -1;
	}
	/* divide what's found in the memory location requested by our operand from the accumulator */

	registers[0] /= memory[operand];
	dprintf("DIV %d\n", operand);
	return 0;
}

static int rem(uint16_t operand, uint8_t imm) {

	if(imm) {
		registers[0] %= operand;
		dprintf("REM %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Get the remainder of what's found in the memory location requested by our operand from the accumulator */
	registers[0] %= memory[operand];
	dprintf("REM %d\n", operand);
	return 0;
}

static int and(uint16_t operand, uint8_t imm) {

	if(imm) {
		registers[0] &= operand;
		dprintf("AND %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Get the binary and of what's found in the memory location requested by our operand from the accumulator */
	registers[0] &= memory[operand];
	dprintf("AND %d\n", operand);
	return 0;
}

static int orr(uint16_t operand, uint8_t imm) {

	if(imm) {
		registers[0] |= operand;
		dprintf("ORR %d\n", operand);
		return 0;
	}


	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Get the binary or of what's found in the memory location requested by our operand from the accumulator */
	registers[0] |= memory[operand];
	dprintf("ORR %d\n", operand);
	return 0;
}


static int xor(uint16_t operand, uint8_t imm) {

	if(imm) {
		registers[0] ^= operand;
		dprintf("XOR %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Get the binary xor of what's found in the memory location requested by our operand from the accumulator */
	registers[0] ^= memory[operand];
	dprintf("XOR %d\n", operand);
	return 0;
}

static int lsl(uint16_t operand, uint8_t imm) {

	if(imm) {
		registers[0] <<= operand;
		dprintf("LSL %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* left shift the accumulator based on the memory address referenced by the operand */
	registers[0] <<= memory[operand];
	dprintf("LSL %d\n", operand);
	return 0;
}

static int rsr(uint16_t operand, uint8_t imm) {

	if(imm) {
		registers[0] >>= operand;
		dprintf("RSR %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* right shift the accumulator based on the memory address referenced by the operand */
	registers[0] >>= memory[operand];
	dprintf("RSR %d\n", operand);
	return 0;
}

static int ldr(uint16_t operand, uint8_t imm) {

	if(imm) {
		registers[0] = operand;
		dprintf("LDR %d\n", operand);
		return 0;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Load into the accumulator from the memory location requested by our operand */
	registers[0] = memory[operand];
	dprintf("LDR %d\n", operand);
	return 0;
}

static int pushacc(uint16_t operand, uint8_t imm) {

	/* PUSHACC doesn't allow for IMM addressing */
	if(imm) {
		error("PUSHACC doesn't support immidiate addressing");
		return -1;
	}

	/* pushacc must have a 0 operand */
	if(operand != 0) {
		error("PUSHACC must have 0 operand");
		return -1;
	}

	/* push the value of the accumulator onto the stack */
	if(push(registers[0]) != 0) {
		error("Failed to push the accumulator onto the stack");
		return -1;
	}
	dprintf("PUSHACC %d\n", operand);
	return 0;
}

static int popacc(uint16_t operand, uint8_t imm) {

	/* POPACC doesn't allow for IMM addressing */
	if(imm) {
		error("POPACC doesn't support immidiate addressing");
		return -1;
	}

	/* popacc must have a 0 operand */
	if(operand != 0) {
		error("POPACC must have 0 operand");
		return -1;
	}

	/* pop the value of the accumulator onto the stack */
	if(pop(&registers[0]) != 0) {
		error("Failed to pop the accumulator from the stack");
		return -1;
	}
	dprintf("POPACC %d\n", operand);
	return 0;
}

static int pushlr(uint16_t operand, uint8_t imm) {

	/* PUSHLR doesn't allow for IMM addressing */
	if(imm) {
		error("PUSHLR doesn't support immidiate addressing");
		return -1;
	}

	/* pushlr must have a 0 operand */
	if(operand != 0) {
		error("PUSHLR must have 0 operand");
		return -1;
	}

	/* push the value of the link register onto the stack */
	if(push(registers[2]) != 0) {
		error("Failed to push the link register onto the stack");
		return -1;
	}
	dprintf("PUSHLR %d\n", operand);
	return 0;
}

static int ret(uint16_t operand, uint8_t imm) {

	/* RET doesn't allow for IMM addressing */
	if(imm) {
		error("RET doesn't support immidiate addressing");
		return -1;
	}

	/* ret must have a 0 operand */
	if(operand != 0) {
		error("RET must have 0 operand");
		return -1;
	}

	/* pop the value of the program counter from the stack */
	if(pop(&registers[1]) != 0) {
		error("Failed to pop the program counter from the stack");
		return -1;
	}
	dprintf("RET %d\n", operand);
	return 0;
}

static int tst(uint16_t operand, uint8_t imm) {

	/* Get the value we're comparing to */
	int32_t comparison = (int32_t)operand;
	if(!imm) {
		/* Verify that the operand is within the memory space */
		if(operand >= memoryLen) {
 			error("Program out of bounds");
 			return -1;
		}
		comparison = (int32_t)memory[operand];
	}
	
	flags = 0;
	if(comparison == (int32_t)registers[0]) {
		flags |= 1;
	}
	else if((int32_t)registers[0] < comparison) {
		flags |= 2;
	}
	else {
		flags |= 4;
	}
	dprintf("TST %d\n", operand);
	return 0;
}

static int jeq(uint16_t operand, uint8_t imm) {

	/* JEQ doesn't allow for IMM addressing */
	if(imm) {
		error("JEQ doesn't support immidiate addressing");
		return -1;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Check the flags for whether the previous tst assured us the values were == */
	if(flags & 1) {
		/* Set the pc to the operand provided */
		registers[2] = registers[1];
		registers[1] = operand - 1;
	}

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}
	
	dprintf("JEQ %d\n", operand);
	return 0;
}

static int jne(uint16_t operand, uint8_t imm) {

	/* JNE doesn't allow for IMM addressing */
	if(imm) {
		error("JNE doesn't support immidiate addressing");
		return -1;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Check the flags for whether the previous tst assured us the values were == */
	if(!(flags & 1)) {
		/* Set the pc to the operand provided */
		registers[2] = registers[1];
		registers[1] = operand - 1;
	}

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	dprintf("JNE %d\n", operand);
	return 0;
}

static int jgt(uint16_t operand, uint8_t imm) {

	/* JGT doesn't allow for IMM addressing */
	if(imm) {
		error("JGT doesn't support immidiate addressing");
		return -1;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Check the flags for whether the previous tst assured us the values were > */
	if((flags & 4)) {
		/* Set the pc to the operand provided */
		registers[2] = registers[1];
		registers[1] = operand - 1;
	}

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	dprintf("JGT %d\n", operand);
	return 0;
}

static int jge(uint16_t operand, uint8_t imm) {

	/* JGE doesn't allow for IMM addressing */
	if(imm) {
		error("JGE doesn't support immidiate addressing");
		return -1;
	}


	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Check the flags for whether the previous tst assured us the values were >= */
	if((flags & 4) | (flags & 1)) {
		/* Set the pc to the operand provided */
		registers[2] = registers[1];
		registers[1] = operand - 1;
	}

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	dprintf("JGE %d\n", operand);
	return 0;
}

static int jlt(uint16_t operand, uint8_t imm) {

	/* JLT doesn't allow for IMM addressing */
	if(imm) {
		error("JLT doesn't support immidiate addressing");
		return -1;
	}

	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Check the flags for whether the previous tst assured us the values were < */
	if((flags & 2)) {
		/* Set the pc to the operand provided */
		registers[2] = registers[1];
		registers[1] = operand - 1;
	}

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	dprintf("JLT %d\n", operand);
	return 0;
}

static int jle(uint16_t operand, uint8_t imm) {

	/* JLE doesn't allow for IMM addressing */
	if(imm) {
		error("JLE doesn't support immidiate addressing");
		return -1;
	}


	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Check the flags for whether the previous tst assured us the values were <= */
	if((flags & 2) | (flags & 1)) {
		/* Set the pc to the operand provided */
		registers[2] = registers[1];
		registers[1] = operand - 1;
	}

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	dprintf("JLE %d\n", operand);
	return 0;
}

static int jad(uint16_t operand, uint8_t imm) {

	/* JAD doesn't allow for IMM addressing */
	if(imm) {
		error("JAD doesn't support immidiate addressing");
		return -1;
	}


	/* Verify that the operand is within the memory space */
	if(operand >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	/* Set the pc to the memory address referenced by the operand provided */
	registers[2] = registers[1];
	registers[1] = memory[operand] - 1;

	/* Verify that the pc is within the memory space */
	if(registers[1] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}

	dprintf("JAD %d\n", operand);
	return 0;
}

static int ldracc(uint16_t operand, uint8_t imm) {

	/* LDRACC doesn't allow for IMM addressing */
	if(imm) {
		error("LDRACC doesn't support immidiate addressing");
		return -1;
	}

	/* LDRACC must have a 0 operand */
	if(operand != 0) {
		error("LDRACC must have 0 operand");
		return -1;
	}
	if(registers[0] >= memoryLen) {
 		error("Program out of bounds");
 		return -1;
	}
	registers[0] = memory[registers[0]];
	dprintf("LDRACC %d\n", operand);
	return 0;
}

typedef union {
	int i;
	float f;
} Fint;

Fint fint;

static int addf(uint16_t operand, uint8_t imm) {
	fint.i = registers[0];
	float acc = fint.f;

	float op;

	if (imm) {
		fint.i = operand;
		op = fint.f;
	} else {
		if (operand >= memoryLen && !imm) {
			error("Program out of bounds");
			return -1;
		}
		fint.i = memory[operand];
		op = fint.f;
	}

	float res = acc + op;
	fint.f = res;
	registers[0] = fint.i;

	dprintf("ADDF %d\n", operand);

	return 0;
}


static int subf(uint16_t operand, uint8_t imm) {
	fint.i = registers[0];
	float acc = fint.f;

	float op;

	if (imm) {
		fint.i = operand;
		op = fint.f;
	} else {
		if (operand >= memoryLen && !imm) {
			error("Program out of bounds");
			return -1;
		}
		fint.i = memory[operand];
		op = fint.f;
	}

	float res = acc - op;
	fint.f = res;
	registers[0] = fint.i;

	dprintf("SUBF %d\n", operand);

	return 0;
}

static int mulf(uint16_t operand, uint8_t imm) {
	fint.i = registers[0];
	float acc = fint.f;

	float op;

	if (imm) {
		fint.i = operand;
		op = fint.f;
	} else {
		if (operand >= memoryLen && !imm) {
			error("Program out of bounds");
			return -1;
		}
		fint.i = memory[operand];
		op = fint.f;
	}

	float res = acc * op;
	fint.f = res;
	registers[0] = fint.i;

	dprintf("MULF %d\n", operand);

	return 0;
}

static int divf(uint16_t operand, uint8_t imm) {
	fint.i = registers[0];
	float acc = fint.f;

	float op;

	if (imm) {
		fint.i = operand;
		op = fint.f;
	} else {
		if (operand >= memoryLen && !imm) {
			error("Program out of bounds");
			return -1;
		}
		fint.i = memory[operand];
		op = fint.f;
	}

	float res = acc / op;
	fint.f = res;
	registers[0] = fint.i;

	dprintf("DIVF %d\n", operand);

	return 0;
}


/* We hold the instructions in a function table so that we can use the opcode to jump to the appropriate instruction */
operator instructionSet[NUM_INSTRUCTIONS] = {
	jmp,
	jrp,
	ldn,
	sto,
	sub,
	sub,
	cmp,
	stp,
	stoacc,
	add,
	mul,
	div_insn,
	rem,
	and,
	orr,
	xor,
	lsl,
	rsr,
	ldr,
	pushacc,
	popacc,
	pushlr,
	ret,
	tst,
	jeq,
	jne,
	jgt,
	jge,
	jlt,
	jle,
	jad,
	ldracc,
	addf,
	subf,
	mulf,
	divf
};
