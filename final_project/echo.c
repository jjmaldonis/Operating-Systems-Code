void successor_file(char *fsfilename,char* rest, short cwd);

void echo(char *fsfilename, char * rest, short cwd);
void echo(char *fsfilename, char * rest, short cwd)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;
    char *dest;
    dest = rest;
    rest = strsep(&dest, ">");
    if(!dest)
    {
        printf("%s\n", rest);
    }
    else if(!*dest)
    {
        printf("Error: No output file specified\n");
    }
    else //destination fiile specified
    {
        dest = trimws(dest);
        rest = trimws(rest);

        //make some variables
        short file_addr;
        char name[8]; name[0] ='\0';
        int openblock = -1; //a value of -1 means we didn't find an open block
        int value_opensegment;
        char in; //input from filesystem for reading in the bitmap
        char braek;

        //find file if it exists - looking in children of cwd
        file_addr = find_child(fsfilename,dest,cwd);

        char type;
        if(file_addr != 0) //file exists
        {
            fseek(fs,file_addr+18,SEEK_SET);
            fread(&type,1,1,fs);
            if(type == 'f')
            {
                //remove it
                rm(fsfilename,dest,cwd);
            }
            else
            {
                printf("You specified a directory, not a file.\n");
            }
        }

        short cwd_size;
        fseek(fs,cwd+14,SEEK_SET);
        fread(&cwd_size,2,1,fs);
        if( cwd_size == (BLOCKSIZE-20)/2 )
        {
            printf("Your directory is full.\n");
        }
        else //directory is not full
        {

            if(type == 'f' || file_addr == 0)
            {
                //now the file doesn't exist so we need to find an open block
                fseek(fs, 0, SEEK_SET); //set current position to beginning of file 
                braek = 0;
                int i;
                for(i = 0; i < bitmapsize_bytes; i++)
                {
                    fread(&in,1,1,fs);
                    if(in != 255) //something in this 8 block segment is available
                    {
                        int j;
                        for(j = 0; j < 8; j++)
                        {
                            if( !( in & (1 << j) ) ) //available
                            {
                                openblock = i*8 + j; //now we can find the OB by dividing by 8 and taking OB mod 8
                                value_opensegment = in;
                                braek = 1;
                                break;
                            }
                        }
                    }
                    if(braek) {break;}
                }
                if(openblock == -1)
                {
                    printf("An open block was not found. Your filesystem could be full. Or there could be an error... If that's the case, you should probably just ignore it.\n");
                }
                else
                {
                    //get epoch
                    fseek(fs,bitmapsize_bytes+10,SEEK_SET);
                    int epoch;
                    fread(&epoch,4,1,fs);
                    //set bitmap to used
                    fseek(fs, openblock/8, SEEK_SET);
                    char x = (value_opensegment | 1 << (openblock%8));
                    fwrite( &x, 1, 1, fs);

                    //go to the parent directory and increment the parent's size by 2
                    fseek(fs,cwd+14,SEEK_SET);
                    //short size;
                    short p_size;
                    fread(&p_size,2,1,fs);
                    fseek(fs,cwd+14,SEEK_SET);
                    p_size = p_size + 2;
                    fwrite(&p_size,2,1,fs);
                    //now we need to give it this child
                    fseek(fs,4,SEEK_CUR);
                    short child_addr;
                    fread(&child_addr,2,1,fs);
                    short new_block_addr;
                    i = 0;
                    while(child_addr != 0 && i < (BLOCKSIZE-20)/2)
                    {
                        fread(&child_addr,2,1,fs);
                        i++;
                    }
                    if(i != (BLOCKSIZE-20)/2) //we aren't out of space
                    {
                        fseek(fs,cwd+20 + (2*i),SEEK_SET);
                        new_block_addr = bitmapsize_bytes + (openblock * BLOCKSIZE);
                        fwrite(&new_block_addr, 2,1,fs);
                    }
                    else
                    {
                        if(i == (BLOCKSIZE-20)/2) //the directory is full
                        {
                        }
                    }

                    //go to the correct block and name the file
                    fseek(fs, new_block_addr, SEEK_SET);
                    char *name_buf = malloc(8);
                    strcpy(name_buf,dest);
                    fwrite(name_buf, 1, 8, fs);
                    //set parent address
                    fwrite(&cwd, 2, 1, fs);
                    //set timestamp
                    struct timeval tv;
                    gettimeofday(&tv,NULL);
                    int time = tv.tv_sec;
                    fwrite(&time,4,1,fs);
                    //set size
                    short size = strlen(rest);
                    fwrite(&size,2,1,fs);
                    //set successor_addr
                    short zz = 0;
                    fwrite(&zz,2,1,fs);
                    //set type
                    char type = 'f';
                    fwrite(&type,1,1,fs);

                    fseek(fs,1,SEEK_CUR); //+1 for the extra space

                    //write to the file
                    if(strlen(rest) <= BLOCKSIZE-20) //it will all fit in this block
                    {
                        fwrite(rest,1,strlen(rest),fs); //write the length of rest into the file 1 char at a time
                    }
                    else //we need to make more space after we write as much as we can to this block
                    {
                        char *first_half = malloc( (BLOCKSIZE-20) * sizeof(char) +1 ); //leave room for the null char
                        strncpy(first_half,rest,BLOCKSIZE-20);
                        first_half[BLOCKSIZE-20] = '\0'; //if rest < BLOCKSIZE-20 then there will be two null chars so this one wont matter
                        int k = 0;
                        for(k=0;k<BLOCKSIZE-20;k++)
                        {
                            rest++;
                        }
                        fwrite(first_half,1,strlen(first_half),fs);
                        //get a new block and write to that one (recursive function)
                        successor_file(fsfilename,rest,new_block_addr);
                    }
                }
            }
        }
    }
    fclose(fs);
} //end of function echo

