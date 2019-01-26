#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

#include <fcntl.h>

#define MAXCHAR 1000


void PrintMenu(void);
void ReadChar(int serial, char buffer[256]);


int main(int argc, char *argv[])
{

  FILE *fp;
  FILE *fa;
  char UserSelection;
  char UserStrInput[256];
  char ReadBuffer[256];
  int ReadOut;
  int iRetVal;
  char str[MAXCHAR];
  char filename[256];
  

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

  o_tty.c_cflag     |=  (CREAD | CLOCAL);     // turn on READ & ignore ctrl lines
  o_tty.c_iflag     &=  ~(IXON | IXOFF | IXANY);// turn off s/w flow ctrl
  o_tty.c_lflag     &=  ~(ICANON | ECHO | ECHOE | ISIG); // make raw
  o_tty.c_oflag     &=  ~OPOST;              // make raw

  printf("Serial descriptor: %d\n",hSerial);
  printf("RetVal is :%i\n", iRetVal);

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

  do {
    
    ReadOut = 1;
    PrintMenu();
    scanf("%c%*c", &UserSelection);
    printf("Here's the user's selection : %c\n", UserSelection);

    switch(UserSelection){
      case 'o': //Case to trun on the LED in previous HW
        printf("LED is ON\n");
        UserStrInput[0] = UserSelection;
        strcat(UserStrInput, "\n");
        write( hSerial, UserStrInput, strlen(UserStrInput));
        break;
      case 'f': //case to turn on the LED in previous HW
        printf("LED is OFF\n");
        UserStrInput[0] = UserSelection;
        strcat(UserStrInput, "\n");
        write( hSerial, UserStrInput, strlen(UserStrInput));
        break;
      case 'b': //case to turn on Button in previous HW
        printf("Button is turning ON ...\n");
        UserStrInput[0] = UserSelection;
        strcat(UserStrInput, "\n");
        write( hSerial, UserStrInput, strlen(UserStrInput));
        break;
      case '3': //Logging the joystick movements
        UserStrInput[0] = UserSelection;
        // concatenate '\n' to tell the buffer on the board that it's the end 
        strcat(UserStrInput, "\n");
        write( hSerial, UserStrInput, strlen(UserStrInput));
        usleep(50000); //50ms to wait otherwise the string won't be sent fully
        printf("Joystick log : \n");
        // waiting for the JOY_SEL command to get out
        while(ReadOut){
            ReadChar(hSerial, ReadBuffer);
            if(strncmp(ReadBuffer, "OUT", 3) != 0){
              printf("%s\n", ReadBuffer);
              memset(ReadBuffer, 0, sizeof ReadBuffer); //memset to clear out the buffer
            } else {
              memset(ReadBuffer, 0, sizeof ReadBuffer); //memset to clear out the buffer
              ReadOut = 0;
            }
        }
        printf("Stop reading...\n");
        break;
      case '4': //Reading and sending a file to the board
        UserStrInput[0] = UserSelection;
        strcat(UserStrInput, "\n");
        write( hSerial, UserStrInput, strlen(UserStrInput));

        printf("Enter your file name...\n");
        //waiting for the filename
        fgets(filename, 100, stdin);
        strtok(filename, "\n");

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
            write( hSerial, str, strlen(str));
            usleep(50000); //50ms delay otherwise it is sent too quickly for the board
          }
        } else {
            printf("Could not open file %s",filename);
        }
        printf("\n");
        sleep(1);
        write( hSerial, "OUT\n", strlen("OUT\n"));
        fclose(fa);
        fclose(fp);
        break;
      case 'i': //Sending custom commands to the board
        UserStrInput[0] = '5';
        strcat(UserStrInput, "\n");
        write( hSerial, UserStrInput, strlen(UserStrInput));
        printf("Enter a command: \n");
        fgets(UserStrInput, 100, stdin);
        write( hSerial, UserStrInput, strlen(UserStrInput));
        //printf("The command is: \"%s\"\n", strtok(UserTestInput, "\n")); //strtok is used to remove the \n in the input
        memset(UserStrInput, 0, sizeof UserStrInput); //memset to clear out the buffer
        break;
      case 'e':
        printf("\nexiting...\n");
        sleep(1);
        break;
      default:
        printf("\nWrong option\n");
    }

  }while(UserSelection != 'e');

  /* close the serial port */
  close(hSerial);

  return 0;
}

// Main menu function
void PrintMenu(void){
  printf("\n== Program menu ==\n");
  printf("Item o: Turn ON LED\n");
  printf("Item f: Turn OFF LED\n");
  printf("Item b: Read button state\n");
  printf("Item 3: Read joystick\n");
  printf("Item 4: Control display\n");
  printf("Item i: Enter a custom command\n");
  printf("Item e: Exit\n");
  printf("Select:\n\n");
}

// Function to read characters sent by the board for the joystick logging
void ReadChar(int serial, char buffer[256]){

  char chArrBuf[256];
  int index = 0;
  int stop = 1;
  
  while(stop){
    int n = read(serial, chArrBuf, sizeof(chArrBuf));
    if(n > 0)
    {
      for(int i = 0; i < n; i++){
        if(chArrBuf[i] != '\n'){
          buffer[index] = chArrBuf[i];
          index++;
        } else {
          stop = 0; 
        }
      }
    } 
  } // while()
}
