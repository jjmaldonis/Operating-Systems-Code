#include <sys/time.h>
#include <stdio.h>

int main()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    printf("time in sec = %i\n",tv.tv_sec);
    printf("sizeof(tv.tv_sec)=%i\n",sizeof(tv.tv_sec));
    return 0;
}
