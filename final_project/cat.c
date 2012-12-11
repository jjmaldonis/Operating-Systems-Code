void cat_succ(char *fsfilename, short file_addr, short size, short parent_addr);

void cat(char *fsfilename, char *rest, short cwd);
void cat(char *fsfilename, char *rest, short cwd)
{
    FILE *fs = fopen(fsfilename,"r+");

    if(!rest)
    {
        printf("Error: No output file specified\n");
    }
    else
    {
        //find file if it exists
        short file_addr;
        file_addr = find_child(fsfilename,rest,cwd);

        short type;
        if(file_addr != 0)
        {
            fseek(fs,file_addr+18,SEEK_SET);
            fread(&type,1,1,fs);
        }
        if(file_addr != 0 && type == 'f') //file exists so output its contents
        {
            //get size
            fseek(fs,file_addr+14,SEEK_SET);
            short size;
            fread(&size,2,1,fs);
            //get succ_addr
            short succ_addr;
            fread(&succ_addr,2,1,fs);
            //get contents of file
            fseek(fs,2,SEEK_CUR); //go forward 4 bytes to skip the rest of the file info
            char in;
            int k = 0;
            for(k; k < size; k++)
            {
                fread(&in,1,1,fs);
                printf("%c",in);
                if(k >= BLOCKSIZE-20)
                {
                    break;
                }
            }
            if(succ_addr != 0)
            {
                cat_succ(fsfilename, succ_addr, size-(BLOCKSIZE-20), file_addr);
            }
            printf("\n");

        }
        else
        {
            printf("The file you specified does not exist.\n");
        }
    }
    fclose(fs);
}

void cat_succ(char *fsfilename, short file_addr, short size, short parent_addr)
{
    FILE *fs = fopen(fsfilename,"r+");

    //get succ_addr
    fseek(fs,file_addr+16,SEEK_SET);
    short succ_addr;
    fread(&succ_addr,2,1,fs);
    //get contents of file
    fseek(fs,2,SEEK_CUR);
    //fseek(fs,file_addr+18,SEEK_SET); //go to the correct spot
    char in;
    int k = 0;
    for(k; k < size; k++)
    {
        fread(&in,1,1,fs);
        printf("%c",in);
        if(k >= BLOCKSIZE-20)
            break;
    }
    if(succ_addr != 0)
    {
        cat_succ(fsfilename, succ_addr, size-(BLOCKSIZE-20), file_addr);
    }
    fclose(fs);
}
