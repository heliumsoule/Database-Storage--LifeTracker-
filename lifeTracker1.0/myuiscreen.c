#include <fcntl.h>
#include "keyboard.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> 
#include <unistd.h>
#include "xterm_control.h"
#include <netdb.h>

#define MAX_BUF 1000

char temporary[1000];

int row, col;                         //120 x 28                                
int i, j;                            //counters.
int screen = 0;                     //screen toggler, 0, 1, or 2 to determine which screen to be on (main, add, or edit)      
int CATEGORY = 8;                  //CATEGORY determines which category the user is on.
char input[1000];                 //input is the char array that takes the array of strings from myStore.
char Category[16];                   //Category, Title and Body on screen = 0, take the input from Search and sends them to catStorage,
char Title[29];                     //a temporary array that houses the records with the matching parameters. In screen = 1, C, T, and B 
char Body[141];                    //function as parameters for the record to be added, and on screen = 2, to be edited.
int recordView = 0;               //displays which record the user is on. Subtract one from the record number to determine recordView.
int maxRecord;                   //The total number of records, acting as the upper bound in for loops and the such. 
int currentRecord = 0;              //What is the current record that the user is on?
struct Stat {                      //Struct to store each of the parameters, title, body, category, and timedate as a name value pair.
  char *name;
  char *value;
};
struct Stat *data;              //Struct to determine the maximum number of records.
struct Stat *record;           //Struct to parse the input for records.
struct ArrayRecords {               //Struct to hold each of the record entries with a subject, body, category and timedate.
  char *subject;
  char *body;
  char *category;
  char *timedate; 
};
struct ArrayRecords *dataStorage;       //dataStorage, the master array that houses every record.
struct realCategory{                   //realCategory determines the array of categories.
  char *category;
};
struct realCategory *categoryList;   //List of every category that the user creates.
struct ArrayRecords *catStorage;    //Represents what is in each of the categories. 
int catCounter;                    //The total number of categories.
int tempCounter;                  //Current number of records in the current category.
int sort = 0;                    //Toggle switch for sorting. 0 is chronologically, 1 is alphabetically.
int search = 0;                 //Toggle switch for searching. 1 is ON, 0 is OFF. If on, then search will scroll through the special "category" of search items
int total;                     //The total number of name value pairs.
int reset = 0;                //Universal reset counter

char send_buffer[MAX_BUF];
char receive_buffer[MAX_BUF];
char *server;
int portno;

//PROTOTYPES
void addScreen();
void bubbleSort();
void bubbleSortTime();
void clearSearch();
void deleteScreen();
void fillCategory();
void lifeTracker();
void parseInputForStat();
int parseInputForRecords(int counter);
int readmyStoreFromChildSOCKETS(char *argv1, char *argv2, char *argv3, char *argv4, char *argv5);
void recordsHelper();
void removeBlanks();
void searchResults(int range);
int send_to_server(char *server_name, int portno, char *send_buffer, char *receive_buffer, int max_buf);
int setStat();
void setup();
void updateRecords(int range);

//Function to pipe from myStore, a TEXT database and myuiscreen.
//Modifications include considering more arguments, and changing execvp.
int readmyStoreFromChildSOCKETS(char *argv1, char *argv2, char *argv3, char *argv4, char *argv5) {
  //gather_message(send_buffer, argv+3, argc-3, MAX_BUF); 
  input[0] = 'H';
  input[1] = '\0';
  //printf("input value before send_to_server %s\n", input);
  sprintf(send_buffer, "return|%s|%s|%s|%s|%s", argv1, argv2, argv3, argv4, argv5);

   if (send_to_server(server, portno, send_buffer, input, MAX_BUF) < 0) {
    printf("Client: ERROR in send_to_server\n");
    return -1;
  }
  return 0;
}

int send_to_server(char *server_name, int portno, char *send_buffer, char *receive_buffer, int max_buf) {
  int sockfd;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int n;

  // create a socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Client cannot create socket");
    return -1;
  }
  
  if ((server = gethostbyname(server_name)) == NULL) {
    perror("Client cannot gethostbyname");
    return -1;
  }
  
  // create server addr
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  
  // connect to the server 
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)  {
      perror("Client: ERROR connecting");
      exit(1);
  }

  if (write(sockfd, send_buffer, strlen(send_buffer)) < 0) {
  printf("Client: error writing to server\n");
  return -1;
  }
  
  //server receives, returns
  *receive_buffer = '\0';
  n = read(sockfd, receive_buffer, max_buf-1);
  if (n >= 0) receive_buffer[n] = '\0';
  close(sockfd);
  return n;
}

//Function to determine the maximum number of records (from myui1)
//by separating the input into name value pairs stored in data
void parseInputForStat(){
  int i;
  char *p;
  p = input;
  for(i = 0; i < sizeof(input); i++){
    if(input[i] == '|') total++;
  }
  total /= 2;
  free(data);
  data = (struct Stat*) malloc((total + 1) * sizeof(struct Stat));
  for(i = 0; i < sizeof(input); i++) {
    if(input[i] == ':' && input[i + 1] == ' '){
      input[i] = '\0';
      input[i++] = '\0';
    }
    else if(input[i] == '|') 
      input[i] = '\0';
  }
  for(i = 0; i < total; i++){
    p++;
    data[i].name = malloc((strlen(p) + 1) * sizeof(char));
    strcpy(data[i].name, p);
    while(*p) p++;
    p+= 2;
    data[i].value = malloc((strlen(p) + 1) * sizeof(char));
    strcpy(data[i].value, p);
    while(*p) p++;
    p+= 2;
  }
  maxRecord = atoi(data[3].value);
}

