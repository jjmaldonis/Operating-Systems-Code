#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "formatfs.h" //I put the FS size and ZeroSize in here, as well as the block size

/**
 * formatfs.c - initialize filesystem
 *
 * Initialize the filesystem.  This initializing all bytes to 0 and
 * creating the volume control block as necessary.
 **/

/* 2^24 */
//#define FILESYSSIZE 16777216

/* should evenly divide 16,777,216 */
/* default is 1048576 = 2^20 */
//#define ZEROSIZE 1048576

void makerootdir(FILE *fs);

/**
 * main() - main function
 *
 * Create a new filesystem file given by the first argument.
 *
 * return 0 on success.
 **/
int main(int argc, char *argv[])
{

    int i;
    char zeros[ZEROSIZE];


    if(argc < 2)
    {
        printf("%s: filesystem file argument required\n", argv[0]);
        printf("usage: %s <filesystem file>\n", argv[0]);
        return 1;
    }

    // make sure zeros is blank
    memset(zeros, 0, ZEROSIZE);

    FILE *fs = fopen(argv[1], "w");

    int numiter = FILESYSSIZE/ZEROSIZE;

    printf("writing %d chunks of size %d\n", numiter, ZEROSIZE);
    for(i=0; i < numiter; i++)
    {
        fwrite(zeros, sizeof(char), ZEROSIZE, fs);
    }

    makerootdir(fs);

    fclose(fs);

    return 0;
}

void makerootdir(FILE *fs)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;
    //open filesystem for reading and find next available block; then close fs file.

    //set bitmap to used
    fseek(fs, 0, SEEK_SET);
    char x = (0 | 1 << 0);
    fwrite( &x, 1, 1, fs);

    //name the directory
    fseek(fs, bitmapsize_bytes, SEEK_SET); //set current position
    char name_buffer[8] = "root\0\0\0\0";
    fwrite( name_buffer, 1, 8, fs);
    //set parent address to -1 so we know it is the root
    //I could leave the parent directory out completely but this makes all the directories uniform
    short y = -1;
    fwrite(&y, 2, 1, fs);
    //set timestamp
    int ts = tv.tv_sec;
    fwrite(&ts,4,1,fs);
    //set size
    short zero = 0;
    fwrite(&zero,2,1,fs);
    //set successor addr
    short zz = 0;
    fwrite(&zz,2,1,fs);
    //set type
    char type = 'd';
    fwrite(&type,1,1,fs);
}
