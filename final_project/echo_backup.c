void successor_file(char *fsfilename,char* rest, short cwd);

void echo(char *fsfilename, char * rest, short cwd);
void echo(char *fsfilename, char * rest, short cwd)
{
    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;
    char *dest;
    dest = rest;
    rest = strsep(&dest, ">");
    if(!dest)
    {
        //printf("%s\n", rest);
    }
    else if(!*dest)
    {
        printf("Error: No output file specified\n");
    }
    else
    {
        dest = trimws(dest);
        rest = trimws(rest);

        //find file if it exists - looking in children of cwd
        FILE *fs = fopen(fsfilename, "r+b");
        fseek(fs,cwd+20,SEEK_SET);
        short file_addr;
        fread(&file_addr,sizeof(short),1,fs);
        char name[8]; name[0] ='\0';
        short size;
        int openblock = -1; //a value of -1 means we didn't find an open block
        int value_opensegment;
        char in; //input from filesystem for reading in the bitmap
        char braek;
        if(file_addr != 0)
        {
            fseek(fs,file_addr,SEEK_SET);
            fread(name,1,8,fs);
        }
        int i = 1;
        while(strcmp(name,dest) != 0 && i < 201 && file_addr != 0)
        {
            fseek(fs,cwd+20+(2*i),SEEK_SET);
            fread(&file_addr,sizeof(short),1,fs);
            if(file_addr != 0)
            {
                fseek(fs,file_addr,SEEK_SET);
                fread(name,1,8,fs);
            }
            i++;
        }

        if(file_addr != 0 && i != 246) //file exists
        {
            //reset file size
            fseek(fs,file_addr+14,SEEK_SET);
            size = strlen(rest);
            fwrite(&size,2,1,fs);

            //write to file; we don't need to worry about erasing it first because we do things using its size anyways
            fseek(fs,4,SEEK_CUR);
            fwrite(rest,1,strlen(rest),fs); //write the length of rest into the file 1 char at a time
        }
        else //file doesn't exist
        {
            //make file
            //fine an open spot
            fseek(fs, 0, SEEK_SET); //set current position to beginning of file 
            braek = 0;
            for(i = 0; i < bitmapsize_bytes; i++)
            {
                fread(&in,1,1,fs);
                //in_int = (int) in[0];
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
            if(openblock == -1) { printf("An open block was not found. Your filesystem could be full. Or there could be an error... If that's the case, ignore it.\n"); }

            //get epoch
            fseek(fs,bitmapsize_bytes+10,SEEK_SET);
            int epoch;
            fread(&epoch,4,1,fs);

            //set bitmap to used
            fseek(fs, openblock/8, SEEK_SET);
            char x = (value_opensegment | 1 << (openblock%8));
            fwrite( &x, 1, 1, fs);
            //name the directory
            fseek(fs, bitmapsize_bytes + (openblock * BLOCKSIZE), SEEK_SET);
            char *name_buf = malloc(8);
            strcpy(name_buf,dest);
            fwrite(name_buf, 1, 8, fs);
            //set parent address
            fwrite(&cwd, sizeof(short), 1, fs);
            //set timestamp
            struct timeval tv;
            gettimeofday(&tv,NULL);
            int z = tv.tv_sec - epoch;
            fwrite(&z,4,1,fs);
            //set size
            short size = strlen(rest);
            fwrite(&size,2,1,fs);
            //set successor_addr
            //FIX
            short zz = 0;
            fwrite(&zz,2,1,fs);
            //set type
            char type = 'f';
            fwrite(&type,1,1,fs);

            //write to the file
            if(strlen(rest) <= BLOCKSIZE-20) //it will all fit in this block
            {
                fseek(fs,1,SEEK_CUR); //+1 for the extra space
                fwrite(rest,1,strlen(rest),fs); //write the length of rest into the file 1 char at a time
            }
            else
            {
                char *first_half = malloc( (BLOCKSIZE-20) * sizeof(char) +1 ); //leave room for the null char
                strncpy(first_half,rest,BLOCKSIZE-20);
                first_half[BLOCKSIZE-20] = '\0'; //if rest < BLOCKSIZE-20 then there will be two null chars so this one wont matter
                int k = 0;
                for(k=0;k<BLOCKSIZE-20;k++)
                {
                    rest++;
                }
                fseek(fs,1,SEEK_CUR);
                fwrite(first_half,1,strlen(first_half),fs);
                successor_file(fsfilename,rest,bitmapsize_bytes+(openblock*BLOCKSIZE));

            }

            //go to the parent directory and increment the parent's size by 2
            fseek(fs,cwd+14,SEEK_SET);
            //short size;
            fread(&size,2,1,fs);
            fseek(fs,cwd+14,SEEK_SET);
            size = size + 2;
            fwrite(&size,2,1,fs);
            //now we need to give it this child
            fseek(fs,4,SEEK_CUR);
            short child_addr;
            fread(&child_addr,2,1,fs);
            i = 0;
            while(child_addr != 0 && i < 246)
            {
                fread(&child_addr,2,1,fs);
                i++;
            }
            fseek(fs,cwd+20 + (2*i),SEEK_SET);
            short y = bitmapsize_bytes + (openblock * BLOCKSIZE);
            fwrite(&y, 2,1,fs);


        }
        fclose(fs);
    }
}

void successor_file(char *fsfilename,char* rest, short cwd, short succ_addr)
{
    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;
    FILE *fs = fopen(fsfilename,"r+");

    short this_addr;

    if(succ_addr == 0) //if there isnt already a succ. addr
    {
        //fine an open spot
        fseek(fs, 0, SEEK_SET); //set current position to beginning of file 
        int openblock = -1; //a value of -1 means we didn't find an open block
        int value_opensegment;
        char in; //input from filesystem for reading in the bitmap
        int i;
        char braek = 0;
        for(i = 0; i < bitmapsize_bytes; i++)
        {
            fread(&in,1,1,fs);
            //in_int = (int) in[0];
            if(in != 255) //something in this 8 block segment is available
            {
                int j;
                for(j = 0; j < 8; j++)
                {
                    if( !( in & (1 << j) ) ) //available
                    {
                        openblock = i*8 + j; //now we can find the OB by dividing by 8 and taking OB mod 8
                        value_opensegment = in;
                        this_addr = openblock*BLOCKSIZE + bitmapsize_bytes;
                        braek = 1;
                        break;
                    }
                }
            }
            if(braek) {break;}
        }
        if(openblock == -1) { printf("An open block was not found. Your filesystem could be full. Or there could be an error... If that's the case, ignore it.\n"); }
    }
    else
    {
        this_addr = succ_addr;
    }

    //get epoch
    fseek(fs,bitmapsize_bytes+10,SEEK_SET);
    int epoch;
    fread(&epoch,4,1,fs);

    //set bitmap to used
    fseek(fs, openblock/8, SEEK_SET);
    char x = (value_opensegment | 1 << (openblock%8));
    fwrite( &x, 1, 1, fs);
    //name - skip this section
    fseek(fs, bitmapsize_bytes + (openblock * BLOCKSIZE)+8, SEEK_SET);
    //set parent address
    fwrite(&cwd, sizeof(short), 1, fs);
    //set timestamp
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int z = tv.tv_sec - epoch;
    fwrite(&z,4,1,fs);
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
    printf("\n\n POS = %i\n\n",ftell(fs));

    //write to the file
    if(strlen(rest) <= BLOCKSIZE-20) //it will all fit in this block
    {
        fwrite(rest,1,strlen(rest),fs); //write the length of rest into the file 1 char at a time
    }
    else
    {
        char *first_half = malloc( (BLOCKSIZE-20) * sizeof(char) +1 ); //leave room for the null char
        strncpy(first_half,rest,BLOCKSIZE-20);
        first_half[BLOCKSIZE-20]='\0'; //if rest is less than BLOCKSIZE-20 there will be two null chars and this one will be ignored
        int k = 0;
        for(k=0;k<BLOCKSIZE-20;k++)
        {
            rest++;
        }
        fwrite(first_half,1,strlen(first_half),fs);

        //fix parent's successor addr
        fseek(fs,cwd+16,SEEK_SET);
        short this_addr = bitmapsize_bytes + (openblock*BLOCKSIZE);
        fwrite(&this_addr,2,1,fs);
        printf("Set succ_addr to %i at %i\n",this_addr,cwd+16);

        successor_file(fsfilename,rest,bitmapsize_bytes+(openblock*BLOCKSIZE)); //call recursively
    }

    fclose(fs);
}