//Same as parseInputForStat() except for records
void recordsHelper(){
  int i;
  char *p;
  p = input;
  for(i = 0; i < sizeof(input); i++){
    if(input[i] == '|') total++;
  }
  total /= 2;
  free(record);
  record = (struct Stat*) malloc((total + 1) * sizeof(struct Stat));
  for(i = 0; i < sizeof(input); i++){
    if(input[i] == ':' && input[i+1] == ' '){
      input[i] = '\0';
      input[i++] = '\0';
    }
    else if(input[i] == '|') 
      input[i] = '\0';
  }
  for(i = 0; i < total; i++){
    p++;
    record[i].name = p;
    while(*p) p++;
    p+= 2;
    record[i].value = p;
    while(*p) p++;
    p+= 2;
  }
}

//Takes the updated records struct in recordsHelper and stores it into dataStorage,
//the master array.
int parseInputForRecords(int counter) {
  char c;
  int i;
  free(dataStorage);
  dataStorage = (struct ArrayRecords*)malloc((counter) * sizeof(struct ArrayRecords)); 
  for(i = 0; i < counter; i++) {
    char str[80];
    sprintf(str, "%d", i+1);
    readmyStoreFromChildSOCKETS("display", str, NULL, NULL, NULL);
    recordsHelper();
    char *ARGA = record[2].value;
    char *ARGB = record[3].value;
    char *ARGC = record[4].value;
    char *ARGD = record[5].value;

    dataStorage[i].timedate = malloc((strlen(ARGA) + 1) * sizeof(char));
    dataStorage[i].subject = malloc((strlen(ARGB) + 1) * sizeof(char));
    dataStorage[i].body = malloc((strlen(ARGC) + 1) * sizeof(char));
    dataStorage[i].category = malloc((strlen(ARGD) + 1) * sizeof(char));

    strcpy(dataStorage[i].timedate, ARGA);
    strcpy(dataStorage[i].subject, ARGB);
    strcpy(dataStorage[i].body, ARGC);
    strcpy(dataStorage[i].category, ARGD); 
  }
  return 0;
}

//Determines the maxRecord, and uses that to setup the master array, dataStorage.
int setStat(){
  readmyStoreFromChildSOCKETS("stat", NULL, NULL, NULL, NULL); 
  parseInputForStat();
  parseInputForRecords(maxRecord); 
  return 0;
}

