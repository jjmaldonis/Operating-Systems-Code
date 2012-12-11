short printbm(char *fsfilename);
short printbm(char *fsfilename)
{
    FILE *fs = fopen(fsfilename,"r+");

    int bitmapsize_bytes = FILESYSSIZE/BLOCKSIZE/8;

    fseek(fs,0,SEEK_SET);
    char in;
    fread(&in,1,1,fs);

    int i,j,k;
    for(i = 0; i < bitmapsize_bytes/64; i++)
    {
        for(j = 0; j < 64/8; j++)
        {
            for( k = 0; k < 8; k++)
            {
                printf("%i",( ((int) in) & (1 << k) ) >> k);
            }
            fread(&in,1,1,fs);
        }
        printf("\n");
    }

    fclose(fs);
} 
