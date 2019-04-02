#include "logs.h"

int pipesLog;
int eventsLog;

void open_log_files()
{
  pipesLog = open(pipes_log, O_APPEND | O_WRONLY | O_CREAT | O_TRUNC, 0666);
  eventsLog = open(events_log, O_APPEND | O_WRONLY | O_CREAT | O_TRUNC, 0666);
}

void close_log_files()
{
  close(pipesLog);
  close(eventsLog);
}

void log_event(event_log_type_t type, local_id l_id, local_id to_id, balance_t s_balance)
{
  char buf[MAX_LOG_LENGTH];

  switch (type)
  {
  case STARTED:
    sprintf(buf, log_started_fmt, get_physical_time(), l_id, getpid(), getppid(), s_balance);
    break;
  case DONE:
    sprintf(buf, log_done_fmt, get_physical_time(), l_id, s_balance);
    break;
  case TRANSFER_IN:
    sprintf(buf, log_transfer_in_fmt, get_physical_time(), l_id, s_balance, to_id);
    break;
  case TRANSFER_OUT:
    sprintf(buf, log_transfer_out_fmt, get_physical_time(), l_id, s_balance, to_id);
    break;
  case RECEIVED_ALL_DONE:
    sprintf(buf, log_received_all_done_fmt, get_physical_time(), l_id);
    break;
  case RECEIVED_ALL_STARTED:
    sprintf(buf, log_received_all_started_fmt, get_physical_time(), l_id);
    break;
  default:
    break;
  }

  printf("%s", buf);
  write(eventsLog, buf, strlen(buf));
}

void log_pipe(pipe_log_type_t type, local_id current_id, local_id from, local_id to, int descriptor)
{
  char buf[MAX_LOG_LENGTH];

  switch (type)
  {
  case OPEN:
    sprintf(buf, pipe_opend_msg, current_id, from, to, descriptor);
    break;
  case CLOSE:
    sprintf(buf, pipe_closed_msg, current_id, from, to, descriptor);
    break;
  }
  write(pipesLog, buf, strlen(buf));
}
