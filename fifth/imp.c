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


char *strFile;
char* lineSep[500];

int num;

void ReadFile(void);

int StrTokenizer(char *myString, char myDelim[], char *myArr[MAXCHAR]){
  
  int i = 0;
  char *token = strtok(myString, myDelim);
  
  while(token != NULL)
  {
      myArr[i] = malloc(strlen(token) + 1); //needs stdlib
      strcpy(myArr[i], token);
      token = strtok(NULL, " ");
      i++;
  }
  
  //num = (sizeof(&myArr) / sizeof(&myArr[0]));

  return 0;

}


int main(int argc, char *argv[])
{
    ReadFile();
    int blockIndex, strIndex;
    char strBlock[10][1000];
    char action;
    int i = 0;

    for(int j=0; j<=strlen(strFile); j++){
        if(strFile[j] == '\n' && strFile[j+1] == '\n'){
            blockIndex++;
            strIndex = 0;
        } else {
            strBlock[blockIndex][strIndex++] = strFile[j];
        }
    }
    
    //printf("%s\n", strBlock[0]);
    while( i <= strlen(strBlock[0])){
        if(strBlock[0][i] != '\n'){
            printf("%c", strBlock[0][i]);
        } else {
            printf("\n");
        }
        i++;
    }

    printf("%d", i);
    
    return 0;

}



void ReadFile(void){

    FILE *fp, *fa;
    char filename[256];
    long len;

    strcpy(filename, "splash.txt");

    fa = fopen(filename, "a+");
    fseek(fa, -1, SEEK_END); 

    //look if '\n' at the end of the file, if not put it
    if(fgetc(fa) != '\n'){
        fprintf(fa, "%s", "\n");
    }

    // Open again to read the file
    fp = fopen(filename, "r");

    if (fp){
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        strFile = malloc (len);
        if (strFile)
        {
            fread(strFile, 1, len, fp);
        }

    } else {
        printf("Could not open file %s",filename);
    }

    fclose(fa);
    fclose(fp);
}


//printf("%c || 0x%x\n", strBlock[0][i], strBlock[0][i]);
