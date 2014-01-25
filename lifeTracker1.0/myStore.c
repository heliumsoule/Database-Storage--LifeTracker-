// myStore:  manages a linked-list database (called "myStore.dat") of little items

/* Implements commands:
    add
    stat
    display
    delete
*/

#define version "1.00"
#define author "Jing Lin and Matthew Laikhram"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

char *Usage = "Usage:\tmyStore add \"subject\" \"body\"\n\
    myStore stat\n\
    myStore display {item-no}\n\
    myStore delete {item-no}\n\
    myStore edit {item-no} \"subject\" \"body\"\n";

#define NOTHING     0
#define ADD         1
#define STAT        2
#define DISPLAY     3
#define DELETE      4
#define EDIT        5

#define TRUE    1
#define FALSE   0

// Command line arguments processed:
int command = NOTHING;
char *subject = NULL;
char *body = NULL;
char *category = NULL;
char sendToClient[1000];
int item_start = -1;
int item_end = -1;
int portno = 51000;
char input[1000];

// Prototypes:
int parseArgs(int argc, char *argv[]);
int isPositive(char *s);
int readData(void);
int add(char *subject, char *body, char *category);
int edit(char *sn, char *subject, char *body, char *category);
int writeData(void);
int display(char *sn);
int delete(char *sn);
void statis(void);
char *rstrip(char *s);
void list(void);
int runMyStore(char *arg1, char *arg2, char *arg3, char *arg4, char *arg5);
char *SkipWhitespace(char *s, int Whitespace);
int SeparateIntoFields(char *s, char **fields, int max_fields);
void server_start();

// this describes the data item on disk
struct data {
    time_t theTime;
    char theSubject[31];
    char theCategory[16];
    char theBody[141];
};

// this describes the data item in memory (data with a link to the next carrier)
struct carrier {
    struct data theData;
    struct carrier *next;
};

// These variables track the data in memory
int nitems = 0;
struct carrier *first = NULL;
struct carrier *last = NULL;

int rewrite = FALSE;        // if data changes then rewrite
char errmsg[100] = "";

int fd_read, fd_write;
char *fifo_read = "/tmp/fifo_server.dat";
// ---------------------------------- main() --------------------------------
int main(int argc, char *argv[]) {
  if (argc == 2)                            //if you input 2 arguments, then the first input becomes the port number
    portno = atoi(argv[1]);

  server_start();
  return 0;
}

// ---------------------------------------- server_start -------------------------------------------
void server_start() {
  int master_sockfd, current_sockfd;
  socklen_t client_len;
  struct sockaddr_in serv_addr, client_addr;
  char buffer[1000], c;
  int nread, i;

  // Create master socket:
  if ((master_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {       //0 is a fixed number; always use 0
    perror("Server: Cannot create master socket.");
    exit(-1);
  }

  // create socket structure
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;                          //connects to internet
  serv_addr.sin_addr.s_addr = INADDR_ANY;                  //allows anyone to connect
  serv_addr.sin_port = htons(portno);                      //designates a port number to connect to

  // bind the socket to the local port
  if (bind(master_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("Server: Error on binding");
    exit(1);
  }

  // listen
  listen(master_sockfd, 5);               //5 is a fixed number; always use 5

  client_len = sizeof(client_addr);
  printf("Server listening on port %d\n", portno);

  // master loop
  while(TRUE) {
    // block until a client connects
    if ((current_sockfd = accept(master_sockfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
      perror("Server: Error on accept()");
      exit(1);
    }

    nread = read(current_sockfd, buffer, 1000);                     //reads from client (255 character max)
    if (nread > 0) {
      // Quit command received?
      if (buffer[0] == 'q') {                                       //if string starts with q, then quit and close the server
	//write(current_sockfd, QUITTING, sizeof(QUITTING));
	close(current_sockfd);
	close(master_sockfd);
	printf("Server quitting...\n");
	exit(0);
      }
      // upper-case command received?
      else{
	char *fields[6];
	SeparateIntoFields(buffer, fields, 6);
	runMyStore(fields[1], fields[2], fields[3], fields[4], fields[5]);
	write(current_sockfd, sendToClient, 1000);       //write to client
	close(current_sockfd);
      }
    }
  }
}

int runMyStore(char *arg1, char *arg2, char *arg3, char *arg4, char *arg5){
    if (!readData()) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Error reading myStore.dat\n\n%s|\n", Usage);
        return 1;
    }
    
    if (strcmp(arg1, "add") == 0 && !add(arg2,arg3,arg4)) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Failure to add new item|\n");
        return 1;
    }
    
    if (strcmp(arg1, "stat") == 0) {
        statis();
    }
    
    if (strcmp(arg1, "display") == 0 && !display(arg2)) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Cannot display %s|\n",arg2);
        return 1;
    }
    
    if (strcmp(arg1, "delete") == 0 && !delete(arg2)) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Cannot delete %s|\n", arg2);
        return 1;
    }
    
    if (strcmp(arg1, "edit") == 0 && !edit(arg2, arg3, arg4, arg5)) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: cannot edit %s|\n",arg2);
        return 1;
    }
    
    if (rewrite)
        if (!writeData()) {
            if (errmsg[0] != '\0')
                printf("|status: ERROR: %s|\n", errmsg);
            else
                printf("|status: ERROR: Could not write the data, file may be destroyed|\n");
            return 1;
        }
    
    
    return 0;
}

