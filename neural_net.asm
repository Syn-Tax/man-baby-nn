; NURAL NETWORK IN EXTENDED MANCHESTER BABY ASSEMBLY
; The program starts at address 1, so we need to pad out address 0 
          VAR 0       ; Declare 32-bit variable to fill space 

START: LDR ARRAY_A
    STO A

    LDR ARRAY_B
    STO B

    JMP MSE_LOSS
END: STP         ; Stop processor

;; METHOD DECLARATIONS

; backpropagation!!! (god this is stupid)
BACKWARD: PUSHLR ; pointer to predictions in A, pointer to targets in B, floating point number of predictions in C
    ; create counter to loop through layers backwards
    LDR NUM_LAYERS
    STO H

    ; move floating point number of preds to D
    LDR C
    STO D

    ; calculate gradients for final layer
    LDR BIAS_GRADS_PTR
    ADD H

    STO C ; store in C to calculate derivative of MSE

    LDR H
    PUSHACC

    JMP MSE_PRIME ; calculate error grads

    ; calculate weight gradients
    ; 

; method to calculate the derivatives of MSE loss, pointer to predictions in A, pointer to targets in B, pointer to results in C, floating point number of predictions in D
MSE_PRIME: PUSHLR ; -2 * (target - pred) / num_preds
    ; get integer number of outputs
    LDR LAYER_DIMS
    ADD NUM_LAYERS
    SUB #1
    LDRACC
    STO E ; store this in register E

    ; start counter
    LDR #0
    STO H

MSE_PRIME_LOOP: LDR H ; for loop exit condition
    TST E
    JGE MSE_PRIME_RET

    ; get prediction
    LDR A
    ADD H
    LDRACC
    STO F

    ; get target
    LDR B
    ADD H
    LDRACC

    SUBF F ; target - pred
    STO F ; store back into F

    MULF MINUS_TWO_FLOAT ; accumulator now holds -2 * (target - pred)
    DIVF D ; accumulator now holds -2 * (target - pred) / num_preds

    STO I ; store in output register temporarily

    ; find address to store at
    LDR C
    ADD H

    STOACC I ; store output at correct address

    ; increment counter
    LDR H
    ADD #1
    JMP MSE_PRIME_LOOP

MSE_PRIME_RET: RET

; method to calculate MSE loss, pointer to predictions in A, pointer to targets in B
MSE_LOSS: PUSHLR
    ; get number of outputs
    LDR LAYER_DIMS
    ADD NUM_LAYERS
    SUB #1
    LDRACC
    STO C ; store this in register C

    ; start sum at zero in output register
    LDR #0
    STO I

    ; start floating point counter to allow for mean later
    LDR #0
    STO D

    ; looy through each output and sum square errors
    LDR #0
    STO H

MSE_LOOP: LDR H ; for loop exit condition
    TST C
    JGE MSE_RET

    ; get prediction
    LDR A
    ADD H
    LDRACC ; load prediction into accumulator
    STO E

    ; get target
    LDR B
    ADD H
    LDRACC ; load target into accumulator

    SUBF E ; target - pred

    STO E
    MULF E ; square error

    ; add to output
    ADDF I
    STO I

    ; increment floating point counter
    LDR D
    ADDF ONE_FLOAT
    STO D

    ; increment counter
    LDR H
    ADD #1
    STO H

    JMP MSE_LOOP

MSE_RET: LDR I ; finish mean calculation
    DIVF D
    STO I

    RET

; method to conduct forward pass through the network
FORWARD: PUSHLR

    ; copy inputs into first output layer
    LDR #0
    STO H

    ; get input layer dim into F
    LDR LAYER_DIMS
    LDRACC
    STO F

INPUT_LOOP: LDR H ; for loop exit condition
    TST F
    JGE INPUT_RET

    ; get input value from register A
    LDR A
    ADD H
    LDRACC

    STO I ; store in output register temporarily

    ; get output address
    LDR OUTPUTS_PTR
    LDRACC
    ADD H

    STOACC I ; store I into output address

    ; increment counter
    LDR H
    ADD #1
    STO H

    JMP INPUT_LOOP

    ; initialise counter from 1 (skipping input layer)
