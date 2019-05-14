//
// Created by edvanchi on 5/5/19.
//

#ifndef TASK6_PA6_H
#define TASK6_PA6_H

#include "common.h"
#include "ipc.h"

/*
 * <timestamp> process <local id> (pid <PID>, paranet <PID>) has STARTED with balance $<id>
 */
static const char * const log_started_fmt =
        "%d: process %1d (pid %5d, parent %5d) has STARTED\n";

static const char * const log_received_all_started_fmt =
        "%d: process %1d received all STARTED messages\n";

static const char * const log_done_fmt =
        "%d: process %1d has DONE\n";

static const char * const log_received_all_done_fmt =
        "%d: process %1d received all DONE messages\n";

static const char * const log_request_for_fork_fmt =
        "%d: process %1d ask for fork from process %d\n";

static const char * const log_responce_with_fork_fmt =
        "%d: process %1d give fork to process %d\n";

static const char * const log_loop_operation_fmt =
        "process %d is doing %d iteration out of %d\n";
#endif //TASK6_PA6_H
