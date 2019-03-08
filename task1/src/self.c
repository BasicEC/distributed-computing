#include "self.h"
#include "self.h"
#include <stdlib.h>

int get_w_pipefd_by_id(self_t *self, local_id dst)
{
    return *(self->pipefds + dst + self->current_pid * self->p_count);
}

int get_r_pipefd_by_id(self_t *self, local_id dst)
{
    return *(self->pipefds + self->current_pid + dst * self->p_count);
}

int *get_all_w_pipefds(self_t *self)
{
    pipefds = (int *)malloc(sizeof(int) * self->p_count);
    for (int i = 0; i < self->p_count; i++)
    {
        *(pipefds + i) = *(self->pipefds + i + self->current_pid * self->p_count);
    }
    return pipefds;
}

int *get_all_r_pipefds(self_t *self)
{
    pipefds = (int *)malloc(sizeof(int) * self->p_count);
    for (int i = 0; i < self->p_count; i++)
    {
        *(pipefds + i) = *(self->pipefds + self->current_pid + i * self->p_count);
    }
    return pipefds;
}