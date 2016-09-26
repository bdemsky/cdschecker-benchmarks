#ifndef _LOCK_H
#define _LOCK_H

#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>

typedef struct TicketLock {
	atomic_uint ticket;
	atomic_uint turn;
} TicketLock;

void initTicketLock(TicketLock* l);

void lock(TicketLock *l);

void unlock(TicketLock *l);

#endif
