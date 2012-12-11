void ls_succ(char *fsfilename, short addr);

void ls(char *fsfilename, short cwd);
void ls(char *fsfilename, short cwd)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;
    printf("%-8s     %-5s       %-1s    %s\n","name","size","type","time created");
    //get epoch
    fseek(fs,bitmapsize_bytes+10,SEEK_SET);
    int epoch;
    fread(&epoch,4,1,fs);
    char *dir_name[8]; dir_name[0]='\0';
    short dir_size;
    short parent_addr;
    int timestamp;
    short success_addr;
    char dirtype;
    fseek(fs,cwd,SEEK_SET);
    fread(&dir_name,1,8,fs);
    fread(&parent_addr,2,1,fs);
    if(parent_addr == -1) //we are in the root
    {
        fread(&timestamp,4,1,fs);
        timestamp = epoch;
    }
    else
    {
        fread(&timestamp,4,1,fs);
    }
    fread(&dir_size,2,1,fs);
    fread(&success_addr,2,1,fs);
    fread(&dirtype,1,1,fs);
    time_t t = timestamp;
    printf("%-8s     %-5i       %-1c       %s",".",dir_size,dirtype,ctime(&t));

    if(parent_addr!=-1)//we are not in the root dir
    {
        char *parent_name[8]; parent_name[0]='\0';
        short parent_size;
        short pparent_addr;
        int parent_time;
        char parent_type;
        short psuccess_addr;
        fseek(fs,parent_addr,SEEK_SET);
        fread(&parent_name,1,8,fs);
        fread(&pparent_addr,2,1,fs);
        if(pparent_addr == -1) //the parent is the root
        {
            fread(&parent_time,4,1,fs);
            parent_time = epoch;
        }
        else
        {
            fread(&parent_time,4,1,fs);
        }
        fread(&parent_size,2,1,fs);
        fread(&psuccess_addr,2,1,fs);
        fread(&parent_type,1,1,fs);
        t = parent_time;
        printf("%-8s     %-5i       %-1c       %s","..",parent_size,parent_type,ctime(&t));
    }

    //get succ_addr for this cwd
    short succ_addr;
    fseek(fs,cwd+16,SEEK_SET);
    fread(&succ_addr,2,1,fs);
    //seek to start of child addresses in cwd
    fseek(fs,2,SEEK_CUR);
    //get first child addr
    short child_addr;
    fread(&child_addr,2,1,fs);
    //make some variables
    char child_name[8];
    int time;
    short size;
    char type;
    int i = 1;
    while(i < (BLOCKSIZE-20)/2)
    {
        if(child_addr != 0)
        {
            fseek(fs, child_addr,SEEK_SET);
            fread(&child_name,1,8,fs);
            fread(&parent_addr,2,1,fs);
            fread(&time,4,1,fs);
            fread(&size,2,1,fs);
            fseek(fs,2,SEEK_CUR); //we don't care if there is a successor for the child
            fread(&type,1,1,fs);
            t = time;
            printf("%-8s     %-5i       %-1c       %s",child_name,size,type,ctime(&t));
        }
        fseek(fs, cwd + 20 +(i*2),SEEK_SET);
        fread(&child_addr,2,1,fs);
        i++;
    }
    if(succ_addr != 0) //there is more to find!
    {
        ls_succ(fsfilename,succ_addr);
    }
    fclose(fs);
}

void ls_succ(char *fsfilename, short addr)
{
    FILE *fs = fopen(fsfilename,"r+");

    //get succ_addr for this cwd
    short succ_addr;
    fseek(fs,addr+16,SEEK_SET);
    fread(&succ_addr,2,1,fs);
    //seek to start of child addresses in this cwd
    fseek(fs,2,SEEK_CUR);
    //get first child addr
    short child_addr;
    fread(&child_addr,2,1,fs);
    //make some variables
    char child_name[8];
    int time;
    short size;
    char type;
    time_t t;
    int i = 1;
    while(i < (BLOCKSIZE-20)/2)
    {
        if(child_addr != 0)
        {
            fseek(fs, child_addr,SEEK_SET);
            fread(&child_name,1,8,fs);
            fseek(fs,2,SEEK_CUR); //skip parent addr
            fread(&time,4,1,fs);
            fread(&size,2,1,fs);
            fseek(fs,2,SEEK_CUR); //we don't care if there is a successor for the child
            fread(&type,1,1,fs);
            t = time;
            printf("%-8s     %-5i       %-1c       %s",child_name,size,type,ctime(&t));
        }
        fseek(fs, addr + 20 +(i*2),SEEK_SET);
        fread(&child_addr,2,1,fs);
        i++;
    }
    fclose(fs);
}
