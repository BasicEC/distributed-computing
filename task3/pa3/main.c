#define _GNU_SOURCE

#include "main.h"
#include "banking.h"
#include "connections.h"
#include "util.h"

pid_t parentPid;
FILE* pLogFile;

int processID;

int* pBalance;
int* pLockedBalance;
BalanceHistory* pBalanceHistory;


void copy(void* from, void* to, size_t count) {
	for (int i = 0; i < count; i++) {
		((char*)to)[i] = ((char*)from)[i];
	}
}

void zero(void* from, size_t count) {
	for (int i = 0; i < count; i++) {
		((char*)from)[i] = 0;
	}
}

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount)
{
	Message msg;
	DataInfo info;
	info.senderId = PARENT_ID;

	TransferOrder order;
	order.s_src = src;
	order.s_dst = dst;
	order.s_amount = amount;

	register_event();
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_type = TRANSFER;
	msg.s_header.s_payload_len = sizeof(TransferOrder);
	copy(&order, msg.s_payload, msg.s_header.s_payload_len);

	send(&info, src, &msg);

	// wait ACK message
	receive(&info, dst, &msg);
}


void store_history(timestamp_t time, balance_t amount, balance_t pending) {
	// find row with current time stamp
	int findIdx = -1;
	for (int idx = 0; idx < pBalanceHistory->s_history_len; idx++) {
		if (pBalanceHistory->s_history[idx].s_time == time) {
			findIdx = idx;
			break;
		}
	}

	BalanceState* pBalanceState;

	if (findIdx < 0 || findIdx >= pBalanceHistory->s_history_len) {
		if (pBalanceHistory->s_history_len > 0) {
			pBalanceState = &pBalanceHistory->s_history[pBalanceHistory->s_history_len - 1];
			if (pBalanceState->s_time < time - 1) {
				for (timestamp_t t = pBalanceHistory->s_history[pBalanceHistory->s_history_len - 1].s_time + 1; t < time; t++) {
					store_history(t, pBalanceState->s_balance, pBalanceState->s_balance_pending_in);
				}
			}
		}

		pBalanceState = &pBalanceHistory->s_history[pBalanceHistory->s_history_len];
		pBalanceHistory->s_history_len++;
	} else {
		pBalanceState = &pBalanceHistory->s_history[findIdx];
	}

	pBalanceState->s_balance = amount;
	pBalanceState->s_balance_pending_in = pending;
	pBalanceState->s_time = time;
}

/*
 * Child workflow
 */

int system_done(pid_t pid, int selfId) {
	register_event();
	// sync
	Message msg;
	sprintf(msg.s_payload, log_done_fmt, get_time(), selfId, pBalance[selfId - 1]);

	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = strlen(msg.s_payload) + 1;
	msg.s_header.s_type = DONE;

	DataInfo info;
	info.senderId = selfId;

	send_multicast(&info, &msg);

	for (int id = 0; id < get_childCount() + 1; id++) {
		if (id != PARENT_ID && id != selfId) {
			receive(&info, id, &msg);
		}
	}

	store_history(get_time(), pBalance[selfId], 0);

	// send balance info
	register_event();
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_type = BALANCE_HISTORY;
	msg.s_header.s_payload_len =
			sizeof(pBalanceHistory->s_history_len) +
			sizeof(pBalanceHistory->s_id) +
			sizeof(BalanceState) * (pBalanceHistory->s_history_len);

	copy(pBalanceHistory, msg.s_payload, msg.s_header.s_payload_len);
	send(&info, PARENT_ID, &msg);

	free(pBalanceHistory);

	// sync ended
	fprintf(pLogFile, log_received_all_done_fmt, get_time(), selfId);
	fflush(pLogFile);

	return 0;
}