// ================================ SeparateIntoFields ===================================
int SeparateIntoFields(char *s, char **fields, int max_fields) {
  int i;
    static char null_c = '\0';
    
    for (i = 0; i < max_fields; ++i) fields[i] = &null_c;
    
    for (i = 0; i < max_fields; ++i) {
        while (*s && (*s == '|' || *s == '\t' || *s == '\n')) ++s;  // skip whitespace
        if (!*s) return i;
        fields[i] = s;
        if (i == max_fields - 1) return i+1;
        while (*s && *s != '|' && *s != '\t' && *s != '\n') ++s;    // skip non-whitespace
        if (!*s) return i+1;
        *s++ = '\0';
    }
    return -1;

}

// ------------------------------- parseArgs() -------------------------------
// parse the command-line arguments, and assign the global variables from them
// return FALSE if any problem with the command-line arguments
int parseArgs(int argc, char *argv[]) {
    if (argc < 2) return FALSE;
    
    // try zero-argument commands: list and stat
    if (argc == 2) {
        if (strcmp(argv[1], "stat") == 0) {
            command = STAT;
            return TRUE;
        }
        else {
            sprintf(errmsg, "Unrecognized argument: %s", argv[1]);
            return FALSE;
        }
    }
    // try the one-argument commands: delete and display
    else if (argc == 3) {
        if (strcmp(argv[1],"delete") == 0 && isPositive(argv[2])) {
            command = DELETE;
            item_start = atoi(argv[2]);
            return TRUE;
        }
        else if (strcmp(argv[1],"display") == 0 && isPositive(argv[2])) {
            command = DISPLAY;
            item_start = atoi(argv[2]);
            return TRUE;
        }
        else {
            sprintf(errmsg, "Unrecognized 2-argument call: %s %s", argv[1],argv[2]);
            return FALSE;
        }
    }
    // try the two-argument command: add
    else if (argc == 5) {
        if (strcmp(argv[1],"add") == 0) {
            command = ADD;
            subject = argv[2];
            body = argv[3];
	        category = argv[4];
            return TRUE;
        }
        else {
            sprintf(errmsg, "Unrecognized 3-argument call: %s %s %s",argv[1],argv[2],argv[3]);
            return FALSE;
        }
    }
    // try the three-argument command: edit 
    
    else if (argc == 6) {
        if (strcmp(argv[1], "edit") == 0 && isPositive(argv[2])) {
            command = EDIT;
            item_start = atoi(argv[2]);
            subject = argv[3];
            body = argv[4];
            category = argv[5];
            //strcpy(category, argv[5]); 
            return TRUE;
        } 
        else {
            sprintf(errmsg, "Unrecognized 4-argument call: %s %s %s %s",argv[1],argv[2],argv[3],argv[4]);
            return FALSE;
        } 
    } 
    else
        return FALSE;
}

