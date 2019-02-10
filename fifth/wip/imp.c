// implementation file

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
#define BLOCKNUM 10


// structure for the block of text
struct Block
{ 
    char blockContent[50][500];
};

// Global variables
int blockNum;

void ReadFile(void);
void ContentProcessing(int lineIndex, struct Block Blocks);

int StrTokenizer(char *myString, char myDelim[], char *myArr[MAXCHAR]){
  
    int i = 0;
    char strCopy[200]; // prevents strtok() changes the original string
    strcpy(strCopy, myString);

    char *token = strtok(strCopy, myDelim);

    while(token != NULL)
    {
        myArr[i] = malloc(strlen(token) + 1); //needs stdlib
        strcpy(myArr[i], token);
        token = strtok(NULL, myDelim);
        i++;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    ReadFile();

    return 0;

}

// Read the file and put the content of the file into a variable
void ReadFile(void){

    FILE *fp, *fa;
    char filename[256];
    // create structures
    struct Block Blocks[BLOCKNUM]; //not dynamic because it relies on the fact that you can only have 10 blocks
    char str[MAXCHAR];
    int lineIndex = 0;

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
        while (fgets(str, MAXCHAR, fp) != NULL){
            //printf("%s", str);
            if((strncmp(str, "\n", 1) == 0)){
                printf("Reading tokens...\n");
                ContentProcessing(lineIndex, Blocks[blockNum]);
                blockNum++;
                lineIndex = 0;
            }
            strcpy(Blocks[blockNum].blockContent[lineIndex], str);
            lineIndex++;
        }

        ContentProcessing(lineIndex, Blocks[blockNum]);
    } else {
        printf("Could not open file %s",filename);
    }

    fclose(fa);
    fclose(fp);
}


void ContentProcessing(int lineIndex, struct Block Blocks){
    
    char* lineTokens[500];
    char* concatTokens[100];
    char* concatTokensRight[100];
    int m = 0, p = 0, q = 0; 
    //processing information in blocks
    //loop through lines of each blocks and then decompose the actions
    for(int n = 0; n <= lineIndex; n++){
                
        StrTokenizer(Blocks.blockContent[n], ":", lineTokens);

        while(lineTokens[m] != NULL){
            if(strncmp(lineTokens[m], "#include", 8) == 0){
                printf("%s\n\n", lineTokens[m+1]);
                //add action to read file
            } else if(strncmp(lineTokens[m], "#wait_for_joystick", 17) == 0){
                if(lineTokens[m+1] == NULL){
                    //wait for read indefinitely
                } else {
                    p = n + 1;
                    while(p <= lineIndex){
                        StrTokenizer(Blocks.blockContent[p], ":", concatTokens);
                        while(concatTokens[q] != NULL){
                            if(strncmp(concatTokens[q], "#if", 3) == 0){
                                if(strncmp(concatTokens[q+1], "JOY_LEFT", 8) == 0) {
                                    printf("%s and action : %s", concatTokens[q+2], concatTokens[q+3]);
                                } else if(strncmp(concatTokens[q+1], "JOY_UP", 6) == 0) {
                                    printf("%s and action : %s", concatTokens[q+2], concatTokens[q+3]);
                                } else if(strncmp(concatTokens[q+1], "JOY_DOWN", 8) == 0) {
                                    printf("%s and action : %s", concatTokens[q+2], concatTokens[q+3]);
                                } else if(strncmp(concatTokens[q+1], "JOY_RIGHT", 9) == 0) {
                                    printf("%s and action : %s", concatTokens[q+2], concatTokens[q+3]);
                                } else if(strncmp(concatTokens[q+1], "#else", 5) == 0){
                                
                                }
                            }
                            q++;
                        }
                        q = 0;
                            // retokenize and then goto m+1
                        p++;
                    }
                    printf("Waiting %d seconds for joystick input...\n", atoi(lineTokens[m+1])/1000);
                    sleep(atoi(lineTokens[m+1])/1000);
                }
            } else if(strncmp(lineTokens[m], "#exit", 5) == 0){
                printf("Exiting...\n");
            } else if(strncmp(lineTokens[m], "DRAW", 4) == 0){
                printf("%s\n", Blocks.blockContent[n]);
                //send Blocks.blockContent[n]
                // send concat with m+1 / or line and processing after directly on the board
            } else {
                
            }

            m++;
        } //while()
        memset(lineTokens, 0, sizeof lineTokens);
        m = 0;
    } //for()
}