//Creates the text representation of myuiscreen by using ASCII art.
void setup() {                    //print using myui (stat)
  xt_par0(XT_CLEAR_SCREEN);
  setStat();
  xt_par2(XT_SET_ROW_COL_POS, row = 1, col = 1);
  xt_par0(XT_CH_GREEN);
  printf("-------------------------------------------------------LifeTracker------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  xt_par2(XT_SET_ROW_COL_POS, row = 2, col = 25);
  printf("Number of Records: %s | Authors: %s | Version: %s ", data[3].value, data[2].value, data[1].value);
  xt_par2(XT_SET_ROW_COL_POS,row = 3, col = 24);
  if (maxRecord != 0) printf("First Record  Time: %s | Last Record  Time: %s", data[4].value, data[5].value);
  else printf("             First Record Time: (NULL) | Last Record Time: (NULL)");
  xt_par2(XT_SET_ROW_COL_POS, row = 4, col = 1);
  printf("------------------------------------------------------------------------------------------------------------------------");
  xt_par2(XT_SET_ROW_COL_POS, row = 5, col = 1);
  xt_par0(XT_CH_BLUE);
  if(screen == 0)
    printf("------------UP/Down-Scroll between Subjects/Records/Search Left/Right-Toggle between Subjects/Records/Search------------");
  else if(screen == 1 || screen == 2)
    printf("-------------------------------------UP/Down-Switch rows Left/Right-Switch columns--------------------------------------");
  else
    printf("------------------------------------------------------------------------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  xt_par2(XT_SET_ROW_COL_POS, row = 7, col = 21); 
}

//Function to update the screen by displaying the correct records when the user scrolls
//up and down. The maximum number of records displayed at any time is four, from range to
//range + four. 
void updateRecords(int range){
  free(catStorage);
  int trow, tcol;
  char TempBodyA[71];
  char TempBodyB[71];
  int r;
  int i;
  for(i = 6; i <= 25; i++){
    xt_par2(XT_SET_ROW_COL_POS,trow=i,tcol=20);
    printf("                                                                       ");
  }  
  tempCounter = 0;
  //struct ArrayRecords *catStorage; 
  catStorage = (struct ArrayRecords*)malloc((maxRecord) * sizeof(struct ArrayRecords));
  for(i = 0; i < maxRecord; i++){
    if(strcmp(dataStorage[i].category, categoryList[CATEGORY-8].category) == 0){
      catStorage[tempCounter] = dataStorage[i];
      tempCounter++;
    }
  }
  i = 0;
  for(r = range; r < 4+range; r++){
    if(r < tempCounter){
      int k = 0;
      while(k != 70) {
        TempBodyA[k] = ' ';
        TempBodyB[k] = ' ';
        k++;
      }
      TempBodyA[70] = '\0';
      TempBodyB[70] = '\0';
      k = 0;
      xt_par2(XT_SET_ROW_COL_POS, trow = 7+5*i, tcol = 20);
      xt_par0(XT_CH_GREEN);
      printf("Record %d (%s)", r+1, catStorage[r].timedate); //get the time using myui1
      xt_par0(XT_CH_WHITE);
      xt_par2(XT_SET_ROW_COL_POS, trow = 8+5*i, tcol = 20);
      int j = 0;
      while(catStorage[r].body[j] != '\0'){
        if(j < 70) TempBodyA[j] = catStorage[r].body[j];
        else if(j < 140) TempBodyB[j%70] = catStorage[r].body[j];
        j++;
      }
      xt_par0(XT_CH_GREEN);
      printf("%s", catStorage[r].subject);
      xt_par0(XT_CH_WHITE);
      xt_par2(XT_SET_ROW_COL_POS, trow = 9+5*i, tcol = 20);
      printf("%s", TempBodyA);
      xt_par2(XT_SET_ROW_COL_POS, trow = 10+5*i, tcol = 20);
      printf("%s", TempBodyB);
      i++;
    }
  }
  xt_par2(XT_SET_ROW_COL_POS,row,col);
}

//Creates an array of every category in the myStore database.
void fillCategory(){
  free(categoryList);
  categoryList = (struct realCategory*)malloc((maxRecord) * sizeof(struct realCategory));
  catCounter = 0;
  int temp = 0;
  int a = 0;               
  int b = 0;
  for(a = 0; a < maxRecord - 1; a++) {
    for(b = 0; b < maxRecord - a - 1; b++) {
      if(strcmp(dataStorage[b].category,dataStorage[b+1].category) > 0) {
      	struct ArrayRecords temp;
      	temp = dataStorage[b+1];
      	dataStorage[b+1] = dataStorage[b];
      	dataStorage[b] = temp;
      }
    }            
  } 
  for(i = 0; i < maxRecord; i++){                                           //checks the record you are up to
    for(j = 0; j < catCounter; j++){                                        //compares its category with each category in the list
      if(strcmp(dataStorage[i].category, categoryList[j].category) == 0){   //if the current record has a category that already exists,
      	temp = 1;                                                          
      	break;
      }
    }
    if(temp == 0){                                                          //if the record has gone through the above loop without "breaking"
      categoryList[j].category = dataStorage[i].category;                   //add that record's category to the list
      catCounter++;
    }
    else temp = 0;                                                          //else, reset the temp and try again
  }
}

//removeBlanks() removes the ' ' characters in the arrays, Title, Body and Category
//by putting a null byte at the last occurence of a character.
void removeBlanks() {
  int end = 0;
  for(i = 15; i >= 0; i--){
    if(Category[i] > ' ' && Category[i] <= '~'){
      end = i+1;
      if (end < 16) Category[end] = '\0';
      break;
    }
  }
  for(i = 0; i < end; i++){
    if(Category[i] == '\0') Category[i] = ' ';
  }
  if(end == 0) Category[0] = '\0';
  end = 0;
  for(i = 28; i >= 0; i--){
    if(Title[i] > ' ' && Title[i] <= '~'){
      end = i+1;
      if (end < 29) Title[end] = '\0';
      break;
    }
  }
  for(i = 0; i < end; i++){
    if(Title[i] == '\0') Title[i] = ' ';
  }
  if(end == 0) Title[0] = '\0';
  end = 0;
  for(i = 139; i >= 0; i--){
    if(Body[i] > ' ' && Body[i] <= '~'){
      end = i+1;
      if (end < 140) Body[end] = '\0';
      break;
    }
  }
  for(i = 0; i < end; i++){
    if(Body[i] == '\0') Body[i] = ' ';
  }
  if(end == 0) Body[0] = '\0';
}

//The function to loop through dataStorage to determine the records that match 
//the input parameters for T, B, and C in Search.
void searchResults(int range){
  free(catStorage);
  int trow, tcol;
  char TempBodyA[71];
  char TempBodyB[71];
  int r;
  int i;
  removeBlanks();
  for(i = 6; i <= 25; i++){
    xt_par2(XT_SET_ROW_COL_POS,trow=i,tcol=20);
    printf("                                                                       ");
  }  
  tempCounter = 0;
  //struct ArrayRecords *catStorage; 
  catStorage = (struct ArrayRecords*)malloc((maxRecord) * sizeof(struct ArrayRecords));
  for(i = 0; i < maxRecord; i++){
    if(strstr(dataStorage[i].subject, Title) != NULL &&
       strstr(dataStorage[i].body, Body) != NULL &&
       strstr(dataStorage[i].category, Category) != NULL) { 
      catStorage[tempCounter] = dataStorage[i];
      tempCounter++;
    }
  }
  i = 0;
  //printf("Is there anything in catStorage? %s\n", catStorage[1].body);
  for(r = range; r < 4+range; r++){
    if(r < tempCounter){
      int k = 0;
      while(k != 70) {
        TempBodyA[k] = ' ';
        TempBodyB[k] = ' ';
        k++;
      }
      TempBodyA[70] = '\0';
      TempBodyB[70] = '\0';
      k = 0;
      xt_par2(XT_SET_ROW_COL_POS, trow = 7+5*i, tcol = 20);
      xt_par0(XT_CH_GREEN);
      printf("Record %d (%s)", r+1, catStorage[r].timedate); //get the time using myui1
      xt_par0(XT_CH_WHITE);
      xt_par2(XT_SET_ROW_COL_POS, trow = 8+5*i, tcol = 20);
      int j = 0;
      while(catStorage[r].body[j] != '\0'){
        if(j < 70) TempBodyA[j] = catStorage[r].body[j];
        else if(j < 140) TempBodyB[j%70] = catStorage[r].body[j];
        j++;
      }
      xt_par0(XT_CH_GREEN);
      printf("%s", catStorage[r].subject);
      xt_par0(XT_CH_WHITE);
      xt_par2(XT_SET_ROW_COL_POS, trow = 9+5*i, tcol = 20);
      printf("%s", TempBodyA);
      xt_par2(XT_SET_ROW_COL_POS, trow = 10+5*i, tcol = 20);
      printf("%s", TempBodyB);
      i++;
    }
  }
  xt_par2(XT_SET_ROW_COL_POS,row,col);
}

//Clears the search after the user exits from that portion of the screen.
void clearSearch(){
  //insert code for resetting Title, Category, and Body (for some reason we have several versions of this)
  int trow, tcol;
  xt_par2(XT_SET_ROW_COL_POS, trow = 8, tcol = 95);
  xt_par0(XT_CH_GREEN);
  printf("Category:");
  xt_par2(XT_SET_ROW_COL_POS, trow = 11, tcol = 95);
  printf("Title:");
  xt_par2(XT_SET_ROW_COL_POS, trow = 15, tcol = 95);
  printf("Body:");
  xt_par0(XT_CH_WHITE);
  xt_par2(XT_SET_ROW_COL_POS, trow = 9, tcol = 95);
  printf("------------------");
  xt_par2(XT_SET_ROW_COL_POS, trow = 12, tcol = 95);
  printf("-------");
  xt_par2(XT_SET_ROW_COL_POS, trow = 13, tcol = 95);
  printf("------------------");
  for(i = 0; i < 5; i++){
    xt_par2(XT_SET_ROW_COL_POS, trow = 16+i, tcol = 95);
    printf("-------------------------");
  }
  xt_par2(XT_SET_ROW_COL_POS, trow = 16 + 5, tcol = 95);
  printf("---------------");
  xt_par2(XT_SET_ROW_COL_POS, row, col);
}

//Alphabetical sorting with bubbleSort.
void bubbleSort() {
  int a = 0, b = 0;
  for(a = 0; a < maxRecord - 1; a++) {
    for(b = 0; b < maxRecord - a - 1; b++) {
      if(strcmp(dataStorage[b].subject,dataStorage[b+1].subject) > 0 && 
         strcmp(dataStorage[b].category,dataStorage[b+1].category) == 0) {
      	 struct ArrayRecords temp;
      	temp = dataStorage[b+1];
      	dataStorage[b+1] = dataStorage[b];
      	dataStorage[b] = temp;
      }
    }
  } 
  for(i = 0; i < 18; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 8 + i, col = 2);
    if(i < catCounter){
      printf("              ");
      xt_par2(XT_SET_ROW_COL_POS, row = 8 + i, col = 2);
      printf("%s", categoryList[i].category);
    }
    else printf("--------------");  //prints the list of subjects, utilizes the forking program/myui1  cmd(mystore, subject, i+1)
  }
  recordView = currentRecord = 0;
  xt_par2(XT_SET_ROW_COL_POS,row = 8, col = 2);
  CATEGORY = row;
  updateRecords(0);
}

//Chronological sorting with bubbleSort.
void bubbleSortTime() {
  int a = 0, b = 0;
  for(a = 0; a < maxRecord - 1; a++) {
    for(b = 0; b < maxRecord - a - 1; b++) {
      if(strcmp(dataStorage[b].timedate,dataStorage[b+1].timedate) > 0 && 
         strcmp(dataStorage[b].category,dataStorage[b+1].category) == 0) {
      	struct ArrayRecords temp;
      	temp = dataStorage[b+1];
      	dataStorage[b+1] = dataStorage[b];
      	dataStorage[b] = temp;
      }
    }
  } 
  for(i = 0; i < 18; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 8 + i, col = 2);
    if(i < catCounter){
      printf("              ");
      xt_par2(XT_SET_ROW_COL_POS, row = 8 + i, col = 2);
      printf("%s", categoryList[i].category);
    }
    else printf("--------------");  //prints the list of subjects, utilizes the forking program/myui1  cmd(mystore, subject, i+1)
  }
  recordView = currentRecord = 0;
  //updateRecords(0);
  xt_par2(XT_SET_ROW_COL_POS,row = 8, col = 2);
  CATEGORY = row;
  updateRecords(0);
}

