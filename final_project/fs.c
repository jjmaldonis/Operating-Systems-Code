# include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include "formatfs.h"
#include <math.h>
#include <sys/time.h>

/**
 * fs.c - project 4 filesystem code
 *
 *
 * Limitations
 *
 * One limitation of this code is that you cannot insert '>' into a
 * file because it is used to delimit where the filename for echo is.
 * We can do some hacking to allow '>' to be escaped but it isn't
 * worth the complexity here.
 **/

char *trimws(char *str);
int str_to_int(char in[]);

#include "printbm.c" //for printing the bitmap
#include "find_child.c"
#include "ls.c"
#include "mkdir.c"
#include "cd.c"
#include "rm.c"
#include "echo.c" //rm.c must go before this one because echo is reliant on rm
#include "cat.c"

/**
 * main() - REPL for the filesystem interface
 *
 * Loads the filesystema and reads commands until EOF is read. A
 * limited number of commands are accepted.
 *
 * returns 0 on normal exit
 **/
int main(int argc, char *argv[])
{
    char *curline;
    char *rl;
    char *cmd;
    char *rest;
    char i;

    char *fsfilename;

    if(argc < 2)
    {
        printf("%s: filesystem file argument required\n", argv[0]);
        printf("usage: %s <filesystem file>\n", argv[0]);
        return 1;
    }

    fsfilename = argv[1];

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;
    int bitmapsize_blocks = bitmapsize_bytes/BLOCKSIZE;
    printf("The bitmap is %d bytes.\n",bitmapsize_bytes);

    short cwd = bitmapsize_bytes; //two bytes! initialize it to the root directory, which is located just after the bitmap

    for(;;)
    {
        rest = curline = readline("> ");

        // if EOF with no text, then exit
        if (!curline)
        {
            break;
        }

        rest = trimws(curline);

        // if there is no text we continue
        if(!*rest)
        {
            free(curline);
            continue;
        }

        add_history(curline);

        cmd = strsep(&rest, " ");

        cmd = trimws(cmd);
        if(rest) { rest = trimws(rest); }

        if(!strcmp(cmd, "ls")) //function made
        { ls(fsfilename, cwd); }
        else if(!strcmp(cmd, "cd")) //function made
        { cwd = cd(fsfilename, rest, cwd); }
        else if(!strcmp(cmd, "mkdir") && rest) //function made
        { mkdir(fsfilename, rest, cwd); }
        else if(!strcmp(cmd, "cat") && rest) //function made
        { cat(fsfilename,rest,cwd); }
        else if(!strcmp(cmd, "rm") && rest)
        { rm(fsfilename,rest,cwd); }
        else if(!strcmp(cmd, "echo") && rest) //function made
        { echo(fsfilename, rest, cwd); }
        else if(!strcmp(cmd, "printbm") || !strcmp(cmd, "bitmap"))
        { printbm(fsfilename); }
        else if(!strcmp(cmd, "exit"))
        {
            break;
        }
        else if(!strcmp(cmd, "cwd"))
        {
            FILE *fs = fopen(fsfilename,"r+");
            fseek(fs,cwd,SEEK_SET);
            char name[8];
            fread(name,1,8,fs);
            printf("%s\n",name);
            fclose(fs);
        }
        else
        {
            printf("Invalid Command: %s", curline);
            if(rest) { printf(" %s", rest); }
            printf("\n");
        }

        free(curline);

    }
    return 0;
}


/**
 * trimws() - trim whitespace characters
 * @str: string
 *
 * Takes the argument str and increments it to point to the first
 * non-whitespace character. Also looks at the end of the string,
 * using strlen, and places a new null character '\0' after the last
 * non-whitespace character after str.
 *
 * Code snagged from: http://stackoverflow.com/q/122616
 *
 * returns an updated pointer
 **/
char *trimws(char *str)
{
    while(isspace(*str)) { str++; }

    char *end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) { end--; }

    *(end+1) = 0;

    return str;
}

int str_to_int(char in[])
{
    int num = 0;
    int i,j,add;
    for(i = 0; i < strlen(in); i++)
    {
        add = in[i]-48;
        for(j = 0; j < strlen(in)-i-1; j++)
        {
            add = add * 10;
        }
        num = num + add;
    }
    return num;
}

