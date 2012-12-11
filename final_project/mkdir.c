void mkdir(char *fsfilename, char *rest, short cwd);
void mkdir(char *fsfilename, char *rest, short cwd)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;

    //try to find the directory specified - if we do, print error
    short child_addr;
    child_addr = find_child(fsfilename,rest,cwd);

    //get size of cwd to make sure there is room
    short cwd_size;
    fseek(fs,cwd+14,SEEK_SET);
    fread(&cwd_size,2,1,fs);

    //now if we found a child with the same name it's address will be stored in child_addr, and the cwd size is stored as well
    if(child_addr != 0 ) //if we fould the child
    {
        printf("This directory already exists.\n");
    }
    else if(cwd_size == (BLOCKSIZE-20)/2 )
    {
        printf("The directory is full.\n");
    }
    else
    {
        //find next available block
        int openblock = -1; //a value of -1 means we didn't find an open block
        int value_opensegment;
        char in; //input from filesystem for reading in the bitmap
        char braek = 0;
        fseek(fs, 0, SEEK_SET);
        int i;
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
                        openblock = i*8 + j; //now we can find the open block by dividing by 8 and taking openblock mod 8
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
            printf("An open block was not found. Your filesystem could be full. Or there could be an error... If that's the case, ignore it.\n"); 
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
            //name the directory
            fseek(fs, bitmapsize_bytes + (openblock * BLOCKSIZE), SEEK_SET);
            char *name_buf = malloc(8);
            strcpy(name_buf,rest);
            fwrite(name_buf, 1, 8, fs);
            //set parent address
            fwrite(&cwd, sizeof(short), 1, fs);
            //set timestamp
            struct timeval tv;
            gettimeofday(&tv,NULL);
            int z = tv.tv_sec;
            fwrite(&z,4,1,fs);
            //set size
            short zz = 0;
            fwrite(&zz,2,1,fs);
            //set successor_addr
            fwrite(&zz,2,1,fs);
            //set type
            char type = 'd';
            fwrite(&type,1,1,fs);


            //go to the parent directory and increment the parent's size by 2
            fseek(fs,cwd+14,SEEK_SET);
            short size;
            fread(&size,2,1,fs);
            fseek(fs,cwd+14,SEEK_SET);
            size = size + 2;
            fwrite(&size,2,1,fs);
            //now we need to give it this child
            //FIX - if the dir is full we need to make another block for it
            fseek(fs,4,SEEK_CUR);
            fread(&child_addr,2,1,fs);
            i = 0;
            while(child_addr != 0 && i < (BLOCKSIZE-20)/2)
            {
                fread(&child_addr,2,1,fs);
                i++;
            }
            fseek(fs,cwd+20 + (2*i),SEEK_SET);
            short this_addr = bitmapsize_bytes + (openblock * BLOCKSIZE);
            fwrite(&this_addr, 2,1,fs);
        }
    }
    fclose(fs);
}
