********************************
*        SAMPLE PROGRAM 10     *
********************************
*
         ORG  $300
*
NUM      EQU  $06
MEM      EQU  $07
RSLT     EQU  $08
STAT     EQU  $09
*
YSAV1    EQU  $35
COUT1    EQU  $FDF0
CVID     EQU  $FDF9
COUT     EQU  $FDED
PRBYTE   EQU  $FDDA
*
*
OPERATOR LDA  #$00
         PHA
         PLP
         LDA  NUM
         AND  MEM
         STA  RSLT
         PHP
         PLA
         STA  STAT
         RTS
*
PRHEX    LDA  #$A4
         JSR  COUT
         LDA  NUM
         JMP  PRBYTE
*
PRBIT    LDA  NUM
         LDX  #$08
TEST     ASL
         BCC  PZ
P0       PHA
         LDA  #$B1
         JSR  COUT
         LDA  #$A0
         JSR  COUT
         BCS  NXT
*
PZ       PHA
         LDA  #$B0
         JSR  COUT
         LDA  #$A0
         JSR  COUT
*
NXT      PLA
         DEX
         BNE  TEST
*
EXIT     RTS
*
         NOP
         NOP
         NOP
*
CSHOW    CMP  #$80
         BCC  CONT
         CMP  #$8D
         BEQ  CONT
         CMP  #$A0
         BCS  CONT
*
         PHA
         STY  YSAV1
         AND  #$7F
         JMP  CVID
*
CONT     JMP  COUT1
*
EOF      BRK
*
         CHK