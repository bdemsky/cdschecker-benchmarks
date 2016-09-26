#ifndef _LINUXRWLOCKS_H
#define _LINUXRWLOCKS_H

#include <stdio.h>
#include <threads.h>
#include <stdatomic.h>

#include "librace.h"

#define RW_LOCK_BIAS            0x00100000
#define WRITE_LOCK_CMP          RW_LOCK_BIAS

typedef union {
	atomic_int lock;
} rwlock_t;


int read_can_lock(rwlock_t *lock);

int write_can_lock(rwlock_t *lock);

void read_lock(rwlock_t *rw);

void write_lock(rwlock_t *rw);

int read_trylock(rwlock_t *rw);

int write_trylock(rwlock_t *rw);

void read_unlock(rwlock_t *rw);

void write_unlock(rwlock_t *rw);

#endif
