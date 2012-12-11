#include <stdio.h>

int main()
{
    FILE *fp1;
    FILE *fp2;
    fp1 = fopen("text.txt","r");
    fp2 = fopen("text.txt","w");
    fseek(fp1, 0, SEEK_SET);

    char word[5];
    fread(word, 1,5,fp1);

    fseek(fp2,5,SEEK_SET);
    fwrite(word,1,5,fp2);

    fclose(fp1);
    fclose(fp2);


    return 0;
}
