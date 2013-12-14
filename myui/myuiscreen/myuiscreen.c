#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xterm_control.h"
#include "keyboard.h"
#include <unistd.h> 

int row, col;   //120 x 28                                How will we save what the user as written into a record/subject?
int i;                                                  //How will implement the "subject" section? (possibly an array of records)
int SUBJECT = 8;   
char input[1000];

int ReadMystoreFromChild(char *argv1, char *argv2) {
  int pid, mypipe[2];
  char *newargv[5];
  char *errmsg;
  int n_input = 0;
  
  // turn off special keyboard handling
  getkey_terminate();
  
  // create the pipe
  if (pipe(mypipe) == -1) {
    strcpy(errmsg,"Problem in creating the pipe");
    return 0;
  }
  
  pid = fork();
  
  if (pid == -1) {
    strcpy(errmsg, "Error in forking");
    return 0;
  }
  if (pid == 0) {  // child
    close(mypipe[0]);  // Don't need to read from the pipe
    dup2(mypipe[1],STDOUT_FILENO);  // connect the "write-end" of the pipe to child's STDOUT
    
    newargv[0] = newargv[1] = "./mystore";
    newargv[2] = argv1;
    newargv[3] = argv2;
    newargv[4] = NULL;
    execvp(newargv[0],newargv+1);
    exit(0);
  }
  else if (pid > 0) {
    char *s = input;
    int c;

    close(mypipe[1]);  // Don't need to write to the pipe
    
    // read the data into the input array from mypipe[0]
    FILE *fpin;
    if ((fpin = fdopen(mypipe[0],"r")) == NULL) {
      printf("ERROR: Cannot read from mypipe[0]\n");
      exit(1);
    }
    for (n_input = 0; n_input < sizeof(input)-1; ++n_input) {
      if ((c = getc(fpin)) == EOF) break;
      *s++ = c;
    }
    input[n_input] = '\0';
    fclose(fpin);
    
    wait(NULL);  // wait for child to finish
    close(mypipe[0]);
  }
  
  return n_input;
}



void Setup() {                          //print using myui (stat)
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
    printf("--------------");  //prints the list of subjects, utilizes the forking program/myui1  cmd(mystore, subject, i+1)
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
    printf("Record %d (--:--:--)", i+1);                  //get the time using myui1
    xt_par0(XT_CH_WHITE);
    xt_par2(XT_SET_ROW_COL_POS, row = 8+5*i, col = 20);
    printf("------------------------------");            //get the title using myui1
    xt_par2(XT_SET_ROW_COL_POS, row = 9+5*i, col = 20);
    printf("----------------------------------------------------------------------");   //print first 56 characters of the body using myui1
    xt_par2(XT_SET_ROW_COL_POS, row = 10+5*i, col = 20);
    printf("----------------------------------------------------------------------");   //print last 56 characters of the body using myui1
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
  xt_par2(XT_SET_ROW_COL_POS,row=SUBJECT,col=2);
}

