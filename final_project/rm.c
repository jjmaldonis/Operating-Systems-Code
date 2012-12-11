void rm_succ(char *fsfilename, short succ_addr);

void rm(char *fsfilename, char *rest, short cwd);
void rm(char *fsfilename, char *rest, short cwd)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;

    //try to find the directory specified
    short child_addr;
    child_addr = find_child(fsfilename, rest, cwd);

    //now if we found the correct child it's address will be stored in child_addr, otherwise child_addr will be 0
    if(child_addr != 0) //if we fould the child
    {
        //decrement size of cwd by 2
        fseek(fs,cwd+14,SEEK_SET);
        short size;
        fread(&size, 2, 1, fs);
        size = size - 2;
        fseek(fs,cwd+14,SEEK_SET);
        fwrite(&size,2,1,fs);

        //get child size (because if its a directory it has to be empty)
        fseek(fs,child_addr+14,SEEK_SET);
        short child_size;
        fread(&child_size,2,1,fs);
        //get child's successor block if one exists
        short succ_addr;
        fread(&succ_addr,2,1,fs);
        //get child type
        char child_type;
        fread(&child_type,1,1,fs);

        //remove it if we should
        if( (child_size == 0 && child_type == 'd') || child_type == 'f' )
        {
            //delete the addr from the cwd
            fseek(fs,cwd+20,SEEK_SET);
            short temp_child_addr; //this is what I am going to use for iterating to find the correct spot
            fread(&temp_child_addr,2,1,fs); //read in the first one
            int i = 1;
            while(child_addr != temp_child_addr) //iterate i until we find the right one
            {
                fread(&temp_child_addr,2,1,fs);
                i++;
            }
            //i will now be 1 too far
            fseek(fs,cwd+20+(2*(i-1)),SEEK_SET);
            short zero = 0;
            fwrite(&zero,2,1,fs);

            //delete the file/directory - this is actually unneccessary but its good to do anyways
            char zeros[BLOCKSIZE];
            memset(zeros, 0, BLOCKSIZE); //make sure zeros is blank
            fseek(fs,child_addr,SEEK_SET);
            fwrite(zeros,1,BLOCKSIZE,fs);
            //set bitmap to un-used
            char bit = (child_addr-bitmapsize_bytes)/BLOCKSIZE;
            fseek(fs, bit/8, SEEK_SET); //navigate to the correct bitmap place
            char bitmap_bit;
            fread(&bitmap_bit,1,1,fs);
            fseek(fs,bit/8,SEEK_SET);
            char x = (bitmap_bit ^ (1 << (bit%8)) );
            fwrite( &x, 1, 1, fs);
            if(succ_addr != 0) //there is a block with more data in it
            {
                fclose(fs);
                //remove that block
                rm_succ(fsfilename,succ_addr);
                FILE *fs = fopen(fsfilename,"r+");
            }
        }
        else
        {
            printf("The directory you specified has children, you must delete them first.\n");
        }
    }
    else
    {
        printf("The file/directory specified does not exist.\n");
    }
    fclose(fs);
}

void rm_succ(char *fsfilename, short addr)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;

    //get successor block if one exists
    fseek(fs,addr+16,SEEK_SET);
    short succ_addr;
    fread(&succ_addr,2,1,fs);

    //remove this block
    //delete the file/directory - this is actually unneccessary but its good to do anyways
    char zeros[BLOCKSIZE];
    memset(zeros, 0, BLOCKSIZE); //make sure zeros is blank
    fseek(fs,addr,SEEK_SET);
    fwrite(zeros,1,BLOCKSIZE,fs);
    //set bitmap to un-used
    char bit = (addr-bitmapsize_bytes)/BLOCKSIZE;
    fseek(fs, bit/8, SEEK_SET); //navigate to the correct bitmap place
    char bitmap_bit;
    fread(&bitmap_bit,1,1,fs);
    fseek(fs,bit/8,SEEK_SET);
    char x = (bitmap_bit ^ (1 << (bit%8)) );
    fwrite( &x, 1, 1, fs);
    if(succ_addr != 0) //there is a block with more data in it
    {
        fclose(fs);
        //remove that block
        rm_succ(fsfilename,succ_addr);
        FILE *fs = fopen(fsfilename,"r+");
    }
    fclose(fs);
}
