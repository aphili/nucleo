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



struct reception {
    unsigned int serial;
};

unsigned int serial;

//Threads
void *ReadFile(void *v);
void *DisplaySerial(void* reception);

//Basic functions
int SetSerial(int argc, char *argv[]);
void ReadSerial(int serial, char buffer[256]);
void PrintMenu(void);

// Main function
int main(int argc, char *argv[])
{
    char MenuStrInput;
    char *StrRef;
    
    int hSerial = SetSerial(argc, argv);
    int numThreads = 2;
    pthread_t thrs[numThreads];

    // Pass a struct in pthread_create
    struct reception rec;
    rec.serial = hSerial;

    do{
        PrintMenu(); 
        scanf("%c%*c", &MenuStrInput);
        
        switch(MenuStrInput){
            case 'o':
                StrRef = "*IDN?";
                printf("%s", StrRef);
                break;
            case 'f':
                break;
            case 'b':
                break;
            case '3':
                break;
            case '4':
                break;
            case 'i':
                break;
            case 'e':
                printf("Exiting...\n");
                // Finishing threads
                for(int i = 0; i<numThreads; i++){
                    pthread_cancel(thrs[i]);
                }
                sleep(1);
                break;
            default:
                printf("\nWrong option\n");
        }
    } while(MenuStrInput != 'e');

    /*
    //Call threads
    pthread_create(&thrs[0], NULL, ReadFile, NULL);
    pthread_create(&thrs[1], NULL, DisplaySerial, &rec);

    getchar();
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(thrs[i], NULL);
    }*/



    printf("Stop reading...\n");
    close(hSerial);
    return 0;
}

// Thread to open a file and read it
// You can only send a point to a thread, however a pointer can point to a "normal structure"
void *ReadFile(void *v){

    FILE *fp;
    FILE *fa;
    char str[MAXCHAR];
    char filename[256];

    strcpy(filename, "splash.txt");
    // Open the file once to check if there's an end of line at EOF
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
        usleep(50000); //50ms delay otherwise it is sent too quickly for the board
        }
    } else {
        printf("Could not open file %s",filename);
    }
    
    printf("\n");
    sleep(1);
    return 0;
}

// Thread to display information sent be board
void *DisplaySerial(void* reception){

    struct reception* rec = (struct reception*) reception;
    
    serial = rec->serial;

    int ReadOut = 1;
    char ReadBuffer[256];

    while(ReadOut){
        ReadSerial(serial, ReadBuffer);
        if(strncmp(ReadBuffer, "OUT", 3) != 0){
            printf("%s\n", ReadBuffer);
            memset(ReadBuffer, 0, sizeof ReadBuffer); //memset to clear out the buffer
        } else {
            memset(ReadBuffer, 0, sizeof ReadBuffer); //memset to clear out the buffer
            ReadOut = 0;
        }
    } //while()
    return 0; //always return something after thread
}

// Function to read characters sent by the board for the joystick logging
void ReadSerial(int serial, char buffer[256]){

    char chArrBuff[256];
    int index = 0;
    int stop = 1;

    while(stop){
        int n = read(serial, chArrBuff, sizeof(chArrBuff));
        if(n > 0)
        {
            for(int i = 0; i < n; i++){
                if(chArrBuff[i] != '\n'){
                    buffer[index] = chArrBuff[i];
                    index++;
                } else {
                    stop = 0; 
                }
            } //for()
        } 
    } // while()
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

// Displaying the menu
void PrintMenu(void){
  printf("\n== Program menu ==\n");
  printf("Item o: Send message \"*IDN?\"\n");
  printf("Item f: Turn OFF LED\n");
  printf("Item b: Read button state\n");
  printf("Item 3: Read joystick\n");
  printf("Item 4: Control display\n");
  printf("Item i: Enter a custom command\n");
  printf("Item e: Exit\n");
  printf("Select:\n\n");
}