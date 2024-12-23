; NURAL NETWORK IN EXTENDED MANCHESTER BABY ASSEMBLY
; The program starts at address 1, so we need to pad out address 0 
          VAR 0       ; Declare 32-bit variable to fill space 

START: LDR ARRAY_A
    STO A

    LDR ARRAY_B
    STO B

    JMP DOT
END:      STP         ; Stop processor

;; METHOD DECLARATIONS

; method to conduct forward pass through the network
FORWARD: PUSHLR
    ; initialise counter from 1 (skipping input layer)
    LDR #1
    STO H

    ; loop through each layer
LAYER_LOOP: LDR H
    TST NUM_LAYERS
    JGE

    ; loop through each neuron

FORWARD_RET: RET

; Method to initialise and allocate memory for weights and biases
INIT: PUSHLR
    LDR #0
    STO H ; initialise counter in register D

    ; allocate biases 1st dimension
    LDR NUM_LAYERS
    STO A
    JMP MALLOC
    LDR I
    STO BIASES_PTR

    ; allocate weights 1st dimension
    LDR NUM_LAYERS
    STO A
    JMP MALLOC
    LDR I
    STO WEIGHTS_PTR

    ; allocate outputs 1st dimension
    LDR NUM_LAYERS
    STO A
    JMP MALLOC
    LDR I
    STO OUTPUTS_PTR

    ; allocate input layer 'outputs' (to make forward pass easier)
    LDR LAYER_DIMS
    LDRACC
    STO A
    JMP MALLOC

    LDR OUTPUTS_PTR
    STOACC I

INIT_LOOP: LDR H ; for loop exit condition
    ADD #1
    TST NUM_LAYERS
    JGE INIT_RET

    ; allocate biases for each layer
    LDR LAYER_DIMS
    ADD #1
    ADD H
    LDRACC
    STO A

    JMP MALLOC

    LDR BIASES_PTR
    ADD H
    
    STOACC I

    ; allocate output space for each layer
    LDR LAYER_DIMS
    ADD #1
    ADD H
    LDRACC
    STO A

    JMP MALLOC

    LDR OUTPUTS_PTR
    ADD #1
    ADD H
    STOACC I

    ; allocate weights 2nd dimension
    LDR LAYER_DIMS
    ADD #1
    ADD H
    LDRACC
    STO A

    JMP MALLOC

    LDR WEIGHTS_PTR
    ADD H

    LDR #0
    STO G

    STOACC I

INIT_LOOP_LOOP: LDR LAYER_DIMS ; for loop exit condition
    ADD H
    LDRACC

    TST G
    JGE INIT_LOOP_RET

    ; get amount of memory required
    LDR WEIGHTS_PTR
    ADD H

    LDRACC

    ; allocate memory
    STO A
    JMP MALLOC

    ; save pointer into weights array at relevant location
    LDR WEIGHTS_PTR
    ADD H
    LDRACC
    ADD G

    STOACC I

    ; increment G
    LDR G
    ADD #1
    STO G


INIT_LOOP_RET: LDR H ; increment main loop counter and jump to start
    ADD #1
    STO H
    JMP INIT_LOOP 

INIT_RET: RET

; pseudo-random number generator
; TODO: fix this for floating point (https://allendowney.com/research/rand/downey07randfloat.pdf could be interesting)
RANDOM: PUSHLR
    LDR SEED
    XOR #61
    STO I
    LDR SEED
    RSR #16
    XOR I

    STO I
    LSL #3
    ADD I
    STO I

    RSR #4
    XOR I
    STO I

    MUL RVAL
    STO I

    RSR #15
    XOR I
    STO I

    ; increment seed
    LDR SEED


    RET

; floating point dot product implementation 
; pointer to input arrays in registers A and B
; output is in register H
DOT: PUSHLR
    ; initialise output to 0
    LDR #0
    STO I

    LDR #0
    STO E ; start counter in register C

DOT_LOOP: LDR E ; for loop exit condition
    TST NUM_LAYERS
    JGE DOT_RET

    ; get Cth index of array A
    LDR A
    ADD E
    LDRACC
    STO F

    ; get Cth index of array B
    LDR B
    ADD E
    LDRACC

    MULF F ; multiply elements together
    ADDF I ; add to current total
    STO I ; store result in output

    ; update counter
    LDR E
    ADD #1
    STO E

    JMP DOT_LOOP

DOT_RET: RET

ERROR: LDR #1
    STO ERR
    RET

MALLOC: PUSHLR							; Push the link register onto the stack
    LDR A
	STO MEMBLOCK 						; Store the accumulator into MEMBLOCK
	LDR FREEMEM_PTR 					; Load FREEMEM_PTR into the accumulator
    STO I
	PUSHACC 							; Push the accumulator onto the stack
	ADD MEMBLOCK 						; Add MEMBLOCK to the accumulator
	STO FREEMEM_PTR 					; Store the accumulator into FREEMEM_PTR
	POPACC 								; Pop the top of the stack into the accumulator
	RET 								; Pop the top of the stack into the program counter

; Variable declarations follow

; fake registers to make storing temporary values easier
VAR -1
; method arguments
A:    VAR 0
B:    VAR 0
C:    VAR 0
D:    VAR 0
; temp variables
E:    VAR 0
F:    VAR 0
G:    VAR 0
H:    VAR 0
; method output
I:    VAR 0
ERR:  VAR 0 ; error flag
VAR -1

SEED: VAR 0
RVAL: VAR 668265261

; network variables
NUM_LAYERS: VAR 1
LAYER_DIMS: VAR LAYER_DIMS_A
LAYER_DIMS_A: VAR 1
              VAR 1

; managed variables
WEIGHTS_PTR: VAR 0
BIASES_PTR: VAR 0
OUTPUTS_PTR: VAR 0

ARRAY_A: VAR ARR_A
ARR_A: VAR 1082130432

; MALLOC variables
MEMBLOCK: VAR 0
FREEMEM_PTR: VAR MEMORY_START
MEMORY_START: VAR 0