INPUT_RET: LDR #1
    STO H

    ; loop through each layer
LAYER_LOOP: LDR H
    TST NUM_LAYERS
    JGE FORWARD_RET

    ; store size of current layer in F
    LDR LAYER_DIMS
    ADD H
    STO F

    ; start second counter in G
    LDR #0
    STO G

    ; loop through each neuron
NEURON_LOOP:  LDR G
    TST F
    JGE NEURON_RET

    ; dot weights and previous layer's output
    ; get current weights pointer
    LDR WEIGHTS_PTR
    ADD H
    SUB #1
    LDRACC

    ADD G
    STO A

    ; get current outputs pointer
    LDR OUTPUTS_PTR
    ADD H
    SUB #1
    STO B

    ; push current counters onto stack
    LDR F
    PUSHACC

    LDR G
    PUSHACC

    LDR H
    PUSHACC

    ; calculate dot product
    JMP DOT

    ; pop counters from stack
    POPACC
    STO H

    POPACC
    STO G

    POPACC
    STO F

    ; add bias
    LDR BIASES_PTR
    ADD H
    SUB #1
    LDRACC
    ADD G
    LDRACC ; load current bias into accumulator

    ADDF I ; add to weighted sum
    STO I ; store back in output register

    ; store output
    LDR OUTPUTS_PTR
    ADD H
    LDRACC
    ADD G
    STOACC I

    ; increment G
    LDR G
    ADD #1
    JMP NEURON_LOOP

    ; increment H and restart layer loop
NEURON_RET: LDR H
    ADD #1
    JMP LAYER_LOOP

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

    ; allocate bias grads 1st dimension
    LDR NUM_LAYERS
    STO A
    JMP MALLOC
    LDR I
    STO BIAS_GRADS_PTR

    ; allocate weights 1st dimension
    LDR NUM_LAYERS
    STO A
    JMP MALLOC
    LDR I
    STO WEIGHTS_PTR

    ; allocate weight grads 1st dimension
    LDR NUM_LAYERS
    STO A
    JMP MALLOC
    LDR I
    STO WEIGHT_GRADS_PTR

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

    ; allocate bias grads for each layer
    LDR LAYER_DIMS
    ADD #1
    ADD H
    LDRACC
    STO A

    JMP MALLOC

    LDR BIAS_GRADS_PTR
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

    STOACC I

    ; allocate weight grads 2nd dimension
    LDR LAYER_DIMS
    ADD #1
    ADD H
    LDRACC
    STO A

    JMP MALLOC

    LDR WEIGHT_GRADS_PTR
    ADD H

    STOACC I

    ; initialise next counter
    LDR #0
    STO G

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

    ; allocate weight grads
    JMP MALLOC

    LDR WEIGHT_GRADS_PTR
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

; floating point transpose matrix, vector product
; pointer to input matrix in register A, pointer to input vector in register B, pointer to output vector in register C

; floating point dot product implementation 
; pointer to input arrays in registers A and B
; output is in register I
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
VAR -1

SEED: VAR 0
RVAL: VAR 668265261

; network variables
NUM_LAYERS: VAR 2
LAYER_DIMS: VAR LAYER_DIMS_A
LAYER_DIMS_A: VAR 2
              VAR 1

; test arrays
ARRAY_A: VAR ARR_A
ARR_A: VAR 1056964608

ARRAY_B: VAR ARR_B
ARR_B: VAR 0


; data
INPUTS_PTR: VAR INPUTS
INPUTS: VAR 1065353216
    VAR 0

ONE_FLOAT: VAR 1065353216
MINUS_TWO_FLOAT: VAR 3221225472

; managed variables
WEIGHTS_PTR: VAR 0
BIASES_PTR: VAR 0
OUTPUTS_PTR: VAR 0
WEIGHT_GRADS_PTR: VAR 0
BIAS_GRADS_PTR: VAR 0

; MALLOC variables
MEMBLOCK: VAR 0
FREEMEM_PTR: VAR MEMORY_START
MEMORY_START: VAR 0