void successor_file(char *fsfilename,char* rest, short prev_block_addr)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;

    //make some variables
    short file_addr;
    char name[8]; name[0] ='\0';
    short size;
    int openblock = -1; //a value of -1 means we didn't find an open block
    int value_opensegment;
    char in; //input from filesystem for reading in the bitmap
    char braek;

    //we need to find an open block
    fseek(fs, 0, SEEK_SET); //set current position to beginning of file 
    braek = 0;
    int i;
    for(i = 0; i < bitmapsize_bytes; i++)
    {
        fread(&in,1,1,fs);
        if(in != 255) //something in this 8 block segment is available
        {
            int j;
            for(j = 0; j < 8; j++)
            {
                if( !( in & (1 << j) ) ) //available
                {
                    openblock = i*8 + j; //now we can find the OB by dividing by 8 and taking OB mod 8
                    value_opensegment = in;
                    braek = 1;
                    break;
                }
            }
        }
        if(braek) {break;}
    }
    if(openblock == -1)
    {
        printf("An open block was not found. Your filesystem could be full. Or there could be an error... If that's the case, you should probably just ignore it.\n");
    }
    else
    {

        //get epoch
        fseek(fs,bitmapsize_bytes+10,SEEK_SET);
        int epoch;
        fread(&epoch,4,1,fs);
        //set bitmap to used
        fseek(fs, openblock/8, SEEK_SET);
        char x = (value_opensegment | 1 << (openblock%8));
        fwrite( &x, 1, 1, fs);

        short new_block_addr = bitmapsize_bytes + (openblock*BLOCKSIZE);
        fseek(fs, new_block_addr+20, SEEK_SET); //go to the children

        //write to the file
        if(strlen(rest) <= BLOCKSIZE-20) //it will all fit in this block
        {
            fwrite(rest,1,strlen(rest),fs); //write the length of rest into the file 1 char at a time

            //fix parent's successor addr
            fseek(fs,prev_block_addr+16,SEEK_SET);
            fwrite(&new_block_addr,2,1,fs);

        }
        else //we need to make more space after we write as much as we can to this block
        {
            char *first_half = malloc( (BLOCKSIZE-20) * sizeof(char) +1 ); //leave room for the null char
            strncpy(first_half,rest,BLOCKSIZE-20);
            first_half[BLOCKSIZE-20] = '\0'; //if rest < BLOCKSIZE-20 then there will be two null chars so this one wont matter
            int k = 0;
            for(k=0;k<BLOCKSIZE-20;k++)
            {
                rest++;
            }
            fwrite(first_half,1,strlen(first_half),fs);

            //fix parent's successor addr
            fseek(fs,prev_block_addr+16,SEEK_SET);
            fwrite(&new_block_addr,2,1,fs);

            //get a new block and write to that one (do the same thing again)
            successor_file(fsfilename,rest,new_block_addr);
        }
    }
    fclose(fs);
}
