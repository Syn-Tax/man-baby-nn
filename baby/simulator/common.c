#include <common.h>
#include <ctype.h>

static size_t spaceLen(char* inStr) {
	/* This is to verify that our string is the right length, excluding whitespace soas to support files put together with windows line endings */
	size_t i = 0;
	if(!inStr) {
		return 0;
	}
	while(inStr[i] && !isspace(inStr[i])) {
		i += 1;
	}
	return i;
}

static int convertBinToUint32(char* inStr, uint32_t* output) {

	/* Make sure the input String is the right length */
	if(spaceLen(inStr) != 32) {
		error("Line length wrong");
		return -1;
	}
	for(uint32_t i = 0; i < 32; i += 1) {
		if(inStr[i] < '0' || inStr[i] > '1') {
			error("Line contains character other than 0 or 1");
			return -1;
		}

		/* If the character is between 0 and 1, it's either '0' or '1'. When we subtract '0' the output is the number 0 or 1, rather than the ASCII representation. */
		*output |= (inStr[i] - '0') << i;
	}
	return 0;
}


int loadFile(char* filename) {
	int ret;

	/* Open the file and read it into a buffer */
	FILE* f = fopen(filename, "rb");
	if(!f) {
		error("Failed to open file");
		return -1;
	}

	ret = fseek(f, 0, SEEK_END);
	if(ret != 0) {
		fclose(f);
		error("Failed to seek through file");
		return -1;
	}

	size_t len = ftell(f);
	if(len == 0) {
		fclose(f);
		error("File is empty");
		return -1;
	}

	ret = fseek(f, 0, SEEK_SET);
	if(ret != 0) {
		fclose(f);
		error("Failed to seek through file");
		return -1;
	}

	char* buf = malloc(len);
	if(!buf) {
		error("Failed to allocate memory for file");
		fclose(f);
		return -1;
	}
	size_t read = fread(buf, 1, len, f);
	fclose(f);
	if(read != len) {
		error("Failed to read file");
		return -1;
	}

	/* Use strtok to read through each line */
	char* line = strtok(buf, "\n");
	unsigned long i = 0;
	while(line) {
		/* Ensure we're not reading outside of memory space */
		if(i >= memoryLen) {
			error("Program too long for memory");
			return -1;
		}

		/* Convert each line into a uint32_t and write into the memory location */
		ret = convertBinToUint32(line, &memory[i]);
		if(ret != 0) {
			error("Failed to convert line to binary");
			return -1;
		}
		line = strtok(NULL, "\n");
		i += 1;
	}
	free(buf);
	return 0;
}

static void printMemoryLine(uint32_t line) {
		for(int j = 0; j < 32; j += 1) {
			if((line >> j) & 1) {
				printf(YELLOW_TERM "O" NORMAL_TERM);
			}
			else {
				printf(RED_TERM "." NORMAL_TERM);
			}
		}
		printf("\n");
}

void printMemory() {
	/* Note we're onto the next instruction */
	printf("******************************\n");

	/* Print the value of memory */
	for(uint32_t i = 0; i < memoryLen; i += 1) {
		if(i == registers[1]) {
			printf("--> ");
		}
		printMemoryLine(memory[i]);
	}

	/* Print the value of the accumulator */
	printf("Accumulator: ");
	printMemoryLine(registers[0]);

	/* Print the value of the program counter */
	printf("CI: ");
	printMemoryLine(registers[1]);

	/* Print the value of the PI */
	printf("PI: ");
	printMemoryLine(registers[3]);

}


static char* instructionSetMnemonics[] = {
	"JMP",
	"JRP",
	"LDN",
	"STO",
	"SUB",
	"SUB",
	"CMP",
	"STP",
	"STOACC",
	"ADD",
	"MUL",
	"DIV",
	"REM",
	"AND",
	"ORR",
	"XOR",
	"LSL",
	"RSR",
	"LDR",
	"PUSHACC",
	"POPACC",
	"PUSHLR",
	"RET",
	"TST",
	"JEQ",
	"JNE",
	"JGT",
	"JGE",
	"JLT",
	"JLE",
	"JAD",
	"LDRACC",
	"ADDF",
	"SUBF",
	"MULF",
	"DIVF"
};

void printDecode() {
	babyinsn* insn = (babyinsn*)&registers[3];
	/* Print our operator and operand */
	printf("Operator: ");
	printMemoryLine(insn->opcode);
	printf("Mnemonic: %s\n", instructionSetMnemonics[insn->opcode]);

	printf("Operand: ");
	printMemoryLine(insn->operand);
}
