#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char main()
{

    //char *str[10];
    char str[8] = "root\0\0\0\0";
    char * str2 = malloc (8);
    strcpy(str2,"hello");

    /*str[0] = (char) 'h';
    str[1] = (char) 'e';
    str[2] = (char) 'l';
    str[3] = (char) 'l';
    str[4] = (char) 'o';
    str[5] = (char) '\0';
*/

    //printf("%ls\n",str);
    printf("%s\n",str2);
    printf("%lc\n",str2[1]);
    //str2++; //this works
    str2 = &str2[3]; //this works as well
    printf("%s\n",str2);


    return 0;
}
