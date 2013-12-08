#include <stdio.h>
#include <string.h>
#include "xterm_control.h"
#include "keyboard.h"

int row, col;
int i;

void main(){
  xt_par0(XT_CLEAR_SCREEN);
  xt_par2(XT_SET_ROW_COL_POS, row = 1, col = 1);
  xt_par0(XT_CH_GREEN);
  printf("----------------------------------------------------LifeTracker---------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  printf("                                  Number of Records:---| Author:------| Version:-.--\n                       First Record Time:------------:--:--| Last Record Time:------------:--:--\n------------------------------------------------------------------------------------------------------------------------");
  xt_par0(XT_CH_BLUE);
  printf("-------------------------------------UP/Down-Switch rows Left/Right-Switch columns--------------------------------------");
  xt_par0(XT_CH_WHITE);
  for(i = 0; i < 12; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 11+i, col = 24);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = 11+i, col = 96);
    printf("|");
  }
  xt_par2(XT_SET_ROW_COL_POS, row = 11, col = 25);
  printf("------------------------------Subject----------------------------------");
  xt_par2(XT_SET_ROW_COL_POS, row = 16, col = 25);
  printf("------------------------------");
  xt_par2(XT_SET_ROW_COL_POS, row = 19, col = 25);
  printf("----------------------------------------------------------------------");
  xt_par2(XT_SET_ROW_COL_POS, row = 20, col = 25);
  printf("----------------------------------------------------------------------");
  xt_par0(XT_CH_GREEN);
  xt_par2(XT_SET_ROW_COL_POS, row = 12, col = 25);
  printf("Record - (--:--:--)");
  xt_par2(XT_SET_ROW_COL_POS, row = 15, col = 25);
  printf("Title:");
  xt_par2(XT_SET_ROW_COL_POS, row = 18, col = 25);
  printf("Body:");
  xt_par0(XT_CH_BLUE);
  xt_par2(XT_SET_ROW_COL_POS, row = 22, col = 25);
  printf("------------------F2-Edit Title F3-Edit Body F7-Save-------------------");
  xt_par2(XT_SET_ROW_COL_POS, row = 28, col = 1);
  xt_par0(XT_CH_BLUE);
  printf("-------------------------------------------------------F9-EXIT----------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
}