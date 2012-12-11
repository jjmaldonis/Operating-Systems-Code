short find_child(char *fsfilename, char *name, short cwd);
short find_child(char *fsfilename, char *name, short cwd)
{
    FILE *fs = fopen(fsfilename,"r+");


    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;

    //try to find the directory specified - looking in the children of cwd
    //FIX - if the dir is full you need to look in its succ_dir as well
    fseek(fs,cwd+20,SEEK_SET); //seek to where the child addresses start
    short child_addr;
    fread(&child_addr,2,1,fs); //read in the first child addr
    char temp_name[8]; temp_name[0] = '\0'; //make a var to test against "name"
    if(child_addr != 0) //if the first child_addr != 0 (it will be 0 if it doesn't exist) then go to that child and read in its name
    {
        fseek(fs,child_addr,SEEK_SET);
        fread(temp_name,1,8,fs);
    }
    int i = 1;
    while(strcmp(temp_name,name) != 0 && i < (BLOCKSIZE-20)/2)
    {
        fseek(fs,cwd+20+(2*i),SEEK_SET);
        fread(&child_addr,2,1,fs);
        if(child_addr != 0)
        {
            fseek(fs,child_addr,SEEK_SET);
            fread(temp_name,1,8,fs);
        }
        i++;
    }

    //now if we found the correct child it's address will be stored in child_addr, otherwise child_addr will be 0 or i will be (BLOCKSIZE-20)/2
    
    if(child_addr != 0 && i != (BLOCKSIZE-20)/2)
    {
        return child_addr;
    }
    else
    {
        return 0;
    }
    fclose(fs);
} 