//Function for printing the main screen (everything on the main screne)
void lifeTracker() {
  setup();
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
  fillCategory();
  xt_par0(XT_CH_GREEN);
  printf("--Categories--");
  xt_par0(XT_CH_WHITE);
  if(sort == 0) bubbleSortTime();
  else if (sort == 1) bubbleSort();
  xt_par2(XT_SET_ROW_COL_POS, row = 26, col = 2);
  xt_par0(XT_CH_BLUE); 
  printf("---F6-Sort----");
  updateRecords(0);
  xt_par2(XT_SET_ROW_COL_POS, row = 26, col = 20);
  xt_par0(XT_CH_BLUE);
  printf("-----------------------F2-Add F3-Delete F4-Edit------------------------");
  xt_par0(XT_CH_GREEN);
  xt_par2(XT_SET_ROW_COL_POS, row = 7, col = 95);
  printf("---------Search----------");
  clearSearch();  //also delete the code in between these lines
  xt_par0(XT_CH_BLUE);
  xt_par2(XT_SET_ROW_COL_POS, row = 26, col = 95);
  printf("--------F7-Search--------");
  xt_par2(XT_SET_ROW_COL_POS, row = 28, col = 1);
  xt_par0(XT_CH_BLUE);
  printf("---------------------------------------------------------F9-EXIT--------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  xt_par2(XT_SET_ROW_COL_POS,row=CATEGORY,col=2);
}

//Displays the screen to be implemented for add and edit.
void addScreen(){
  setup();
  
  for(i = 0; i < 12; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 11+i, col = 24);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = 11+i, col = 96);
    printf("|");
  }
  xt_par2(XT_SET_ROW_COL_POS, row = 11, col = 25);
  printf("-----------------------------------------------------------------------");
  xt_par0(XT_CH_GREEN);
  xt_par2(XT_SET_ROW_COL_POS, row = 12, col = 25);
  printf("Record - (---------- --:--:--)");                       //print the record number and time using myui1 
  xt_par2(XT_SET_ROW_COL_POS, row = 13, col = 25);
  printf("Category:");
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
  xt_par2(XT_SET_ROW_COL_POS,row = 14, col = 25);
}

