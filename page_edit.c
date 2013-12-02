#include <stdio.h>

#include "keyboard.h"
#include "xterm_control.h"

#define CONTROL_C 3

char Board[23][80] = {' '};
int row, col;

void GreetUser() {
    char *greeting = "Arrow keys work, and F10 terminates...";
    xt_par2(XT_SET_ROW_COL_POS,row=1,col=1);
    xt_par0(XT_CLEAR_SCREEN);
    printf("%s",greeting);
}

void RunEditor() {
    int c;
    while (1) {
        while ((c = getkey()) == KEY_NOTHING) ;
        if(c == KEY_F10) break;
        else if (c == KEY_UP && row > 1) 
            xt_par2(XT_SET_ROW_COL_POS,--row,col);
        else if (c == KEY_DOWN && row < 24)
            xt_par2(XT_SET_ROW_COL_POS,++row,col);
        else if (c == KEY_RIGHT && col < 80)
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
        else if (c == KEY_LEFT && col > 1)
            xt_par2(XT_SET_ROW_COL_POS,row,--col);
        else if (c == KEY_ENTER) 
            xt_par2(XT_SET_ROW_COL_POS,++row,col=1);
        else if (c == KEY_DELETE) {
            putchar(' ');
            Board[row-2][col-1] = ' ';
        }
        else if (c >= ' ' && c <= '~') {
            putchar(c);
            Board[row-2][col-1] = c;
            printf("Location is %d %d", row, col);  
            if (col < 80)
                ++col;
            else
                xt_par2(XT_SET_ROW_COL_POS,++row,col=1);
        }
    }
}

void main(int argc, char * argv[]) {

    GreetUser();

    int i = 0, j = 0, Length = 1, one = 1;
    FILE *FP;
    FP = fopen(argv[1], "r");
    if(argv[1] == NULL) {
        printf("\n");
        printf("INPUT VALID FILENAME!!!");
        return;
    }
    else if (FP == NULL) {

    }
    else {
        printf("\n");
        int Counter = -1;
        while(((one = fgetc(FP)) != EOF) && i < 23 && j < 80) {
            Counter++;
            if(Counter == 23) {
                Counter = 0;
                i++;
                j = 0;
                Length++;
            }
            Board[i][j++] = one;
            printf("%c", one);
        }
    }
    xt_par2(XT_SET_ROW_COL_POS,row=2,col=1);
    fclose(FP); /*
    for(i = 0; i < 23; i++) {
        for(j = 0; j < 80; j++) {
            if(Board[i][j] = '\0') {
                Board[i][j] = ' ';
                Board[i][80] = '\0';
            }
        }
    }
    */
    RunEditor();

    xt_par0(XT_CLEAR_SCREEN); 
    FILE *PF = fopen(argv[1], "w+"); 
    int k;  
    for(k = 0; k < 23; k++) {
        fputs(Board[k], PF);
        printf("%s", Board[k]);
    }
    fclose(PF);
    getkey_terminate();

}

        
        