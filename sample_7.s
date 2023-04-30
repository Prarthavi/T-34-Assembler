********************************
*        SAMPLE PROGRAM 7      *
********************************
*
         ORG  $300
*
COUT     EQU  $FDED
*
START    LDX  #$00
LOOP     LDA  $0313,X
         JSR  COUT
         INX
         CPX  #$05
         BCC  LOOP
         LDA  #$8D
         JSR  COUT
EXIT     RTS