int main() /* this functions runs for a while and creates a bunch of child processes with fork(); it's going to create 2^forks processes I think */
{
    int forks = 0;
    while(forks <= 0)
    {
        fork();
        forks++;
    }

    int run=0, numRuns=0;
    while(numRuns < 9999999)
    {
        run++;
        if(run == 9999999)
        {
            numRuns++;
        }
    }

    return 0;

}
