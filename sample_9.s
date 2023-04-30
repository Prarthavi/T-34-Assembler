********************************
*        SAMPLE PROGRAM 9      *
********************************
*
         ORG  $300
*
N1       EQU  $06
N2       EQU  $08
RSLT     EQU  $0A
*
START    CLC
         LDA  N1
         ADC  N2
         STA  RSLT
         LDA  N1+1
         ADC  N2+1
         STA  RSLT+1
END      RTS