// --------------------------------- isPositive ------------------------------
// return TRUE if the string is a positive (>= 1) integer
int isPositive(char *s) {
    char *p = s;
    while (*p >= '0' && *p <= '9') 
        ++p;
    if (p != s && *p == '\0' && atoi(s) > 0)
        return TRUE;
    return FALSE;
}

// ---------------------------------- readData --------------------------------
int readData(void) {
    int i;
    struct data current_data;
    struct carrier *current_carrier;
    struct carrier *previous_carrier;
    
    FILE *fp = fopen("myStore.dat", "rb");  // read in binary file mode
    if (!fp)
        return TRUE;    // no such file, that's OK: we're doing this for the first time
    
    if (fread(&nitems, sizeof(int), 1, fp) != 1) {  // try to read nitems
        fclose(fp);
        sprintf(errmsg, "Cannot read nitems");
        return FALSE;
    }
    
    for (i = 0; i < nitems; ++i) {
        if (fread(&current_data, sizeof(struct data), 1, fp) != 1) { //try to read the next item
            fclose(fp);
            sprintf(errmsg,"Cannot read item %d\n",i+1);
            return FALSE;
        }
        if ((current_carrier = calloc(1, sizeof(struct carrier))) == NULL) {  //allocate memory
            fclose(fp);
            sprintf(errmsg,"Cannot allocate %d\n",(int) sizeof(struct carrier));
            return FALSE;
        }
        current_carrier->theData = current_data;    // load the data into the carrier
        // weave the linked-list
        if (i == 0)
            first = current_carrier;
        else
            previous_carrier->next = current_carrier;
        previous_carrier = current_carrier;
    }
    
    fclose(fp);
    last = current_carrier;
    return TRUE;
}

// ---------------------------------------- add --------------------------------------
int add(char *subject, char *body, char *category) {
    struct data current_data;
    struct carrier *current_carrier;
    
    // fill up the members of current_data
    strncpy(current_data.theSubject, subject, 30);
    current_data.theSubject[30]='\0';
    strncpy(current_data.theBody, body, 140);
    current_data.theBody[140] = '\0';
    strncpy(current_data.theCategory, category, 14);
    current_data.theCategory[13] = '\0';
    current_data.theTime = time(NULL);
    
    if ((current_carrier = calloc(1, sizeof(struct carrier))) == NULL) // allocate memory
        return FALSE;
    
    current_carrier->theData = current_data;
    if (nitems == 0)
        first = last = current_carrier;
    else {
        last->next = current_carrier;
        last = current_carrier;
    }
    
    ++nitems;
    rewrite = TRUE;
    //printf("|status: OK|\n");
    
    return TRUE;
}

// ------------------------------------- edit ------------------------------------ 
int edit(char *sn, char *subject, char *body, char *category) { 
    int n = atoi(sn);
    int i;
    struct carrier *ptr;
    struct data this_data;
    
    if (n > nitems) {
        sprintf(errmsg, "Cannot edit item %d.  Item numbers range from 1 to %d",n,nitems);
        return FALSE;
    }
    
    for (i = 1, ptr = first; i < n; ++i)
        ptr = ptr->next;
    
    this_data = ptr->theData;
    strncpy(this_data.theSubject,subject,30);
    this_data.theSubject[30] = '\0';
    strncpy(this_data.theCategory,category,14);
    this_data.theCategory[14] = '\0';
    strncpy(this_data.theBody,body,140);
    this_data.theBody[140] = '\0';
    this_data.theTime = time(NULL);
    ptr->theData = this_data;
    
    rewrite = TRUE;
    //printf("|status: OK|\n");
    return TRUE;
}


