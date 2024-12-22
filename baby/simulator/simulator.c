#include <common.h>
#include <instructions.h>
#include <stack.h>
#include <getopt.h>
#include <errno.h>

uint32_t registers[4] = {0, 1, 1, 0}; /* Accumulator is 0, PC is 1, LR (extension) is 2, PI is 3 */

uint32_t* memory = NULL;
unsigned long memoryLen = 0;

/* The arguments we take as input to the program and whether they require an argument */
static struct option arguments[] = {
	{"load", required_argument, NULL, 'l'},
	{"memory_len", required_argument, NULL, 'm'},
	{"extend_instruction_set", no_argument, NULL, 'e'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0}
};


static void printArgs(char** argv) {
	printf("Usage: %s [OPTIONS]\n", argv[0]);
	printf("\t-h, --help, view this help dialog.\n");
	printf("\t-l, --load <file>, specify the file you wish to load\n");
	printf("\t-m, --memory_len <LEN>, Specify how much memory you wish to have available (default: 32, max: %lu).\n", ULONG_MAX);
	printf("\t-e, --extend_instruction_set, Enable our extended instruction set.\n");
	printf("\t-i, --enable_imm_addressing, Enable immidiate addressing.\n");

}

int main(int argc, char** argv) {

	/* Set arguments to default */
	memoryLen = 32;

	/* Deal with arguments */
	int opt = 0;
	char* fileToLoad = NULL;
	int isExtended = 0;
	int enableImm = 0;
	while((opt = getopt_long(argc, argv, "iehl:m:", arguments, NULL)) > 0) {
		switch(opt) {
		case 'l':

			/* Verify we have an argument */
			if(!optarg) {
				printArgs(argv);
				return -1;
			}
			fileToLoad = optarg;
			break;
		case 'm':

			/* Verify we have an argument */
			if(!optarg) {
				printArgs(argv);
				return -1;
			}
			char* strEnd = NULL;
			/* Convert the arg to an integer and verify that it's within range and properly formed */
			memoryLen = strtoul(optarg, &strEnd, 10);
			if(strEnd != &optarg[strlen(optarg)] || errno == ERANGE || memoryLen == 0) {
				printArgs(argv);
				return -1;
			}
			break;
		case 'e':
			isExtended = 1;
			break;
		case 'i':
			enableImm = 1;
			break;
		case 'h':
		default:
			printArgs(argv);
			return 0;
		}
	}
	if(!fileToLoad) {
		printArgs(argv);
		return -1;
	}

	/* If we're not using the extended instruction set, wipe out the extra instructions */
	if(!isExtended) {
		bzero(&instructionSet[NUM_BASIC_INSTRUCTIONS], sizeof(operator*) * (NUM_INSTRUCTIONS - NUM_BASIC_INSTRUCTIONS));
	}

	/* Setup the memory */
	memory = malloc(memoryLen * sizeof(uint32_t));
	if(!memory) {
		error("Failed to allocate memory for the program memory");
		return -1;
	}
	
	/* Zero out the memory */
	bzero(memory, memoryLen);

	/* Load in the file requested */
	int ret = loadFile(fileToLoad);
	if(ret != 0) {
		free(memory);
		error("Failed to load file");
		return -1;
	}

	while(1) {

		/* Verify that the Program counter is within the memory */
		if(registers[1] > memoryLen || registers[2] > memoryLen) {
			free(memory);
			emptyStack();
			error("Program out of bounds");
			return -1;
		}
		/* Print our memory */
		printMemory();

		registers[3] = memory[registers[1]];

		/* Check that the padding for the instruction is 0 */
		babyinsn* insn = (babyinsn*)&registers[3];
		if((insn->imm != 0 && !enableImm) || insn->pad != 0) {
			free(memory);
			emptyStack();
			error("Bad instruction with non-zero padding or immidiate addressing disabled");
			return -1;
		}

		/* Check that we're not trying to use an extension instruction without enabling the extended instruction set */
		operator instruction = instructionSet[insn->opcode];
		if(!instruction) {
			free(memory);
			emptyStack();
 			error("Instruction doesn't exist within chosen instruction set");
 			return -1;
		}

		/* Demonstrate our decoding prowess */
		printDecode();

		/* Run the instruction from the function table with the operand */
 		ret = instruction(insn->operand, insn->imm);

 		/* 1 means the STP instruction has been called, we exit. */
 		if(ret == 1) {
			break;
 		}
 		if(ret != 0) {
 			free(memory);
 			emptyStack();
 			error("Error when executing instruction");
 			return -1;
 		}

 		/* Move to the next instruction */
		registers[1] += 1;
	}
	free(memory);
	emptyStack();
	printf("STOP, program completed.\n");
	return 0;
}
