********************************
*        SAMPLE PROGRAM 8      *
********************************
*
         ORG  $300
*
PTR      EQU  $06
*
ENTRY    LDA  #$04
         STA  PTR+1
         LDY  #$00
         STY  PTR
* SETS PTR (6,7) TO $400
START    LDA  #$A0
LOOP     STA  (PTR),Y
         INY
         BNE  LOOP
NXT      INC  PTR+1
         LDA  PTR+1
         CMP  #$08
         BCC  START
EXIT     RTS