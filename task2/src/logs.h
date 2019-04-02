#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "banking.h"
#include "common.h"
#include "pa2345.h"
#include "ipc.h"

#ifndef __IFMO_DISTRIBUTED_CLASS_LOGS__H
#define __IFMO_DISTRIBUTED_CLASS_LOGS__H

// strings for pipes.log
static const char *const pipe_opend_msg =
    "pid - %d open pipe from %d to %d fd - %d\n";
static const char *const pipe_closed_msg =
    "pid - %d close pipe from %d to %d fd - %d\n";
static const int MAX_LOG_LENGTH = 256;

typedef enum {
  STARTED = 0,
  DONE = 1,
  TRANSFER_IN = 2,
  TRANSFER_OUT = 3,
  RECEIVED_ALL_DONE = 4,
  RECEIVED_ALL_STARTED = 5,
} event_log_type_t;

typedef enum {
  OPEN = 0,
  CLOSE = 1,
} pipe_log_type_t;

void open_log_files();

void close_log_files();

void log_event(event_log_type_t type, local_id l_id, local_id to_id, balance_t s_balance);

void log_pipe(pipe_log_type_t type, local_id current_id, local_id from, local_id to, int descriptor);

#endif
