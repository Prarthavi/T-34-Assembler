********************************
*        SAMPLE PROGRAM 3      *
********************************
*
         ORG  $300
BELL     EQU  $FF3A
*
ENTRY    CLC
         BCC  EXPT
*
FILL     NOP
*
EXPT     JSR  BELL
*
DONE     RTS
         CHK