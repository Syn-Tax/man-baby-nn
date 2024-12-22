; NURAL NETWORK IN EXTENDED MANCHESTER BABY ASSEMBLY
; The program starts at address 1, so we need to pad out address 0 
          VAR 0       ; Declare 32-bit variable to fill space 

START: LDR ARRAY_A
    STO A

    LDR ARRAY_B
    STO B

    JMP DOT
; The next two instructions negate the stored answer
END:      STP         ; Stop processor

;; METHOD DECLARATIONS

; floating point dot product implementation 
; pointer to input arrays in registers A and B
; output is in register H
DOT: PUSHLR
    ; initialise output to 0
    LDR #0
    STO H

    LDR #0
    STO C ; start counter in register C

DOT_LOOP: LDR C ; for loop exit condition
    TST NUM_LAYERS
    JGE DOT_RET

    ; get Cth index of array A
    LDR A
    ADD C
    LDRACC
    STO D

    ; get Cth index of array B
    LDR B
    ADD C
    LDRACC

    MULF D ; multiply elements together
    ADDF H ; add to current total
    STO H ; store result in output

    ; update counter
    LDR C
    ADD #1
    STO C

    JMP DOT_LOOP

DOT_RET: RET

ERROR: LDR #1
    STO ERR
    RET


; Variable declarations follow

; fake registers to make storing temporary values easier
VAR -1
A:    VAR 0
B:    VAR 0
C:    VAR 0
D:    VAR 0
E:    VAR 0
F:    VAR 0
G:    VAR 0
H:    VAR 0
ERR:  VAR 0 ; error flag
VAR -1


; array A
NUM_LAYERS: VAR 1

ARRAY_A: VAR ARR_A
ARR_A: VAR 1082130432

ARRAY_B: VAR ARR_B
ARR_B: VAR 1082130432