//Displays the screen for deleting a record. User will only be able to delete a record
//after pressing F3 two times, to ascertain that she wants to delete the record.
void deleteScreen(){
  setup();
  
  for(i = 0; i < 12; i++){
    xt_par2(XT_SET_ROW_COL_POS, row = 11+i, col = 24);
    printf("|");
    xt_par2(XT_SET_ROW_COL_POS, row = 11+i, col = 96);
    printf("|");
  }
  xt_par2(XT_SET_ROW_COL_POS, row = 11, col = 25);
  printf("-----------------------------------------------------------------------");
  xt_par0(XT_CH_GREEN);
  xt_par2(XT_SET_ROW_COL_POS, row = 12, col = 25);
  printf("Record - (---------- --:--:--)");                       //print the record number and time using myui1 
  xt_par2(XT_SET_ROW_COL_POS, row = 13, col = 25);
  printf("Category:");
  xt_par2(XT_SET_ROW_COL_POS, row = 15, col = 25);
  printf("Title:");
  xt_par2(XT_SET_ROW_COL_POS, row = 18, col = 25);
  printf("Body:");
  xt_par0(XT_CH_BLUE);
  xt_par2(XT_SET_ROW_COL_POS, row = 22, col = 25);
  printf("-------------------------------F3-Delete-------------------------------");
  xt_par2(XT_SET_ROW_COL_POS, row = 28, col = 1);
  xt_par0(XT_CH_BLUE);
  printf("-------------------------------------------------------F9-EXIT----------------------------------------------------------");
  xt_par0(XT_CH_WHITE);
  xt_par2(XT_SET_ROW_COL_POS,row = 14, col = 25);
}

