#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>
#include "ipc.h"
#include "common.h"
#include "pa1.h"
#include "self.h"
#include "banking.h"

int PROCESS_COUNT;
int CONNECTIONS_COUNT;

balance_t *parse_arguments(char **args)
{
    void *ptr = NULL;
    if (strcmp(args[1], "-p") == 0 || strcmp(args[1], "-P") == 0)
    {
        PROCESS_COUNT = (short)strtol(args[2], ptr, 10) + 1;
        CONNECTIONS_COUNT = PROCESS_COUNT;
    }

    balance_t *balances = (balance_t *)malloc(sizeof(balance_t) * (PROCESS_COUNT - 1));
    
    for (int i = 0; i < PROCESS_COUNT - 1; i++)
    {
        balances[i] = (balance_t)atoi(args[i + 3]);
    }
}

int main(int argc, char **argv)
{
    balance_t* balances = parse_arguments(argv);
    
    return 0;
}