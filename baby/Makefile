.PHONY: all clean simulator assembler

all: assembler simulator

DEBUG := 0

simulator:
	@gcc -o simulator/simulator simulator/simulator.c simulator/common.c simulator/instructions.c simulator/stack.c -Isimulator -O3 -Wall -Wextra -pedantic -std=c99 -DDEBUG=$(DEBUG)

assembler:
	@gcc -o assembler/assembler assembler/assembler.c -Iassembler -O3 -Wall -Wextra -pedantic -std=c99

clean:
	@rm -rf simulator/simulator
	@rm -rf assembler/assembler
