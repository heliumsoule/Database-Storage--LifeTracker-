#include <stdio.h>
#include <string.h>
#include "xterm_control.h"
#include "keyboard.h"

int row, col;   //120 x 28
int i;
char Board[28][120];


void Setup() {    
  xt_par0(XT_CLEAR_SCREEN);
  xt_par2(XT_SET_ROW_COL_POS, row = 1, col = 1);
  xt_par0(XT_CH_GREEN);
  printf("----------------------------------------------------LifeTracker---------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  printf("                                  Number of Records:---|  Author:------| Version:-.--\n                       First Record  Time:------------:--:--| Last Record  Time:------------:--:--\n------------------------------------------------------------------------------------------------------------------------");
  xt_par0(XT_CH_BLUE);
  printf("------------UP/Down-Scroll between Subjects/Records/Search  Left/Right-Toggle between Subjects/Records/Search------------");
  xt_par0(XT_CH_WHITE);
}

void LifeTracker() {
  xt_par0(XT_CLEAR_SCREEN);
  xt_par2(XT_SET_ROW_COL_POS, row = 1, col = 1);
  xt_par0(XT_CH_GREEN);
  printf("----------------------------------------------------LifeTracker---------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  printf("                                  Number of Records:---| Author:------| Version:-.--\n                       First Record Time:------------:--:--| Last Record Time:------------:--:--\n------------------------------------------------------------------------------------------------------------------------");
  xt_par0(XT_CH_BLUE);
  printf("------------UP/Down-Scroll between Subjects/Records/Search Left/Right-Toggle between Subjects/Records/Search------------");
  xt_par0(XT_CH_WHITE);
  for(i = 0; i < 20; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = i+7, col = 1);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = i+7, col = 16);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = i+7, col = 19);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = i+7, col = 91);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = i+7, col = 94);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = i+7, col = 120);
    printf("|");
  }
  xt_par2(XT_SET_ROW_COL_POS, row = 7, col = 2);
  xt_par0(XT_CH_GREEN);
  printf("---Subjects---");
  xt_par0(XT_CH_WHITE);
  for(i = 0; i < 17; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 8 + i, col = 2);
    printf("--------------");
  }
  for(i = 0; i < 2; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 25+i, col = 2);
    xt_par0(XT_CH_BLUE); 
    if (i == 0)
      printf("---F5-Add-----");
    else if (i == 1)
      printf("---F6-Sort----");
  }
  for(i = 0; i < 4; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 7+5*i, col = 20);
    xt_par0(XT_CH_GREEN);
    printf("Record %d (--:--:--)", i+1);
    xt_par0(XT_CH_WHITE);
    xt_par2(XT_SET_ROW_COL_POS, row = 8+5*i, col = 20);
    printf("------------------------------");
    xt_par2(XT_SET_ROW_COL_POS, row = 9+5*i, col = 20);
    printf("----------------------------------------------------------------------");
    xt_par2(XT_SET_ROW_COL_POS, row = 10+5*i, col = 20);
    printf("----------------------------------------------------------------------");
  }
    xt_par2(XT_SET_ROW_COL_POS, row = 26, col = 20);
    xt_par0(XT_CH_BLUE);
    printf("-----------------------F2-Add F3-Delete F4-Edit------------------------");
    xt_par0(XT_CH_GREEN);
    xt_par2(XT_SET_ROW_COL_POS, row = 7, col = 95);
    printf("---------Search----------");
    xt_par2(XT_SET_ROW_COL_POS, row = 8, col = 95);
    printf("Subject:");
    xt_par2(XT_SET_ROW_COL_POS, row = 10, col = 95);
    printf("Title:");
    xt_par2(XT_SET_ROW_COL_POS, row = 12, col = 95);
    printf("Body:");
    xt_par0(XT_CH_WHITE);
    xt_par2(XT_SET_ROW_COL_POS, row = 9, col = 95);
    printf("-------------------------");
    xt_par2(XT_SET_ROW_COL_POS, row = 11, col = 95);
    printf("-------------------------");
  for(i = 0; i < 13; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 13+i, col = 95);
    printf("-------------------------");
  }
  xt_par0(XT_CH_BLUE);
  xt_par2(XT_SET_ROW_COL_POS, row = 26, col = 95);
  printf("--------F7-Search--------");
  xt_par2(XT_SET_ROW_COL_POS, row = 28, col = 1);
  xt_par0(XT_CH_BLUE);
  printf("-------------------------------------------------------F9-EXIT----------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
}

