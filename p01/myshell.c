#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include <readline/readline.h>
#include <readline/history.h>

int main()
{
    char* user = getenv("USER"); //set user to the username
    char cwd[1024]; getcwd(cwd,sizeof(cwd)); //set cwd
    
    char *path[128];
    char *command;
    int status, childpid;

    while(1){
        if(cwd != NULL)
        {
            printf("%s:%s > ", user, cwd);
        }

        command = readline(NULL);

        //parse the line into the correct format for 'path'. this puts a new argument in each spot in the path array, beginning with "command". The command variable is now NULL
        int i = 0;
        while( (path[i] = strsep(&command, " ")) != NULL)
        {
            i++;
        }

        //now we are set to run the command
        if( strcmp(path[0],"exit") == 0 || strcmp(path[0],"q") == 0 ) //quit if 'q' or 'exit' is input
        {
            return 0;
        }

        if (strcmp(path[0],"cd") == 0) //if the command is cd, change the directory
        {
            if(path[1] == NULL)
            {
                chdir("/home"); //change dir to home
            }
            else
            {
                chdir(path[1]); //change dir
            }
            getcwd(cwd,sizeof(cwd)); //reset cwd
        }
        else //if its not cd, execute a child process
        {
            if( (childpid = fork()) != 0) //if we are in the parent
            {
                waitpid(childpid, &status, 0); //wait for the child process to finish, store the status of the attempt into status
            }
            else //if we are in the child
            {
                execvp(path[0],path);
                printf("Command Not Recognized. Terminating this child process and continuing on. Try another command.\n");
                return 0;
            }
        }

    }
    return 0;
}
