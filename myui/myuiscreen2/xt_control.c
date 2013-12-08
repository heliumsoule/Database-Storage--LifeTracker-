#include <stdio.h>
#include <string.h>

void main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage xt_control c c nn c ...\nwhere c is a char and nn is the ASCII code for a char\nFor instance:\nxt_control 27 [ 2 J\n\n");
		return;
	}
	
	int i;
	char input[20];
	
	for (i = 1; i < argc; ++i) {
		if (strlen(argv[i]) == 1)
			printf("%c",argv[i][0]);
		else
			printf("%c",atoi(argv[i]));
	}
	printf("%c[6n",27);		// get report of cursor position
	for (i = 0; (input[i]=getchar()) != 'R'; ++i) ;
	input[i++]='R';
	input[i]='\0';
	printf("reported: |%s|\n",input);
}

	
	