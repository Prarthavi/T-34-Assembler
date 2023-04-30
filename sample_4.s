********************************
*        SAMPLE PROGRAM 4      *
********************************
*
         ORG  $300
BELL     EQU  $FF3A
*
ENTRY    CLV
         BVC  BELL
*
FILL1    NOP
*
STEP     BVC  EXPT
*
FILL1    NOP
*
EXPT     JSR  (BELL)
*
DONE     RTS
         CHK