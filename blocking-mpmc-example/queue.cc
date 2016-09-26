#include "queue.h"

// Make them C-callable interfaces

/** @DeclareState: IntList *q;
@Commutativity: enq <-> deq (true)
@Commutativity: deq <-> deq (M1->C_RET!=-1||M2->C_RET!=-1) */

void Queue::enq(int val) {
  Node *n = new Node(val);
  while(true) {
    Node *t = tail.load(acquire);
    Node *next = t->next.load(relaxed);
    if(next) continue;
    if(t->next.CAS(next, n, relaxed)) {
	  /** @OPDefine: true */
      tail.store(n, release);
      return;
    }
  }
}
int Queue::deq() {
  while(true) {
    Node *h = head.load(acquire);
    Node *t = tail.load(acquire);
    Node *next = h->next.load(relaxed);
	/** @OPClearDefine: true */
    if(h == t) return -1;
    if(head.CAS(h, next, release))
	  return next->data;
  }
}

/** @Transition: STATE(q)->push_back(val); */
void enq(Queue *q, int val) {
	q->enq(val);
}

/** @Transition: 
S_RET = STATE(q)->empty() ? -1 : STATE(q)->front();
if (S_RET != -1)
    STATE(q)->pop_front();
@PostCondition:
    return C_RET == -1 ? true : C_RET == S_RET;
@JustifyingPostcondition: if (C_RET == -1)
    return S_RET == -1; */
int deq(Queue *q) {
	return q->deq();
}
