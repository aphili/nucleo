#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <fcntl.h>
#include <pthread.h> 


#define MAXCHAR 1000
#define THREADS_NUM 4


// Thread variables
struct reception {
    unsigned int serial;
};

unsigned int serial;

//Threads
pthread_t thrs[THREADS_NUM];

//Mutexs 
pthread_mutex_t mutex;
pthread_cond_t cond;

//Gloal variables
int comSignal, readSignal, readFile; 
char filename[256];



//Threads
void *ReadFile(void* reception);
void *DisplaySerial(void* reception);
void *DisplayMenu(void *v);
void *MenuSelection(void *v);

//Basic functions
int SetSerial(int argc, char *argv[]);

// Main function
int main(int argc, char *argv[]) {

    int hSerial = SetSerial(argc, argv);

    // Pass a struct in pthread_create
    struct reception rec;
    rec.serial = hSerial;

    pthread_create(&thrs[0], NULL, ReadFile, &rec);
    pthread_create(&thrs[1], NULL, DisplayMenu, NULL);
    pthread_create(&thrs[2], NULL, MenuSelection, NULL);
    pthread_create(&thrs[3], NULL, DisplaySerial, &rec);

    //getchar()
    for (int i = 0; i < THREADS_NUM; ++i) {
        pthread_join(thrs[i], NULL);
    }

    printf("Stop reading...\n");
    close(hSerial);

    return 0;
}

// Displaying the menu
void *DisplayMenu(void* v){

    printf("\n== Program menu ==\n");
    printf("Item o: Send message \"*IDN?\"\n");
    printf("Item f: Turn OFF LED\n");
    printf("Item b: Read button state\n");
    printf("Item 3: Read joystick\n");
    printf("Item 4: Control display\n");
    printf("Item i: Enter a custom command\n");
    printf("Item e: Exit\n");
    pthread_mutex_lock(&mutex); //mutex lock
    while(comSignal == 0){
        pthread_cond_wait(&cond, &mutex); //wait for the condition
    }
    printf("\nBoard data coming through, press \"f\" to see it\n");
    pthread_mutex_unlock(&mutex);

    return 0;
}

// Menu selection thread
void *MenuSelection(void *v){
    
    // Menu variables
    char MenuStrInput;
    char *StrRef;
    
    do{
        scanf("%c%*c", &MenuStrInput);
        switch(MenuStrInput){
            case 'o':
                StrRef = "*IDN?";
                printf("%s", StrRef);
                break;
            case 'f':
                pthread_mutex_lock(&mutex);
                readSignal = 1;
                //fulfill condition for read
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&cond);
                break;
            case 'b':
                break;
            case '3':
                //Pass name of a file to the readfile thread
                // concatenate '\n' to tell the buffer on the board that it's the end 
                printf("Enter your file name...\n");
                //waiting for the filename
                fgets(filename, 100, stdin);
                strtok(filename, "\n");
                pthread_mutex_lock(&mutex);
                readFile= 1;
                //fulfill condition for read
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&cond);
                break;
            case '4':
                break;
            case 'i':
                break;
            case 'e':
                // Finishing threads
                for(int i = 0; i<THREADS_NUM; i++){
                    pthread_cancel(thrs[i]);
                }
                return 0;
                break;
            default:
                printf("\nWrong option\n");
        }
    } while(MenuStrInput != 'e');

    return 0;
}


// Thread to open a file and read it
// You can only send a point to a thread, however a pointer can point to a "normal structure"
void *ReadFile(void* reception){

    FILE *fp;
    FILE *fa;
    char str[MAXCHAR];
    char com[256];

    struct reception* rec = (struct reception*) reception;
    
    serial = rec->serial;

    while(1){
        pthread_mutex_lock(&mutex); //mutex lock
        while(readFile == 0){
            pthread_cond_wait(&cond, &mutex); //wait for the condition
        }

        com[0] = 3;
        strcat(com, "\n");
        write(serial, com, strlen(com));
        usleep(50000);
        fa = fopen(filename, "a+");
        fseek(fa, -1, SEEK_END); 

        //look if '\n' at the end of the file, if not put it
        if(fgetc(fa) != '\n'){
            fprintf(fa, "%s", "\n");
        }

        // Open again to read the file
        fp = fopen(filename, "r");

        if (fp){
            while (fgets(str, MAXCHAR, fp) != NULL){
            printf("%s", str);
            write( serial, str, strlen(str));
            usleep(50000); //50ms delay otherwise it is sent too quickly for the board
            }
        } else {
            printf("Could not open file %s",filename);
        }
        
        printf("\n");
        sleep(1);
        pthread_mutex_unlock(&mutex);
        readFile = 0;
    }
    return 0;
}