int system_work(pid_t pid, int selfId) {
	// some work
	char isWorked = 1;

	while (isWorked) {
		Message msg;
		DataInfo info;

		zero(&msg, sizeof(Message));
		zero(&info, sizeof(DataInfo));

		info.senderId = selfId;
		//store_history(get_time(), pBalance[selfId], pLockedBalance[selfId]);
		if (receive_any(&info, &msg) == 0) {
			switch (msg.s_header.s_type) {
			case TRANSFER: {
					TransferOrder order;
					zero(&order, sizeof(TransferOrder));

					copy(msg.s_payload, &order, msg.s_header.s_payload_len);
					int amount = order.s_amount;

					if (info.receiveId == PARENT_ID) {
						pBalance[selfId] -= amount;
						pLockedBalance[selfId] += amount;
						store_history(get_time(), pBalance[selfId], amount);
						store_history(get_time() + 2, pBalance[selfId], 0);

						register_event();
						msg.s_header.s_local_time = get_time();
						msg.s_header.s_magic = MESSAGE_MAGIC;
						send(&info, order.s_dst, &msg);

						fprintf(pLogFile, log_transfer_out_fmt, get_time(), order.s_src, amount, order.s_dst);
						fflush(pLogFile);

						//receive(&info, order.s_dst, &msg);
						//pLockedBalance[selfId] -= amount;
						//store_history(get_time() - 1, pBalance[selfId], pLockedBalance[selfId]);
					} else {
						fprintf(pLogFile, log_transfer_in_fmt, get_time(), selfId, amount, info.receiveId);
						fflush(pLogFile);

						pBalance[selfId] += amount;
						store_history(get_time(), pBalance[selfId], 0);

						register_event();
						msg.s_header.s_local_time = get_time();
						msg.s_header.s_magic = MESSAGE_MAGIC;
						msg.s_header.s_type = ACK;
						msg.s_header.s_payload_len = 0;
						send(&info, PARENT_ID, &msg);
						//register_event();
						//msg.s_header.s_local_time = get_time();
						//send(&info, order.s_src, &msg);
					}
				} break;
			case STOP: {
					isWorked = 0;
					store_history(get_time(), pBalance[selfId], 0);
				} break;
			default:
				break;
			}

		}
	}

	// work is done
	fprintf(pLogFile, log_done_fmt, get_time(), selfId, pBalance[selfId]);
	fflush(pLogFile);

	return system_done(pid, selfId);
}

int system_started(pid_t pid, int selfId) {
	register_event();
	// sync
	fprintf(pLogFile, log_started_fmt, get_time(), selfId, pid, parentPid, pBalance[selfId]);
	fflush(pLogFile);

	Message msg;
	sprintf(msg.s_payload, log_started_fmt, get_time(), selfId, pid, parentPid, pBalance[selfId]);
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = strlen(msg.s_payload) + 1;
	msg.s_header.s_type = STARTED;

	DataInfo info;
	info.senderId = selfId;

	send_multicast(&info, &msg);

	for (int id = 0; id < get_childCount() + 1; id++) {
		if (id != PARENT_ID && id != selfId) {
			receive(&info, id, &msg);
		}
	}

	pBalanceHistory = (BalanceHistory*)malloc(sizeof(BalanceHistory));
	pBalanceHistory->s_id = selfId;
	pBalanceHistory->s_history_len = 0;
	store_history(0, pBalance[selfId], 0);

	// sync complete
	fprintf(pLogFile, log_received_all_started_fmt, get_time(), selfId);
	fflush(pLogFile);

	return system_work(pid, selfId);
}




int main(int argc, char **argv) {
	int childCount = atoi(argv[2]);

	if (childCount < 1) {
		return -1;
	}
    printf("kek\n");
	set_childCount(childCount);
	pBalance = (int*)malloc(sizeof(int) * childCount + 1);
	pLockedBalance = (int*)malloc(sizeof(int) * childCount + 1);
	pBalance[0] = 0;
	pLockedBalance[0] = 0;
	for (int idx = 1; idx < childCount + 1; idx++) {
		pBalance[idx] = atoi(argv[idx + 2]);
		pLockedBalance[idx] = 0;
	}

	processID = 0;
	parentPid = getpid();

	pLogFile = fopen(events_log, "w+");
	set_pipefile(fopen(pipes_log, "w+"));

	initPipeLines(childCount + 1);

	pid_t childPid;
	for (int id = 1; id < childCount + 1; id++) {
		childPid = fork();
		if (childPid >= 0) {
			if (childPid == 0) {
				// Child process
				processID = id;
				closeUnusedPipes(id);
				system_started(getpid(), id);
				break;
			} else {
				// Parent process
			}
		} else {
			// Error on fork
		}
	}
	if (childPid != 0) {
		closeUnusedPipes(PARENT_ID);

		DataInfo info;
		Message msg;
		info.senderId = PARENT_ID;

		// Receive all START Messages
		for (int idx = 1; idx <= childCount; idx++) {
			receive(&info, idx, &msg);
		}

		bank_robbery(NULL, childCount);

		// Send STOP messages
		register_event();
		msg.s_header.s_local_time = get_time();
		msg.s_header.s_magic = MESSAGE_MAGIC;
		msg.s_header.s_type = STOP;
		msg.s_header.s_payload_len = 0;

		send_multicast(&info, &msg);

		// Receive all DONE messages;
		for (int idx = 1; idx < childCount + 1; idx++) {
			receive(&info, idx, &msg);
		}

		// Receive Balance History
		AllHistory allHistory;
		allHistory.s_history_len = childCount;
		for (int idx = 1; idx < childCount + 1; idx++) {
			receive(&info, idx, &msg);
			BalanceHistory* pClientHistory = &allHistory.s_history[idx - 1];
			copy(msg.s_payload, pClientHistory, msg.s_header.s_payload_len);
		}

		int status;
		while (wait(&status) > 0);

		print_history(&allHistory);
	}

	freePipeLines();

	free(pBalance);
	free(pLockedBalance);

	fclose(get_pipefile());
	fclose(pLogFile);

	return 0;
}