// ----------------------------------- writeData ---------------------------------
int writeData(void) {
    int i;
    struct carrier *ptr;
    struct data this_data;
    
    FILE *fp = fopen("myStore.dat", "wb");  // writing in binary
    if (!fp) {
        sprintf(errmsg, "Cannot open myStore.dat for writing.");
        return FALSE;
    }
        
    if (fwrite(&nitems, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        sprintf(errmsg, "Cannot write the nitems element");
        return FALSE;
    }
    
    for (i = 0, ptr = first; i < nitems; ++i) {
        this_data = ptr->theData;
        if (fwrite(&this_data, sizeof(struct data), 1, fp) != 1) {
            fclose(fp);
            sprintf(errmsg, "Cannot write item: %d",i+1);
            return FALSE;
        }
        ptr = ptr->next;
    }
    
    fclose(fp);
    return TRUE;
}


// ------------------------------------- stat ------------------------------
void statis(void) {
    struct tm *tp1;
    struct tm *tp2;

    if (nitems == 0) return;
    tp1 = localtime(&(first->theData.theTime));
    tp2 = localtime(&(last->theData.theTime));
    sprintf(sendToClient, "|status: OK|\n|version: %s|\n|author: %s|\n|nitems: %d|\n|first-time: %d-%02d-%02d %02d:%02d:%02d|\n|last-time: %d-%02d-%02d %02d:%02d:%02d|\n",version,author, nitems, tp1->tm_year+1900,tp1->tm_mon,tp1->tm_mday,tp1->tm_hour,tp1->tm_min,tp1->tm_sec, tp2->tm_year+1900,tp2->tm_mon,tp2->tm_mday,tp2->tm_hour,tp2->tm_min,tp2->tm_sec);
    return;
}

// ------------------------------------- rstrip ------------------------------
// removes the trailing whitespace 
char *rstrip(char *s) {
    char *p = s + strlen(s) - 1;
    while ((*p == ' ' || *p == '\t' || *p == '\n') && p >= s)
        --p;
    p[1] = '\0';
    return s;
}

// ------------------------------------ display -----------------------------
int display(char *sn) {
    int n = atoi(sn);
    int i;
    struct carrier *ptr;
    struct data this_data;
    struct tm *tp1;
    struct tm *tp;
 
    if (n > nitems) {
        sprintf(errmsg, "Cannot display item %d.  Item numbers range from 1 to %d",n,nitems);
        return FALSE;
    }
    
    for (i = 1, ptr = first; i < n; ++i)
        ptr = ptr->next;
    
    this_data = ptr->theData;
    tp1 = localtime(&this_data.theTime);
    sprintf(sendToClient, "|status: OK|\n|item: %d|\n|time: %d-%02d-%02d %02d:%02d:%02d|\n|subject: %s|\n|body: %s|\n|category: %s|\n",n,tp1->tm_year+1900,tp1->tm_mon,tp1->tm_mday,tp1->tm_hour,tp1->tm_min,tp1->tm_sec,this_data.theSubject,this_data.theBody,this_data.theCategory);
    return TRUE;
}

// ------------------------------------ delete ------------------------------
int delete(char *sn) {
    int n = atoi(sn);
    int i;
    struct carrier *ptr, *previous;

    
    if (n > nitems) {
        sprintf(errmsg, "Cannot delete item %d.  Item numbers range from 1 to %d",n,nitems);
        return FALSE;
    }
        
    previous = first;
    if (n == 1) {
        first = first->next;
        if (nitems == 1) last = NULL;
    }
    else {
        for (i = 2, ptr = first->next; i < n; ++i) {
            previous = ptr;
            ptr = ptr->next;
        }
        previous->next = ptr->next;
        if (n == nitems) last = previous;
    }
    
    --nitems;
    rewrite = TRUE;
    //printf("|status: OK|\n");
    return TRUE;
}