void addscreen(){
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
  printf("Record - (--:--:--)");                       //print the record number and time using myui1 
  xt_par2(XT_SET_ROW_COL_POS, row = 15, col = 25);
  printf("Title:");
  xt_par2(XT_SET_ROW_COL_POS, row = 18, col = 25);
  printf("Body:");
  xt_par0(XT_CH_BLUE);
  xt_par2(XT_SET_ROW_COL_POS, row = 22, col = 25);
  printf("---------------------------F2-Save and Exit----------------------------");
  xt_par2(XT_SET_ROW_COL_POS, row = 28, col = 1);
  xt_par0(XT_CH_BLUE);
  printf("-------------------------------------------------------F9-EXIT----------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  xt_par2(XT_SET_ROW_COL_POS,row = 16, col = 25);
}


int main() {
  int c;
  int i, j;
  int screen = 0;

  for (i = 0; i < 28; i++) {
    for (j = 0; j < 120; j++) {
      Board[i][j] = ' ';

    }
    Board[i][120] = '\0';
  }
    Setup();
  while (1){
    if (screen == 0) 
      LifeTracker();
    while (screen == 0) {
      while ((c = getkey()) == KEY_NOTHING);
      if(c == KEY_F9) screen = 3;
      else if(c == KEY_DOWN) {
	if(col == 2 && row >= 8 && row < 24) 
          xt_par2(XT_SET_ROW_COL_POS,SUBJECT = ++row,col);
	if(col == 20 && row >= 7 && row <= 20)
          xt_par2(XT_SET_ROW_COL_POS,row+=5,col);
	if(col > 94 && col < 120){
	  if(row == 9)
	    xt_par2(XT_SET_ROW_COL_POS,row = 11,col = 95);
	  else if(row == 11)
	    xt_par2(XT_SET_ROW_COL_POS,row = 13,col = 95);
	  else if(row < 25)
	    xt_par2(XT_SET_ROW_COL_POS,++row,col);
	}
      }
      else if(c == KEY_UP){
	if(col == 2 && row > 8 && row <= 24)
          xt_par2(XT_SET_ROW_COL_POS,SUBJECT = --row,col);
	if(col == 20 && row >= 8 && row < 24)
          xt_par2(XT_SET_ROW_COL_POS,row-=5,col);
	if(col > 94 && col < 120){
	  if(row == 9)
	    xt_par2(XT_SET_ROW_COL_POS,row = 13,col = 95);
	  else if(row == 13)
	    xt_par2(XT_SET_ROW_COL_POS,row = 11,col = 95);
	  else if(row == 11)
	    xt_par2(XT_SET_ROW_COL_POS,row = 9,col = 95);
	  else 
	    xt_par2(XT_SET_ROW_COL_POS,--row,col);
	}
      }
      else if (c == KEY_ENTER && col > 94 && col < 120 && row > 12 && row < 25) 
	xt_par2(XT_SET_ROW_COL_POS,++row,col=95);

      else if((c == KEY_LEFT || c == KEY_RIGHT) && col == 2) {
	SUBJECT = row;
	xt_par2(XT_SET_ROW_COL_POS,row=7,col=20);
      }
      else if((c == KEY_LEFT || c == KEY_RIGHT) && col == 20) 
        xt_par2(XT_SET_ROW_COL_POS,row=SUBJECT,col=2);

      else if(c == KEY_LEFT && col > 95 && col < 120)
	xt_par2(XT_SET_ROW_COL_POS,row,--col);

      else if(c == KEY_LEFT && col == 95 && row > 13 && row < 26)
	xt_par2(XT_SET_ROW_COL_POS,--row,col=119);

      else if(c == KEY_RIGHT && col > 94 && col < 119)
	xt_par2(XT_SET_ROW_COL_POS,row,++col);

      else if(c == KEY_RIGHT && col == 119 && row > 11 && row < 25) 
	xt_par2(XT_SET_ROW_COL_POS,++row,col=95);

      else if (c == KEY_BACKSPACE && col > 95 && col < 120 && ((row > 12 && row < 26) || row == 11 || row == 9)) {
	xt_par2(XT_SET_ROW_COL_POS,row,--col);
	putchar(' ');
	Board[row - 1][col - 1] = ' ';
	xt_par2(XT_SET_ROW_COL_POS,row,col);
      } 
      else if (c == KEY_BACKSPACE && col == 95 && row > 13 && row < 26){
	xt_par2(XT_SET_ROW_COL_POS,--row,col=119);
	putchar(' ');
	Board[row - 1][col - 1] = ' ';
	xt_par2(XT_SET_ROW_COL_POS,row,col);
      }
      else if (c == KEY_DELETE) {
	Board[row - 1][col - 1] = ' ';
	putchar(' ');
	xt_par2(XT_SET_ROW_COL_POS,row,col);
      }
      else if ((c >= ' ' && c <= '~') && col >= 95 && col < 120){
       	putchar(c); 
	Board[row-2][col-1] = c; 
	if (col < 119){
	  ++col; 
	}
	else{ 
	  if (row > 12 && row < 25)                                            
	    xt_par2(XT_SET_ROW_COL_POS,++row,col=95);
	  else xt_par2(XT_SET_ROW_COL_POS,row,col); 
	}
      }
      else if (c == KEY_F2)
	screen = 1;
      
      else if (c == KEY_F4 && col == 20)
	screen = 2;

      else if (c == KEY_F7){
	if(col < 95)
	  xt_par2(XT_SET_ROW_COL_POS,row = 9,col = 95);
	//else if(the user has written a requirement)
	//xt_par2(XT_SET_ROW_COL_POS,row = 7,col = 20);
	else
	  xt_par2(XT_SET_ROW_COL_POS,row = SUBJECT,col = 2);   
      } 
      /*else if (c >= '1' && c <= '9') {
	putchar(c);
	Board[row-1][col-1] = c;
	if (col < 112) {
	  ++col;
	} else if (row < 28) {
	  xt_par2(XT_SET_ROW_COL_POS,++row,col=1);
	} else {
	  xt_par2(XT_SET_ROW_COL_POS,row,col);
	}
	}*/
    }
    if (screen == 1 || screen == 2){
      addscreen();
      if (screen == 2){
	//print corresponding record in correct locations using myui1 
      }
    } 
    while(screen == 1 || screen == 2){
      while((c = getkey()) == KEY_NOTHING);
      if (c == KEY_F9){
	screen = 0;
      }
      else if (c == KEY_DOWN){
	if(row == 16)
	  xt_par2(XT_SET_ROW_COL_POS,row = 19,col = 25);
	else if(row < 20)
	  xt_par2(XT_SET_ROW_COL_POS,++row,col);
      }
      else if (c == KEY_UP){	
	if(row == 19)
	  xt_par2(XT_SET_ROW_COL_POS,row = 16,col = 25);
	else if(row == 20)
	  xt_par2(XT_SET_ROW_COL_POS,--row,col);
      }
      else if(c == KEY_LEFT && col > 25 && col < 95)
	xt_par2(XT_SET_ROW_COL_POS,row,--col);

      else if(c == KEY_LEFT && col == 25 && row == 20)
	xt_par2(XT_SET_ROW_COL_POS,--row,col=94);

      else if(c == KEY_RIGHT && col > 24 && col < 94 && row > 18)
	xt_par2(XT_SET_ROW_COL_POS,row,++col);

      else if(c == KEY_RIGHT && col > 24 && col < 54 && row == 16)
	xt_par2(XT_SET_ROW_COL_POS,row,++col);

      else if(c == KEY_RIGHT && col == 94 && row == 19) 
	xt_par2(XT_SET_ROW_COL_POS,++row,col=25);
      
      else if (c == KEY_ENTER && row > 18 && row < 20) 
	xt_par2(XT_SET_ROW_COL_POS,++row,col=25);

      else if (c == KEY_BACKSPACE && col > 25 && col < 95 && ((row > 18 && row < 21) || row == 16)) {
	xt_par2(XT_SET_ROW_COL_POS,row,--col);
	putchar(' ');
	Board[row - 1][col - 1] = ' ';
	xt_par2(XT_SET_ROW_COL_POS,row,col);
      } 
      else if (c == KEY_BACKSPACE && col == 25 && row == 20){
	xt_par2(XT_SET_ROW_COL_POS,--row,col=94);
	putchar(' ');
	Board[row - 1][col - 1] = ' ';
	xt_par2(XT_SET_ROW_COL_POS,row,col);
      }
      else if (c == KEY_DELETE) {
	Board[row - 1][col - 1] = ' ';
	putchar(' ');
	xt_par2(XT_SET_ROW_COL_POS,row,col);
      }
      else if ((c >= ' ' && c <= '~') && col >= 25 && col < 95){
	if (!(row == 16 && col > 54))
	  putchar(c); 
	Board[row-2][col-1] = c; 
	if (col < 94 && !(row == 16 && col > 54)){
	  ++col; 
	}
	else{ 
	  if (row == 19)                                            
	    xt_par2(XT_SET_ROW_COL_POS,++row,col=25);
	  else xt_par2(XT_SET_ROW_COL_POS,row,col); 
	}
      }
      else if(c == KEY_F2 && screen == 1 /* && there is a valid title and description */){
	//save record to corresponding subject
	screen = 0;
      }
      else if(c == KEY_F2 && screen == 2 /* && there has been a valid change in the record */){
	//update record
	screen = 0;
      }	
    }
    if (screen == 3) break;
  }
  getkey_terminate();

  xt_par0(XT_CLEAR_SCREEN);
  xt_par0(XT_BG_DEFAULT);
  xt_par2(XT_SET_ROW_COL_POS,row=1,col=1);
  getkey_terminate();

  return 0;
}

    


