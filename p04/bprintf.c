#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 64


char *mybuff;
int curPtr = 0;

void init_bprintf();
int brpintf(const char *output);

int main()
{
    mybuff = (char*) malloc(BUFSIZE); //initialize my buffer
    mybuff[0] = '\0';

    init_bprintf(); //turn off the normal buffering that minix uses

    //here is some code that shows my function works
    bprintf("Don");
    sleep(2);
    bprintf("\n Jason is \nawesome.\n");
    sleep(2);
    bprintf("I want this to be longer than the buffer size and I don't want it to have any endlines until the very end. I'll add some more stuff on here at the end just to make doubley sure that this is longer than 64 characters.");
    sleep(4);
    int i;
    for(i = 0; i < 6; i++)
    {
        bprintf(" Let's make this fairly long so that it will repeat somewhere in the middle.");
        sleep(2);
    }
    
    printf("\n"); // just so that our next command can be entered on a new line
}

int bprintf(const char *output)
{
    while(index(output,'\n') != 0) //a backslash is in the line
    {
        fwrite( mybuff,strlen(mybuff),1,stdout ); //write whatever is in the buffer to stdout
        mybuff[0] = '\0';
        curPtr = 0; //say that we dumped the buffer
        fwrite( output, strlen(output) - strlen(index(output,'\n')) + 1, 1, stdout ); //write contents of output to stdout up until and including the \n char
        output = &output[strlen(output) - strlen(index(output,'\n')) + 1]; //change output to the rest of the string
    }

    while( strlen(output) > BUFSIZE - curPtr ) //while our output is too big to fit in whats left of the buffer
    {
        strncat(mybuff,output,BUFSIZE-curPtr); //put however much can fit into the buffer
        output = &output[BUFSIZE-curPtr]; //make output the rest of the string. this will work bc BUFSIZE-curPtr < strlen(output)
        fwrite(mybuff,BUFSIZE,1,stdout); //dump the buffer
        mybuff[0] = '\0';
        curPtr = 0; //say that we dumped the buffer
    }

    //now we have a string that can fit in the buffer
    strcat(mybuff,output); //put it in the buffer
    curPtr = strlen(mybuff); //set curPtr to the next available spot

}

void init_bprintf()
{
    setvbuf(stdout, NULL, _IONBF, 0);
}
