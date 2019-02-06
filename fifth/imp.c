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

struct Block
{ 
    int id;
    char blockLines[50][500];
};


char *strFile;
int blockIndex;

int num;

void ReadFile(void);
void BlockProcessing(char *block, int blockId);
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
    int strIndex;
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
    
    BlockProcessing(strBlock[3], 3);

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


void BlockProcessing(char *block, int blockId){

    // i is index of characters in block
    // k is i but with reset for new lines
    int i, k, lineIndex = 0;

    struct Block Blocks[blockIndex];
    Blocks[blockId].id = blockId;

    // reading each line of the block
    while( i <= strlen(block)){
        if(block[i] == '\n'){
            lineIndex++;
            k = 0;
        } else {
            //printf("%c", block[i]);
            Blocks[blockId].blockLines[lineIndex][k] = block[i];
            k++;
        }
        i++;
    }

    ContentProcessing(lineIndex, Blocks[blockId]);
}

void ContentProcessing(int lineIndex, struct Block Blocks){
    
    char* lineTokens[500];
    char* concatTokens[100];
    char* concatTokensRight[100];
    int m,p,q = 0;
    //processing information in blocks
    //loop through lines of each blocks and then decompose the actions
    for(int n = 0; n <= lineIndex; n++){
                
        StrTokenizer(Blocks.blockLines[n], ":", lineTokens);
        
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
                        StrTokenizer(Blocks.blockLines[p], ":", concatTokens);
                        while(concatTokens[q] != NULL){
                            if(strncmp(concatTokens[q], "#if", 3) == 0){
                                printf("%s\n", concatTokens[q+1]);
                                if(strncmp(concatTokens[q+1], "JOY_LEFT", 8) == 0) {

                                } else if(strncmp(concatTokens[q+1], "JOY_UP", 6) == 0) {

                                } else if(strncmp(concatTokens[q+1], "JOY_DOWN", 8) == 0) {

                                } else if(strncmp(concatTokens[q+1], "JOY_RIGHT", 9) == 0) {

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
                sleep(1);
            } else if(strncmp(lineTokens[m], "DRAW", 4) == 0){
                printf("%s\n", Blocks.blockLines[n]);
                //send Blocks.blockLines[n]
                usleep(500000);
                // send concat with m+1 / or line and processing after directly on the board
            } else {
                
            }

            m++;
        } //while()

        m = 0;
    } //for()
}
