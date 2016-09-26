#!/bin/bash
#

Files=(
	mcs-lock/mcs-lock.h mcs-lock/mcs-lock.cc
	linuxrwlocks/linuxrwlocks.h linuxrwlocks/linuxrwlocks.c
	concurrent-hashmap/hashmap.h concurrent-hashmap/hashmap.cc
	read-copy-update/rcu.h read-copy-update/rcu.cc
	seqlock/seqlock.h seqlock/seqlock.cc
	ticket-lock/lock.h ticket-lock/lock.c
	spsc-bugfix/eventcount.h spsc-bugfix/queue.h spsc-bugfix/queue.cc
	mpmc-queue/mpmc-queue.h mpmc-queue/mpmc-queue.cc
	chase-lev-deque-bugfix/deque.h chase-lev-deque-bugfix/deque.c
	ms-queue/queue.h ms-queue/queue.c
)

MainFiles=(
	linuxrwlocks/main.c
	concurrent-hashmap/main.cc
	read-copy-update/main.cc
	seqlock/main.cc
	ticket-lock/main.cc
	spsc-bugfix/main.cc
	mcs-lock/main.cc
	mpmc-queue/main.cc
	chase-lev-deque-bugfix/main.c
	ms-queue/main.c
)

echo "cloc ${Files[*]}"

cloc ${Files[*]} ${MainFiles[*]}
