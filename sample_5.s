********************************
*        SAMPLE PROGRAM 5      *
********************************
*
         ORG  $300
BELL     EQU  $FF3A
*
ENTRY    CLV
         BVC  STEP
*
FILL1    NOP
*
STEP     BVC  EXPT
*
FILL2    NOP
*
EXPT     JSR  BELL
*
DONE     RTS
         CHK