#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"

struct special * read_struct(struct special *s, FILE *fp)
{
    char *buffer = malloc (2); //this will be the word
    buffer[0] = '\0';
    int buffer_size = 2;
    char temp[2]; // for making ch into a string so we can strcat it with buffer
    temp[1] = '\0'; //make sure the string is terminated

    fread(&s->code,sizeof(int),1,fp); //read in the code for the cipher

    int size = 0; //keeps track of size of buffer (our word)
    while(1)
    {
        if(buffer_size <= size)
        {
            buffer = realloc(buffer,buffer_size*2);
            buffer_size = buffer_size*2;
        }

        fread(&temp[0],1,1,fp); // read in another character

        if( temp[0] == 0 || temp[0] == 127 || temp[0] == -1) //break when we have reached the null character or something else we dont want, strcat already puts \0 at the end of buffer
        { break; }

        temp[0] = temp[0] - (s->code); //de-cipher
        strcat(buffer, temp); // cat 'em together again!

        size++; //increment size
    }

    s->word = buffer; //repoint it to the correct word

    if(buffer[0] != '\0')
    {
        return s; //success
    }
    else
    {
        return 0;
    }
}


int main(int argc, char *argv[])
{
    struct special *block;

    FILE * fp;
    fp = fopen("special.data","r+");

    read_struct(block,fp);
    while( strlen(block->word) != 0 )
    {
        printf("%s ",block->word);
        read_struct(block,fp);
    }
    printf("\n");

    fclose(fp);	

    return 0;
}