int main() {
  int c;
  int i, j, SUBJECT;

  for (i = 0; i < 28; i++) {
    for (j = 0; j < 120; j++) {
      Board[i][j] = ' ';

    }
    Board[i][120] = '\0';
  }

  Setup();
  LifeTracker();
  xt_par2(XT_SET_ROW_COL_POS,row=8,col=2);

  while (1) {
    while ((c = getkey()) == KEY_NOTHING);
    if(c == KEY_F9) break;
    else if(c == KEY_DOWN) {
      if(col == 2 && row >= 8 && row < 24) 
          xt_par2(XT_SET_ROW_COL_POS,++row,col);
      if(col == 20 && row >= 7 && row <= 20)
          xt_par2(XT_SET_ROW_COL_POS,row+=5,col);
      if(col > 94 && col < 120){
        if(row == 9)
          xt_par2(XT_SET_ROW_COL_POS,row = 11,col = 95);
        else if(row == 13)
          xt_par2(XT_SET_ROW_COL_POS,row = 9,col = 95);
        else if(row == 11)
          xt_par2(XT_SET_ROW_COL_POS,row = 13,col = 95);
      }
    }
    else if(c == KEY_UP){
      if(col == 2 && row > 8 && row <= 24)
          xt_par2(XT_SET_ROW_COL_POS,--row,col);
      if(col == 20 && row >= 8 && row < 24)
          xt_par2(XT_SET_ROW_COL_POS,row-=5,col);
      if(col > 94 && col < 120){
        if(row == 9)
          xt_par2(XT_SET_ROW_COL_POS,row = 13,col = 95);
        else if(row == 13)
          xt_par2(XT_SET_ROW_COL_POS,row = 11,col = 95);
        else if(row == 11)
          xt_par2(XT_SET_ROW_COL_POS,row = 9,col = 95);
      }
    }
    else if (c == KEY_ENTER) 
      xt_par2(XT_SET_ROW_COL_POS,++row,col=1);
    else if((c == KEY_LEFT || c == KEY_RIGHT) && col == 2) {
      SUBJECT = row;
      xt_par2(XT_SET_ROW_COL_POS,row=7,col=20);
    }
    else if((c == KEY_LEFT || c == KEY_RIGHT) && col == 20) 
        xt_par2(XT_SET_ROW_COL_POS,row=SUBJECT,col=2);
    else if(c == KEY_LEFT && col > 95 && col < 120)
      xt_par2(XT_SET_ROW_COL_POS,row,--col);
    else if(c == KEY_RIGHT && col > 94 && col < 119)
      xt_par2(XT_SET_ROW_COL_POS,row,++col);
    else if (c == KEY_BACKSPACE && col > 1) {
      xt_par2(XT_SET_ROW_COL_POS,row,--col);
      putchar(' ');
      Board[row - 1][col - 1] = ' ';
      xt_par2(XT_SET_ROW_COL_POS,row,col);
    } else if (c == KEY_DELETE) {
      Board[row - 1][col - 1] = ' ';
      putchar(' ');
      xt_par2(XT_SET_ROW_COL_POS,row,col);
    }
    else if (c == KEY_F7){
      if(col < 95)
        xt_par2(XT_SET_ROW_COL_POS,row = 9,col = 95);
      //else if(the user has written a requirement)
      //xt_par2(XT_SET_ROW_COL_POS,row = 7,col = 20);
      else
        xt_par2(XT_SET_ROW_COL_POS,row = SUBJECT,col = 2);   
    } 
    else if (c >= '1' && c <= '9') {
      putchar(c);
      Board[row-1][col-1] = c;
      if (col < 112) {
        ++col;
      } else if (row < 28) {
        xt_par2(XT_SET_ROW_COL_POS,++row,col=1);
      } else {
        xt_par2(XT_SET_ROW_COL_POS,row,col);
      }
    }
  }
  getkey_terminate();

  xt_par0(XT_CLEAR_SCREEN);
  xt_par0(XT_BG_DEFAULT);
  xt_par2(XT_SET_ROW_COL_POS,row=1,col=1);
  getkey_terminate();

  return 0;
}
    


