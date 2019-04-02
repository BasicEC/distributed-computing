#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "banking.h"
#include "self.h"
#include "ipc.h"

void transfer(void * proc_info, local_id src, local_id dst, balance_t amount) {
  
  TransferOrder to;
  to.s_src = src;
  to.s_dst = dst;
  to.s_amount = amount;
  
  Message m;
  Message m_received;
  m.s_header.s_magic = MESSAGE_MAGIC;
  m.s_header.s_type = TRANSFER;
  m.s_header.s_local_time = get_physical_time();
  m.s_header.s_payload_len = sizeof(TransferOrder);

  memcpy(m.s_payload, &to, sizeof(TransferOrder));
  send(proc_info, src, &m);
  receive(proc_info, dst, &m_received);
}