//Huge main function.
int main(int argc, char *argv[]) {
  if (argc < 3){                                                      //if you don't type enough arguments
    printf("Sockets client, usage:\n");
    printf("sockets_client  {IP-address} {port} \n");
    return 0;
  } 
  else {
    free(server);
    server = malloc((strlen(argv[1] + 1) * sizeof(char)));
    strcpy(server, argv[1]);
    portno = atoi(argv[2]);
  }

  int c;
  while(1){
    if(screen == 0){ 
      CATEGORY = 8;
      currentRecord = 0;
      recordView = 0;
      xt_par0(XT_CLEAR_SCREEN);
      lifeTracker();
      xt_par2(XT_SET_ROW_COL_POS,row = 8, col = 2);
    }
    while(screen == 0) {
      while((c = getkey()) == KEY_NOTHING);
        if(c == KEY_F9) screen = 3;
        else if(c == KEY_F3 && maxRecord > 0 && col == 20) screen = 4;
        else if(c == KEY_DOWN) {
          if(col == 2 && row >= 8 && row < catCounter + 7){ 
            xt_par2(XT_SET_ROW_COL_POS,CATEGORY = ++row,col);
  	        updateRecords(recordView);
  	      }
          if(col == 20 && row >= 7 && row <= tempCounter * 5 + 1){
            if(row < 22){
              xt_par2(XT_SET_ROW_COL_POS,row+=5,col);
              ++currentRecord;
  	       }
            else if(recordView < tempCounter-4){ 
              ++recordView;
              if(search == 1) searchResults(recordView);
    	        else updateRecords(recordView);
                xt_par2(XT_SET_ROW_COL_POS,row=22,col);
                ++currentRecord;
            }
          }
        	if(col > 94 && col < 120){
        	  if(row == 9)
        	    xt_par2(XT_SET_ROW_COL_POS,row = 12,col = 95);
        	  else if(row == 12)
        	    xt_par2(XT_SET_ROW_COL_POS,row = 13,col = 95);
        	  else if(row == 13)
        	    xt_par2(XT_SET_ROW_COL_POS,row = 16,col);
            else if(row < 20) 
              xt_par2(XT_SET_ROW_COL_POS,++row,col);
            else if(row == 20 && col < 110)
              xt_par2(XT_SET_ROW_COL_POS,++row,col);
            else if(row == 20 && col > 110)
              xt_par2(XT_SET_ROW_COL_POS,++row,col = 109);
        	}
        }
        else if(c == KEY_UP){
          if(col == 2 && row > 8 && row <= 18){
            xt_par2(XT_SET_ROW_COL_POS,CATEGORY = --row,col);
        	  updateRecords(recordView);
        	}
          if(col == 20 && row >= 7 && row < 24){
            if(row <= 24 && row > 7){
              xt_par2(XT_SET_ROW_COL_POS,row-=5,col);
              --currentRecord;
  	        }
            else if(recordView > 0){
              --recordView;
            if(search == 1) searchResults(recordView);
      	    else updateRecords(recordView);
                xt_par2(XT_SET_ROW_COL_POS,row=7,col);
                --currentRecord;
            }
          }
        	if(row > 9 && col > 94 && col < 120){
        	  if(row == 12)
        	    xt_par2(XT_SET_ROW_COL_POS,row = 9,col = 95);
        	  else if(row == 13)
        	    xt_par2(XT_SET_ROW_COL_POS,row = 12,col = 95);
            else if(row == 16) 
              xt_par2(XT_SET_ROW_COL_POS, row = 13, col = 95);
        	  else if(row < 22)
        	    xt_par2(XT_SET_ROW_COL_POS, --row, col);
        	}
        }
        else if(c == KEY_ENTER && col > 94 && col < 120 && row > 12 && row < 18) xt_par2(XT_SET_ROW_COL_POS,++row,col=95);
        else if((c == KEY_LEFT || c == KEY_RIGHT) && col == 2) {
        	CATEGORY = row;
        	updateRecords(recordView);
        	xt_par2(XT_SET_ROW_COL_POS,row=7,col=20);
        }
        else if((c == KEY_LEFT || c == KEY_RIGHT) && col == 20) {
          xt_par2(XT_SET_ROW_COL_POS,row=CATEGORY,col=2);
          if (search == 1) clearSearch();
          currentRecord = recordView = search = 0;
  	      updateRecords(recordView);
        }
        else if(c == KEY_LEFT && col >= 95) {
          if(col > 95)
            xt_par2(XT_SET_ROW_COL_POS,row,--col);
          else if(row == 13 && col == 95) 
            xt_par2(XT_SET_ROW_COL_POS,--row,col = 101);
          else if(row >= 17 && row <= 21 && col == 95) 
            xt_par2(XT_SET_ROW_COL_POS,--row, col = 119);
        }
        else if(c == KEY_RIGHT && col > 94) {
          if(row == 9 && col < 112) 
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          else if(row == 12 && col < 101) 
              xt_par2(XT_SET_ROW_COL_POS,row,++col);
            else if(row == 12 && col == 101) 
              xt_par2(XT_SET_ROW_COL_POS,++row,col = 95);
          else if(row == 13 && col < 112) 
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          else if(row >= 16 && row < 21 && col < 119)
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          else if(row == 21 && col < 109) 
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          else if(row >= 16 && row <= 21 && col == 119) 
            xt_par2(XT_SET_ROW_COL_POS,++row,col = 95);
        }
        else if(c == KEY_DELETE) {
          putchar(' ');
          if(row == 9) Category[col - 95] = ' ';
          else if(row == 12) Title[col - 95] = ' ';
          else if(row == 13) Title[col - 88] = ' ';
          else if(row >= 16 && row < 22) Body[(row - 16) * 25 + col - 95] = ' ';
        }
        else if(c == KEY_BACKSPACE && col >= 95) {
          if((row == 9 || row == 12) && col > 95) {
	    xt_par2(XT_SET_ROW_COL_POS,row,--col);
            putchar(' ');
            Category[col - 95] = ' ';
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if((row == 9 || row == 12) && col == 95) {
            putchar(' ');
            Title[col - 95] = ' ';
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if(row == 13 && col > 95) {
	          xt_par2(XT_SET_ROW_COL_POS,row,--col);
            putchar(' ');
            Title[col - 88] = ' ';
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if(row == 13 && col == 95) {
	          xt_par2(XT_SET_ROW_COL_POS,--row,col = 101);
            putchar(' ');
            Title[col - 88] = ' ';
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if(row >= 16 && row < 22 && col > 95) {
	          xt_par2(XT_SET_ROW_COL_POS,row,--col);
            putchar(' ');
            Body[(row - 16) * 25 + col - 95] = ' ';
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if(row == 16 && col == 95) {
            putchar(' ');
            Body[(row - 16) * 25 + col - 95] = ' ';
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if(row > 16 && row < 22 && col == 95) {
	          xt_par2(XT_SET_ROW_COL_POS,--row,col = 119);
            putchar(' ');
            Body[(row - 16) * 25 + col - 95] = ' ';
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
        } 
        else if((c >= ' ' && c <= '~') && col >= 95 && col < 120){
          if(row == 9 && col < 112) {
            putchar(c);
            Category[col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          }
          else if(row == 9 && col == 112) {
            putchar(c);
            Category[col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if(row == 12 && col < 101) {
            putchar(c);
            Title[col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          }
          else if(row == 12 && col == 101) {
            putchar(c);
            Title[col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,++row,col=95);
          }
          else if(row == 13 && col < 112) {
            putchar(c);
            Title[col - 88] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          }
          else if(row == 13 && col == 112) {
            putchar(c);
            Title[col - 88] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
          else if(row >= 16 && row < 21 && col < 119) {
            putchar(c);
            Body[(row - 16) * 25 + col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          }
          else if(row >= 16 && row < 21 && col == 119) {
            putchar(c);
            Body[(row - 16) * 25 + col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,++row,col = 95);
          }
          else if(row == 21 && col < 109) {
            putchar(c);
            Body[(row - 16) * 25 + col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,++col);
          }
          else if(row == 21 && col == 109) {
            putchar(c);
            Body[(row - 16) * 25 + col - 95] = c;
            xt_par2(XT_SET_ROW_COL_POS,row,col);
          }
        }
        else if(c == KEY_F2)
          screen = 1;
        else if(c == KEY_F4 && col == 20 && maxRecord != 0)
          screen = 2;
        else if(c == KEY_F6) {
        	if(sort == 0){
        	  bubbleSort();
        	  sort = 1;
        	}
        	else if (sort == 1){
        	  bubbleSortTime();
        	  sort = 0;
        	}
        }
        else if (c == KEY_F7){
        	if(col < 95) {
            xt_par2(XT_SET_ROW_COL_POS,row = 9,col = 95);
            for(reset = 0; reset < 29; reset++) Title[reset] = ' ';
            for(reset = 0; reset < 140; reset++) Body[reset] = ' ';
            for(reset = 0; reset < 16; reset++) Category[reset] = ' ';
          }
        	else if(Title[0] == ' ' && Body[0] == ' ' && Category[0] == ' ') {
            xt_par2(XT_SET_ROW_COL_POS,row = CATEGORY,col = 2);
            clearSearch();
            updateRecords(0);
          }
          else if(Title[0] != ' ' || Body[0] != ' ' || Category[0] != ' ') {
            clearSearch();
	          search = 1;
            searchResults(0);
            xt_par2(XT_SET_ROW_COL_POS,row = 7, col = 20);
          }
        }
    }
    if (screen == 1 || screen == 2){
      for(reset = 0; reset < 29; reset++) Title[reset] = ' ';
      for(reset = 0; reset < 140; reset++) Body[reset] = ' ';
      for(reset = 0; reset < 16; reset++) Category[reset] = ' ';
      addScreen();
      if (screen == 2){ 
        char TempBodyA[71]; 
        char TempBodyB[71];
        int k = 0;
        while(k != 70) {
      	  TempBodyA[k] = ' ';
      	  TempBodyB[k] = ' ';
      	  k++;
        }
        k = 0; 
        xt_par2(XT_SET_ROW_COL_POS, row = 12, col = 25);
        xt_par0(XT_CH_GREEN);
        printf("Record %d (%s)", currentRecord+1, catStorage[currentRecord].timedate); //get the time using myui1
        xt_par0(XT_CH_WHITE);
        xt_par2(XT_SET_ROW_COL_POS, row = 14, col = 25);
        printf("%s", catStorage[currentRecord].category);
        int j = 0; 
        while(catStorage[currentRecord].body[j] != '\0'){
          if (j < 70) TempBodyA[j] = catStorage[currentRecord].body[j];
          else if (j < 140) TempBodyB[j%70] = catStorage[currentRecord].body[j];
          j++;
        }
        TempBodyA[70] = '\0';
        TempBodyB[70] = '\0';
        xt_par2(XT_SET_ROW_COL_POS, row = 16, col = 25);
        printf("%s", catStorage[currentRecord].subject);
        xt_par0(XT_CH_WHITE);
        xt_par2(XT_SET_ROW_COL_POS, row = 19, col = 25);
        printf("%s", TempBodyA); 
        xt_par2(XT_SET_ROW_COL_POS, row = 20, col = 25); 
        printf("%s", TempBodyB); 
        xt_par2(XT_SET_ROW_COL_POS, row = 14, col = 25); 
        strncpy(Title, catStorage[currentRecord].subject, sizeof(Title));
        strncpy(Body, catStorage[currentRecord].body, sizeof(Body));
        strncpy(Category, catStorage[currentRecord].category, sizeof(Category));
      }
    } 
    while(screen == 1 || screen == 2) {
      while((c = getkey()) == KEY_NOTHING);
      if (c == KEY_F9){
        screen = 0;
        xt_par0(XT_CLEAR_SCREEN);
      }
      else if (c == KEY_DOWN){
        if(row == 14) 
          xt_par2(XT_SET_ROW_COL_POS, row = 16, col = 25);
        else if(row == 16)
          xt_par2(XT_SET_ROW_COL_POS,row = 19,col = 25);
    	else if(row < 20)
    	  xt_par2(XT_SET_ROW_COL_POS,++row,col);
      }
      else if (c == KEY_UP){    
        if(row == 16)
          xt_par2(XT_SET_ROW_COL_POS, row = 14, col = 25);
        if(row == 19)
          xt_par2(XT_SET_ROW_COL_POS,row = 16,col = 25);
    	else if(row == 20)
    	  xt_par2(XT_SET_ROW_COL_POS,--row,col);
      }
      else if(c == KEY_LEFT && col > 25 && col < 95) xt_par2(XT_SET_ROW_COL_POS,row,--col);
      else if(c == KEY_LEFT && col == 25 && row == 20) xt_par2(XT_SET_ROW_COL_POS,--row,col=94);
      else if(c == KEY_RIGHT && col > 24 && col < 37 && row == 14) xt_par2(XT_SET_ROW_COL_POS,row,++col);
      else if(c == KEY_RIGHT && col > 24 && col < 53 && row == 16) xt_par2(XT_SET_ROW_COL_POS,row,++col);
      else if(c == KEY_RIGHT && col > 24 && col < 94 && row > 18) xt_par2(XT_SET_ROW_COL_POS,row,++col);
      else if(c == KEY_RIGHT && col == 94 && row == 19) xt_par2(XT_SET_ROW_COL_POS,++row,col=25);   
      else if (c == KEY_ENTER && row > 18 && row < 20) xt_par2(XT_SET_ROW_COL_POS,++row,col=25);      
      else if (c == KEY_BACKSPACE && col >= 25 && col < 95 && ((row > 18 && row < 21) || row == 16 || row == 14)){
      	if(row == 14 && col > 25 && col < 39) {
      	  xt_par2(XT_SET_ROW_COL_POS,row,--col);
      	  putchar(' ');
      	  Category[col - 25] = ' ';
      	}
      	else if(row == 16 && col > 25 && col <= 54) {
      	  xt_par2(XT_SET_ROW_COL_POS,row,--col);
      	  putchar(' ');
      	  Title[col - 25] = ' ';
      	}
      	else if(row == 19 && col > 25 && col <= 95) {
      	  xt_par2(XT_SET_ROW_COL_POS,row,--col);
      	  putchar(' ');
      	  Body[col - 25] = ' ';
      	}
      	else if(row == 20 && col > 25 && col <= 95) {
      	  xt_par2(XT_SET_ROW_COL_POS,row,--col);
      	  putchar(' ');
      	  Body[col + 45] = ' ';
      	}
      	else if(row == 20){                                          
      	  xt_par2(XT_SET_ROW_COL_POS,--row,col=94);
      	  putchar(' ');
      	  Body[69] = ' ';
      	}
	      xt_par2(XT_SET_ROW_COL_POS,row,col);
      }
      else if (c == KEY_DELETE) {
        if(row == 14 && col >= 25 && col < 38) {
      	  putchar(' ');
      	  Category[col - 25] = ' ';
	    }
    	else if(row == 16 && col >= 25 && col < 54) {
    	  putchar(' ');
    	  Title[col - 25] = ' ';
    	}
    	else if(row == 19 && col >= 25 && col <= 95) {
    	  putchar(' ');
    	  Body[col - 25] = ' ';
    	}
    	else if(row == 20 && col >= 25 && col <= 95) {
    	  putchar(' ');
    	  Body[col + 45] = ' ';
    	}
      xt_par2(XT_SET_ROW_COL_POS,row,col);
      } 
      else if((c >= ' ' && c <= '~') && col >= 25 && col < 95) {
      	if(row == 14 && col >= 25 && col < 40) {
      	  putchar(c);
      	  Category[col - 25] = c;
      	}
      	else if(row == 16 && col >= 25 && col < 54) {
      	  putchar(c);
      	  Title[col - 25] = c;
      	}
      	else if(row == 19 && col >= 25 && col <= 95) {
      	  putchar(c);
      	  Body[col - 25] = c;
      	}
      	else if(row == 20 && col >= 25 && col <= 95) {
      	  putchar(c);
      	  Body[col + 45] = c;
      	}
      	if (col < 94 && !(row == 16 && col > 52) && !(row == 14 && col > 36)){
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
        if(Title[0] != ' ' || Body[0] != ' ' || Category[0] != ' ') {
          removeBlanks();
          readmyStoreFromChildSOCKETS("add", Title, Body, Category, NULL);
        }
        int k = 0;
        while(k != 29) Title[k++] = ' ';
        k = 0;
        while(k != 140) Body[k++] = ' ';
        k = 0;
        while(k != 15) Category[k++] = ' ';
        maxRecord++;
        screen = 0;
      }
      else if(c == KEY_F2 && screen == 2 /* && there has been a valid change in the record */){
      //update record
	    removeBlanks();
      char str[80];
    	for(i = 0; i < maxRecord; i++){
    	  if(strcmp(catStorage[currentRecord].subject, dataStorage[i].subject) == 0 &&
    	     strcmp(catStorage[currentRecord].body, dataStorage[i].body) == 0 &&
    	     strcmp(catStorage[currentRecord].category, dataStorage[i].category) == 0 &&
    	     strcmp(catStorage[currentRecord].timedate, dataStorage[i].timedate) == 0){//looks for corresponding record in the actual record list
    	    sprintf(str, "%d", i+1);
    	    break;
    	  }
    	}
      readmyStoreFromChildSOCKETS("edit", str, Title, Body, Category);
      int k = 0;
      while(k != 29) Title[k++] = ' ';
      k = 0;
      while(k != 140) Body[k++] = ' ';
      k = 0;
      while(k != 15) Category[k++] = ' ';
      screen = 0;
    }
  }
    if (screen == 4){
      for(reset = 0; reset < 29; reset++) Title[reset] = ' ';
      for(reset = 0; reset < 140; reset++) Body[reset] = ' ';
      for(reset = 0; reset < 16; reset++) Category[reset] = ' ';
      deleteScreen();
      if (screen == 4){ 
        char TempBodyA[71]; 
        char TempBodyB[71];
        int k = 0;
        while(k != 70) {
      	  TempBodyA[k] = ' ';
      	  TempBodyB[k] = ' ';
      	  k++;
        }
        k = 0;
        xt_par2(XT_SET_ROW_COL_POS, row = 12, col = 25);
        xt_par0(XT_CH_GREEN);
        printf("Record %d (%s)", currentRecord+1, catStorage[currentRecord].timedate); //get the time using myui1
        xt_par0(XT_CH_WHITE);
        xt_par2(XT_SET_ROW_COL_POS, row = 14, col = 25);
        printf("%s", catStorage[currentRecord].category);
        int j = 0; 
        while(catStorage[currentRecord].body[j] != '\0'){
          if (j < 70) TempBodyA[j] = catStorage[currentRecord].body[j];
          else if (j < 140) TempBodyB[j%70] = catStorage[currentRecord].body[j];
          j++;
        }
        TempBodyA[70] = '\0';
        TempBodyB[70] = '\0';
        xt_par2(XT_SET_ROW_COL_POS, row = 16, col = 25);
        printf("%s", catStorage[currentRecord].subject);
        xt_par0(XT_CH_WHITE);
        xt_par2(XT_SET_ROW_COL_POS, row = 19, col = 25);
        printf("%s", TempBodyA); 
        xt_par2(XT_SET_ROW_COL_POS, row = 20, col = 25); 
        printf("%s", TempBodyB); 
        xt_par2(XT_SET_ROW_COL_POS, row = 14, col = 25); 
        strncpy(Title, catStorage[currentRecord].subject, sizeof(Title));
        strncpy(Body, catStorage[currentRecord].body, sizeof(Body));
        strncpy(Category, catStorage[currentRecord].category, sizeof(Category));
        //print corresponding record in correct locations using myui1 
      }
    } 
    while(screen == 4) {
      while((c = getkey()) == KEY_NOTHING);
        if (c == KEY_F9){
          screen = 0;
          xt_par0(XT_CLEAR_SCREEN);
        }
        else if (c == KEY_F3){
        char str[80];
        for(i = 0; i < maxRecord; i++){
        	if(strcmp(catStorage[currentRecord].subject, dataStorage[i].subject) == 0 &&
        	   strcmp(catStorage[currentRecord].body, dataStorage[i].body) == 0 &&
        	   strcmp(catStorage[currentRecord].category, dataStorage[i].category) == 0 &&
        	   strcmp(catStorage[currentRecord].timedate, dataStorage[i].timedate) == 0){           //looks for corresponding record in the actual record list
        	  sprintf(str, "%d", i+1);
        	  break;
        	}
        }
        readmyStoreFromChildSOCKETS("delete", str, NULL, NULL, NULL);
        maxRecord--;
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




