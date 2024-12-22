#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>


#ifndef strdup
#define strdup strdup_real
#endif

#define dprintf printf

#ifndef bzero
#define bzero(x, y) memset(x, '\0', y)
#endif

#define error(errormsg) fprintf(stderr, "Error in %s():%s:%d, \"%s\"\n", __func__, __FILE__, __LINE__, errormsg)
#define errorWithLine(errormsg) fprintf(stderr, "Error on line %ld: %s\n", lineMap[i], errormsg);

#define NUM_BASIC_INSTRUCTIONS 8
#define NUM_INSTRUCTIONS 36


typedef struct {
	uint16_t operand : 12;
	uint8_t imm : 1;
	uint8_t opcode : 6;
	uint32_t pad : 12;
} __attribute__((packed)) babyinsn;

static char* strdup_real(const char* inStr) {
	char* outString = NULL;
	if(!inStr) {
		return NULL;
	}
	size_t inStrLen = strlen(inStr);
	if(inStrLen == 0) {
		return NULL;
	}
	outString = malloc(inStrLen + 1);
	if(!outString) {
		return NULL;
	}
	strcpy(outString, inStr);
	return outString;
}

char* nonWhiteSearch(char* searchString) {
	int i = 0;
	if(!searchString) {
		return NULL;
	}
	while(searchString[i]) {
		/* If we've found a character that isn't whitespace, return it */
		if(!isspace(searchString[i])) {
			return &searchString[i];
		}
		i += 1;
	}
	/* Return NULL if no such character exists */
	return NULL;
}

int whiteSearch(char* searchString) {
	int i = 0;
	if(!searchString) {
		return 0;
	}
	while(searchString[i]) {
		/* If we've found a character that is whitespace, return true */
		if(isspace(searchString[i])) {
			return 1;
		}
		i += 1;
	}
	/* Return 0 if no such character exists */
	return 0;
}


int nonWhiteGet(char* inString, char** outString, char** commandEnd) {
	int i = 0;
	if(!inString) {
		return -1;
	}
	/* Allocate memory for the string */
	*outString = malloc(strlen(inString));
	if(!*outString) {
		error("Out of memory");
		return -1;
	}
	/* Zero this string so that we don't need to add a null terminator seperately */
	bzero(*outString, strlen(inString));
	while(inString[i]) {
		/* If we've reached whitespace */
		if(isspace(inString[i])) {
			/* Set the end of command variable to this whitespace */
			*commandEnd = &inString[i];
			/* Return success (0) if we've placed at least one character in the outString, otherwise we've failed */
			return i == 0;
		}
		/* Copy character to the outString */
		(*outString)[i] = inString[i];
		i += 1;
	}
	return 0;
}

int isInteger(char* inString) {
	int i = 0;
	/* Verify we've been sent an acceptable String */
	if(!inString || strlen(inString) < 1) {
		error("String too short to be an integer");
		return 0;
	}
	/* If the first character is -, that's acceptable - there's no issue with negative integers */
	if(inString[0] == '-') {
		inString += 1;
	}
	while(inString[i]) {
		/* If the string character isn't a number 0 - 9 return false */
		if(inString[i] < '0' || inString[i] > '9') {
			return 0;
		}
		i += 1;

	}
	/* Else return true */
	return 1;
}

size_t countChars(char* inString, char character) {
	int count = 0;
	int i = 0;
	if(!inString) {
		return 0;
	}
	/* Loop through the string counting the instances of your character */
	while(inString[i]) {
		if(inString[i] == character) {
			count += 1;
		}
		i += 1;
	}
	return count;
}