// Thread to display information sent by board
void *DisplaySerial(void* reception){

    int readOut = 1;
    int index = 0;
    int n, k;

    //Buffers
    char CharBuff[100];
    char ReadBuffer[256];

    struct reception* rec = (struct reception*) reception;
    
    serial = rec->serial;

    while(readOut){

        n = read(serial, CharBuff, sizeof(CharBuff));
        
        if(n > 0 && strlen(CharBuff) > 0){

            // get the characters and put it in a buffer
            if(readSignal == 0){
                pthread_mutex_lock(&mutex); // lock mutex for message
                comSignal = 1;
                //fulfill condition for read
                pthread_mutex_unlock(&mutex);
                pthread_cond_signal(&cond);
            }

            for(int i = 0; i < n; i++){
                if(CharBuff[i] != '\n'){
                    ReadBuffer[index] = CharBuff[i];
                    index++;
                } else {
                    pthread_mutex_lock(&mutex); //lock mutex for read
                    while(readSignal == 0){
                        pthread_cond_wait(&cond, &mutex); //wait for the condition
                    }
                    if(strncmp(ReadBuffer, "OUT", 3) != 0){
                        printf("%s\n", ReadBuffer);
                        index = 0;
                        memset(ReadBuffer, 0, sizeof ReadBuffer); //memset to clear out the buffer
                    } else {
                        memset(ReadBuffer, 0, sizeof ReadBuffer); //memset to clear out the buffer
                        readSignal = 0;
                        comSignal = 0;
                    }                    
                }
            } // for()
            pthread_mutex_unlock(&mutex);
        }
    } //while()
    
    return 0; //always return something after thread
}




// Serial setup
int SetSerial(int argc, char *argv[]){

    int iRetVal;
    /*open the serial port */
    int hSerial = open( "/dev/ttyACM0", O_RDWR| O_NONBLOCK | O_NDELAY );

    /* serial port settings */
    struct termios o_tty;
    // o = object
    memset (&o_tty, 0, sizeof o_tty);
    iRetVal = tcgetattr (hSerial , &o_tty);


    /* Set Baud Rate */
    cfsetospeed (&o_tty, B9600);
    cfsetispeed (&o_tty, B9600);

    /* Setting other Port Stuff */
    o_tty.c_cflag     &=  ~PARENB;        // Make 8n1
    o_tty.c_cflag     &=  ~CSTOPB;
    o_tty.c_cflag     &=  ~CSIZE;
    o_tty.c_cflag     |=  CS8;
    o_tty.c_cflag     &=  ~CRTSCTS;       // no flow control
    o_tty.c_lflag     =   0;          // no signaling chars, no echo, no canonical processing
    o_tty.c_oflag     =   0;                  // no remapping, no delays
    o_tty.c_cc[VMIN]      =   0;                  // read doesn't block
    o_tty.c_cc[VTIME]     =   5;                  // 0.5 seconds read timeout

    o_tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
    o_tty.c_iflag     &=  ~(IXON | IXOFF | IXANY);// turn off s/w flow ctrl
    o_tty.c_lflag     &=  ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    o_tty.c_oflag     &=  ~OPOST;              // make raw

    printf("Hello !\nThe Serial descriptor: %d\n",hSerial);
    printf("The RetVal is :%i\n", iRetVal);

    /* check if there is an error */
    if ( iRetVal != 0 ) {
        printf("The serial port connection isn't working");
    }
    else {
        printf("You're successfully connected to the serial port !\n");
    }

    if (argc > 1){
        printf("Welcome! The input parameter is \"%s\"\n", argv[1]);
    } else {
        printf("Welcome! No input param : I am using the default port: \"/dev/ttyACM0\"\n");
    }

    return hSerial;
}
