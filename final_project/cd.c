short cd(char *fsfilename, char*rest, short cwd);
short cd(char *fsfilename, char*rest, short cwd)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;
    if(!rest) //change directory to root
    {
        //printf("change directory to root\n");
        cwd = bitmapsize_bytes;
    }
    else
    {
        if(!strcmp(rest,"..")) //you want to go up one directory level
        {
            //printf("You are trying to go up one directory level.\n");
            fseek(fs,cwd,SEEK_SET);
            char name[8];
            fread(name,1,8,fs);
            //printf("Upper directory is %s.\n",name);

            fseek(fs,cwd+8,SEEK_SET);
            short parent_addr;
            fread(&parent_addr,sizeof(short),1,fs);
            if(parent_addr != -1)
            {
                cwd = parent_addr;
                //printf("cwd changed to %d\n",cwd);
            }
            else
            {
                printf("You are in the root directory, you can't go up any further, silly!\n");
            }
        }
        else if(strcmp(rest,".") != 0) //you don't want to stay in the same directory and you're not moving up one level, so you want to move to the directory you specified
        {
            //try to find the directory specified - looking in the children of cwd
            short child_addr = find_child(fsfilename,rest,cwd);

            //now if we found the correct child it's address will be stored in child_addr
            char type;
            if(child_addr != 0)
            {
                fseek(fs,child_addr+18,SEEK_SET);
                fread(&type,1,1,fs);
            }
            if(child_addr != 0 && type == 'd' ) //if we fould the child
            {
                cwd = child_addr;
            }
            else
            {
                printf("The directory you requested does not exist.\n");
            }
        }
        else 
        {
            printf("The directory you requested does not exist.\n");
        }
    }
    fclose(fs);
    return cwd;
}