char* instructionSet[] = {
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

char* noOperandInstructions[] = {
	"CMP",
	"STP",
	"PUSHACC",
	"POPACC",
	"PUSHLR",
	"LDRACC",
	"RET",
};

char* noIMMInstructions[] = {
	"STO",
	"JMP",
	"JRP",
	"JMP",
	"JEQ",
	"JNE",
	"JGT",
	"JGE",
	"JLT",
	"JLE",
	"JAD",
};

/* The arguments we take as input to the program and whether they require an argument */
static struct option arguments[] = {
	{"load", required_argument, NULL, 'l'},
	{"output", required_argument, NULL, 'o'},
	{"extend_instruction_set", no_argument, NULL, 'e'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0}
};


void printArgs(char** argv) {
	printf("Usage: %s [OPTIONS]\n", argv[0]);
	printf("\t-h, --help, view this help dialog.\n");
	printf("\t-l, --load <file>, specify the file you wish to load\n");
	printf("\t-o, --output <file>, Set the output file, defaults to outputting to the command line\n");	
	printf("\t-e, --extend_instruction_set, Enable our extended instruction set.\n");
	printf("\t-i, --enable_imm_addressing, Enable immidiate addressing.\n");

}


int main(int argc, char** argv) {
	int ret = 0;

	/* Setup default variables */
	int supportImm = 0;
	int extendedInsnSet = 0;
	char* filePath = NULL;
	int opt = 0;
	FILE* outFile = stdout;
	while((opt = getopt_long(argc, argv, "iehl:o:", arguments, NULL)) > 0) {
		switch(opt) {
		case 'l':
			/* Verify we have an argument */
			if(!optarg) {
				printArgs(argv);
				return -1;
			}
			filePath = optarg;
			break;
		case 'e':
			extendedInsnSet = 1;
			break;
		case 'i':
			supportImm = 1;
			break;
		case 'o':
			/* Verify we have an argument */
			if(!optarg) {
				printArgs(argv);
				return -1;
			}
			outFile = fopen(optarg, "wb");
			if(!outFile) {
				error("Failed to open output file");
				return -1;
			}
			break;
		case 'h':
		default:
			printArgs(argv);
			return 0;
		}
	}
	if(!filePath) {
		printArgs(argv);
		return -1;
	}
	/* If we don't have the extended instruction set, remove them from our array */
	if(!extendedInsnSet) {
		bzero(&instructionSet[NUM_BASIC_INSTRUCTIONS], sizeof(char*) * (NUM_INSTRUCTIONS - NUM_BASIC_INSTRUCTIONS));
	}

	/* Load our file */
	FILE* f = fopen(filePath, "rb");
	if(!f) {
		error("Failed to open input file");
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

	printf("Program has been read in:\n%s\n", buf);
	
	/* Count how many lines we have in the program */
	size_t lineCount = countChars(buf, '\n');
	if(lineCount <= 0) {
		error("Failed to count file lines");
		return -1;
	}

	/* Read those lines into our array and setup our lineMap to allow us to give error numbers for specific lines */

	size_t i = 0;
	size_t realLineNum = 0;
	size_t lineMap[lineCount];
	char* lines[lineCount];
	char* line = buf;
	do {
		size_t lineLen = strlen(line);
		if(strchr(line, '\n')) {
			lineLen = strchr(line, '\n') - line;
		}
		char* workingLine = malloc(lineLen + 1);
		strncpy(workingLine, line, lineLen);
		workingLine[lineLen] = '\0';
		realLineNum += 1;
		/* Remove comments as we read the lines in */
		char* semicolon = strstr(workingLine, ";");
		if(semicolon) {
			semicolon[0] = '\0';
		}

		/* If the line is now empty, don't add it to the array */
		if(!nonWhiteSearch(workingLine) ) {
			continue;
		}

		/* Ensure the line doesn't start with whitespace */
		lines[i] = nonWhiteSearch(workingLine);
		lineMap[i] = realLineNum;
		i += 1;
	} while ((line = strchr(line, '\n') + 1) != (char*)1);

	/* Change the line count to ignore empty lines */
	lineCount = i;

	if(lineCount <= 0) {
		error("No real program here. exiting.");
		return -1;
	}

	printf("Program with comments and empty lines removed:\n");
	for(size_t i = 0; i < lineCount; i += 1) {
		printf("%s\n", lines[i]);
	}

	/* Create an array to store our Labels and our output program */
	char** labels = malloc(lineCount * sizeof(char*));
	if(!labels) {
		error("Out of memory");
		return -1;
	}
	bzero(labels, lineCount * sizeof(char*));
	uint32_t* outMap = malloc(lineCount* sizeof(uint32_t));
	if(!outMap) {
		error("Out of memory");
		return -1;
	}
	bzero(outMap, lineCount * sizeof(uint32_t));

	/* Index Labels */
	for(size_t i = 0; i < lineCount; i += 1) {
		char* line = lines[i];

		char* colon = strstr(line, ":");
		if(colon) {
			/* Setup our labels and verify that they don't contain a '#' or whitespace */
			labels[i] = malloc(colon - line + 1);
			strncpy(labels[i], line, colon - line);
			(labels[i])[colon - line] = '\0';
			if(whiteSearch(labels[i])) {
				errorWithLine("Labels must not contain whitespace.");
				return -1;
			}
			if(strstr(labels[i], "#")) {
				errorWithLine("Labels must not contain '#' symbol.");
				return -1;
			}
		}
	}

	printf("Here are the labels we found, indexed to the relevent lines\n");
	for(size_t i = 0; i < lineCount; i += 1) {
		if(!labels[i]) {
			continue;
		}
		printf("Line %ld: %s(%ld)\n", lineMap[i], labels[i], i);
	}

	/* Build program */
	printf("Starting program build:\n");
	for(size_t i = 0; i < lineCount; i += 1) {
		char* line = lines[i];

		/* Ignore labels, as we've already indexed them */
		char* colon = strstr(line, ":");
		if(colon) {
			line = &colon[1];
		}

		/* Skip the whitespace to find the start of the operator */
		char* operatorStart = nonWhiteSearch(line);
		if(!operatorStart) {
			errorWithLine("Couldn't find operator");
			return -1;
		}

		/* Get the operator and after the operator */
		char* operator = NULL;
		char* operatorEnd = NULL;
		ret = nonWhiteGet(operatorStart, &operator, &operatorEnd);
		if(ret != 0) {
			errorWithLine("Failed to get the operator");
			return -1;
		}
		int opcode = 0;
		uint32_t operand = 0;
		int requiresOperand = 1;
		uint8_t imm = 0;

		/* Check if we need an operand (some instructions like STP don't require one) */
		for(size_t j = 0; j < sizeof(noOperandInstructions)/sizeof(char*); j += 1) {
			if(!noOperandInstructions[j]) {
				break;
			}
			if(!strcmp(operator, noOperandInstructions[j])) {
				requiresOperand = 0;
				break;
			}
		}
		/* If we do need an operand */
		if(requiresOperand) {

			/* Find the start of the operand */
			char* operandStart = nonWhiteSearch(operatorEnd);
			if(!operandStart) {
				free(operator);
				errorWithLine("Failed to get the start of the operand");
				return -1;
			}
			char* operandStr = NULL;
			char* operandEnd = NULL;

			/* Get the operand */
			ret = nonWhiteGet(operandStart, &operandStr, &operandEnd);
			if(ret != 0) {
				free(operator);
				errorWithLine("Failed to get the operand");
				return -1;
			}
			/* Verify that there's nothing after the operand */

			if(nonWhiteSearch(operandEnd)) {
				free(operator);
				free(operandStr);
				errorWithLine("Operand isn't end of line.");
				return -1;
			}

			/* Check if the operand is an imm */
			if(operandStr[0] == '#') {
				/* Sanity checks, then update our operand and set IMM to 1 */
				if(!supportImm) {
					free(operator);
					free(operandStr);
					errorWithLine("Immidiate operand support is disabled");
					return -1;
				}

				/* Verify instruction supports IMM */
				int found = 0;
				for(size_t j = 0; j < sizeof(noIMMInstructions) / sizeof(char*); j += 1) {
					if(!noIMMInstructions[j]) {
						break;
					}
					if(!strcmp(operator, noIMMInstructions[j])) {
						opcode = j;
						found = 1;
						break;
					}
				}
				if(found) {
					errorWithLine("Operator doesn't support immidiate addressing.");
					return -1;
				}
				imm = 1;
				printf("%s, %lu\n", operandStr, strlen(&operandStr[1]));
				char* newOperandStr = strdup(&operandStr[1]);
				printf("got here\n");
				if(!newOperandStr) {
					error("Out of memory");
					return -1;
				}
				free(operandStr);
				operandStr = newOperandStr;
			}

			/* Check if the operand is an integer and if so get it */
			if(isInteger(operandStr)) {
				operand = strtol(operandStr, (char **)NULL, 10);
				if(errno == ERANGE) {
					free(operator);
					free(operandStr);
					errorWithLine("Operand too large.");
					return -1;
				}
			}
			else {

				/* Otherwise find the label the operand is referencing and use the address of that label as the operand */
				int found = 0;
				for(size_t j = 0; j < lineCount; j += 1) {
					if(!labels[j]) {
						continue;
					}
					if(!strcmp(operandStr, labels[j])) {
						found = 1;
						operand = j;
						break;
					}
				}
				if(!found) {
					free(operator);
					free(operandStr);
					errorWithLine("Failed to find referenced label");
					return -1;
				}
			}
			free(operandStr);
		}
		/* If we don't need an operand, ensure one doesn't exist */
		else {
			if(nonWhiteSearch(operatorEnd)) {
				free(operator);
				errorWithLine("Operand exists for instruction where operands aren't supported");
				return -1;
			}
		}


		/* If the operator is VAR write the operand directly into the program */
		if(!strcmp(operator, "VAR")) {
			printf("%lu: Adding variable %u\n", i + 1, operand);
			outMap[i] = operand;
			free(operator);
			continue;
		}

		/* Else, find the opcode of the operator we're using */
		int found = 0;
		for(size_t j = 0; j < sizeof(instructionSet)/sizeof(char*); j += 1) {
			if(!instructionSet[j]) {
				break;
			}
			if(!strcmp(operator, instructionSet[j])) {
				opcode = j;
				found = 1;
				break;
			}
		}
		if(!found) {
			errorWithLine("Operator doesn't exist or is a (disabled) extended operator.");
			return -1;
		}

		if((int32_t)operand >= 4096) {
			errorWithLine("Operand too large.");
			return -1;
		}

		/* Build up the instruction and copy it into the output */
		printf("%lu: Adding instruction %s(%u) with operand %u, and imm flag: %u\n", i + 1, operator, opcode, operand, imm);
		free(operator);
		babyinsn insn = (babyinsn){.operand = operand, .imm=imm, .opcode=opcode, .pad=0};
		memcpy(&outMap[i], &insn, sizeof(babyinsn));
	}

	/* Print Program */
	for(size_t i = 0; i < lineCount; i += 1) {
		char line[34] = {0};
		for(int j = 0; j < 32; j += 1) {
			line[j] =  ((outMap[i] >> j) & 1) + '0';
		}
		if (i == lineCount - 1) {
			fprintf(outFile, "%s", line);
		} else {
			fprintf(outFile, "%s\n", line);
		}
	}
	return 0;
}
