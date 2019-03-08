#include "self.h"
#include "self.h"
#include <stdlib.h>

int get_w_pipefd_by_id(proc_info_t *self, local_id dst)
{
    return self->connections[dst]->write;
}

int get_r_pipefd_by_id(proc_info_t *self, local_id dst)
{
    return self->connections[dst]->read;
}