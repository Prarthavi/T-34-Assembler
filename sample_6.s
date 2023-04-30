********************************
*        SAMPLE PROGRAM 6      *
********************************
*
         ORG  $300
CTR      EQU  $06
HOME     EQU  $FC58
COUT     EQU  $FDED
*
START    JSR  HOME
         LDA  #$FF
         STA  CTR
LOOP     LDA  CTR
         JSR  COUT
         DEC  CTR
         BEQ  END
         JMP  LOOP
END      RTS