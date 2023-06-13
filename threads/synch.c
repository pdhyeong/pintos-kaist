/* This file is derived from source code for the Nachos
   instructional operating system.  The Nachos copyright notice
   is reproduced in full below. */

/* Copyright (c) 1992-1996 The Regents of the University of California.
   All rights reserved.

   Permission to use, copy, modify, and distribute this software
   and its documentation for any purpose, without fee, and
   without written agreement is hereby granted, provided that the
   above copyright notice and the following two paragraphs appear
   in all copies of this software.

   IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO
   ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
   CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE
   AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
   HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
   BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
   PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
   MODIFICATIONS.
   */

#include "threads/synch.h"
#include <stdio.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static struct list ready_list;

bool cmp_priority (const struct list_elem *a_elem, const struct list_elem *b_elem, void *aux);
bool cmp_sem_priority (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);


/* Initializes semaphore SEMA to VALUE.  A semaphore is a
   nonnegative integer along with two atomic operators for
   manipulating it:

   - down or "P": wait for the value to become positive, then
   decrement it.

   - up or "V": increment the value (and wake up one waiting
   thread, if any). */
void
sema_init (struct semaphore *sema, unsigned value) {
	ASSERT (sema != NULL);

	sema->value = value;
	list_init (&sema->waiters);
}

/* Down or "P" operation on a semaphore.  Waits for SEMA's value
   to become positive and then atomically decrements it.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but if it sleeps then the next scheduled
   thread will probably turn interrupts back on. This is
   sema_down function. */
void
sema_down (struct semaphore *sema) {
	enum intr_level old_level;

	ASSERT (sema != NULL);
	ASSERT (!intr_context ());

	old_level = intr_disable ();
	while (sema->value == 0) {
		// list_push_back (&sema->waiters, &thread_current ()->elem);
		list_insert_ordered(&sema->waiters, &thread_current()->elem, cmp_priority, NULL);
		thread_block ();
	}
	sema->value--;
	intr_set_level (old_level);
}

/* Down or "P" operation on a semaphore, but only if the
   semaphore is not already 0.  Returns true if the semaphore is
   decremented, false otherwise.

   This function may be called from an interrupt handler. */
bool
sema_try_down (struct semaphore *sema) {
	enum intr_level old_level;
	bool success;

	ASSERT (sema != NULL);

	old_level = intr_disable ();
	if (sema->value > 0)
	{
		sema->value--;
		success = true;
	}
	else
		success = false;
	intr_set_level (old_level);

	return success;
}

/* Up or "V" operation on a semaphore.  Increments SEMA's value
   and wakes up one thread of those waiting for SEMA, if any.

   This function may be called from an interrupt handler. */

// sema 해제 후 priority preemption 기능 추가
void
sema_up (struct semaphore *sema) {
	enum intr_level old_level;

	ASSERT (sema != NULL);
	old_level = intr_disable ();
	if (!list_empty (&sema->waiters)) {
		list_sort(&sema->waiters, cmp_priority, NULL);
		thread_unblock (list_entry (list_pop_front (&sema->waiters), struct thread, elem));
	}
	sema->value++;
	test_max_priority();
	intr_set_level (old_level);
}

static void sema_test_helper (void *sema_);

/* Self-test for semaphores that makes control "ping-pong"
   between a pair of threads.  Insert calls to printf() to see
   what's going on. */
void
sema_self_test (void) {
	struct semaphore sema[2];
	int i;

	printf ("Testing semaphores...");
	sema_init (&sema[0], 0);
	sema_init (&sema[1], 0);
	thread_create ("sema-test", PRI_DEFAULT, sema_test_helper, &sema);
	for (i = 0; i < 10; i++)
	{
		sema_up (&sema[0]);
		sema_down (&sema[1]);
	}
	printf ("done.\n");
}

/* Thread function used by sema_self_test(). */
static void
sema_test_helper (void *sema_) {
	struct semaphore *sema = sema_;
	int i;

	for (i = 0; i < 10; i++)
	{
		sema_down (&sema[0]);
		sema_up (&sema[1]);
	}
}

/* Initializes LOCK.  A lock can be held by at most a single
   thread at any given time.  Our locks are not "recursive", that
   is, it is an error for the thread currently holding a lock to
   try to acquire that lock.

   A lock is a specialization of a semaphore with an initial
   value of 1.  The difference between a lock and such a
   semaphore is twofold.  First, a semaphore can have a value
   greater than 1, but a lock can only be owned by a single
   thread at a time.  Second, a semaphore does not have an owner,
   meaning that one thread can "down" the semaphore and then
   another one "up" it, but with a lock the same thread must both
   acquire and release it.  When these restrictions prove
   onerous, it's a good sign that a semaphore should be used,
   instead of a lock. */
void
lock_init (struct lock *lock) {
	ASSERT (lock != NULL);

	lock->holder = NULL;
	sema_init (&lock->semaphore, 1);
}

/* priority donation 수행 */
void donate_priority(void) {
	// NOTE: 필요에 따라 수행
	struct thread *curr = thread_current();
	while (curr->wait_on_lock) {
		curr->wait_on_lock->holder->priority = curr->priority;
		curr = curr->wait_on_lock->holder;
	}
}

bool cmp_d_priority (const struct list_elem *a_elem, const struct list_elem *b_elem, void *aux) {
	int a = list_entry (a_elem, struct thread, d_elem)->priority;
	int b = list_entry (b_elem, struct thread, d_elem)->priority;
	return a > b;
}

/* Acquires LOCK, sleeping until it becomes available if
   necessary.  The lock must not already be held by the current
   thread.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */
/* lock을 점유하고 있는 스레드와 요청 하는 스레드의 우선순위를 비교하여
priority donation을 수행하도록 수정 */
// NOTE: lock_acquire를 이해한 대로 최종적으로 로직을 수정.
void
lock_acquire (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (!lock_held_by_current_thread (lock));

	if (lock->holder) {
		thread_current ()->wait_on_lock = lock;
		list_insert_ordered(&lock->holder->list_donation, &thread_current()->d_elem, cmp_d_priority, NULL);
		donate_priority();
	}

	sema_down (&lock->semaphore);
	// 스레드는 sema_down에서 락을 얻을 때 까지 기다리다가, 락을 점유할 수 있는 상황이 되면 탈출하여 아래 줄을 실행함
	thread_current()->wait_on_lock = NULL;
	lock->holder = thread_current();
}

/* Tries to acquires LOCK and returns true if successful or false
   on failure.  The lock must not already be held by the current
   thread.

   This function will not sleep, so it may be called within an
   interrupt handler. */
bool
lock_try_acquire (struct lock *lock) {
	bool success;

	ASSERT (lock != NULL);
	ASSERT (!lock_held_by_current_thread (lock));

	success = sema_try_down (&lock->semaphore);
	if (success)
		lock->holder = thread_current ();
	return success;
}

/* NOTE: 아래 lock_release에서 이 함수를 왜 호출했을까? 
락을 놓아주고, 지금까지 그 락을 기다리던 스레드들에게 받았던 donation들을 뱉어내고,
본인의 본래 우선순위로 돌려놓기 위해 호출했음.*/
void refresh_priority(void) {
	struct thread *curr = thread_current();
	// FIXME: Multiple Donation을 구현한 부분. 헤드를 제외한 다른 스레드의 우선순위가 바뀌어서 헤드가 아닌 뒤에 있는 스레드의 우선순위를
	// donation 받아야 될 경우를 대비해서, donation_list를 정렬한 후, 헤드의 우선순위를 비교하여 가져오던가, for문을 통해 donation_list를 순회하며
	// 가장 큰 우선순위를 가져오고, 그 우선순외와 비교하여 donation을 수행하도록 수정할 것.
	int max_prio = list_entry(list_begin(&curr->list_donation), struct thread, d_elem)->priority;

	curr->priority = curr->pre_priority;
	if (!list_empty(&curr->list_donation)) {
		list_sort(&curr->list_donation, cmp_d_priority, NULL);
		if (curr->pre_priority < max_prio) {
			curr->priority = max_prio;
		}
		else{
			curr->priority = curr->pre_priority;
		}
	}
	else{
		curr->priority = curr->pre_priority;
	}
}

/* 우선순위를 다시 계산 */
// NOTE: 가독성 개선했고, 함수에 대해 이해를 마쳤음. remove_with_lock(struct lock *lock)는 다음의 과정을 수행함
/*
락을 점유하고 있는 스레드를 점유 해제시키고, 홀더를 NULL로 바꿔주기 전에, 
이 락을 기다리는 리스트를 순회하여 '이제는 이 락을 필요로 하지 않는 스레드'를 빼주어 리스트를 갱신시켜 주는 작업임!
기다리던 스레드들이 이제는 더 이상 이 락을 필요로 하지 않아서 wait_on_lock이 해당 락이 아니라 NULL이거나, 다른 락에 wait상태로 바뀌어 있다면,
이 녀석들의 주소를 waiters 리스트에 넣어둔 상태로 내버려 둔다면 락을 필요로 하지도 않는 스레드가 락을 기다리는, 이상한 상황이 야기될 것.
확실하진 않아서 우근이형한테 자문 구하면 좋을듯
*/
void remove_with_lock(struct lock *lock){
	struct thread * curr = thread_current();
	struct list_elem *curr_elem = list_begin(&curr->list_donation);
	while (list_end(&curr->list_donation) != curr_elem) {
		struct thread *curr_thread = list_entry(curr_elem, struct thread, d_elem);
		if (curr_thread->wait_on_lock == lock){
			list_remove(curr_elem);
		}
		curr_elem = list_next(curr_elem);
	}
}

/* Releases LOCK, which must be owned by the current thread. 현재 스레드가 소유하고 있어야 하는 LOCK을 해제합니다.
   This is lock_release function.
   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to release a lock within an interrupt
   handler. */
void
lock_release (struct lock *lock) {
	ASSERT (lock != NULL);
	ASSERT (lock_held_by_current_thread (lock));

	remove_with_lock(lock);
	refresh_priority();
	lock->holder = NULL;
	sema_up (&lock->semaphore);
}

/* Returns true if the current thread holds LOCK, false
   otherwise.  (Note that testing whether some other thread holds
   a lock would be racy.) */
bool
lock_held_by_current_thread (const struct lock *lock) {
	ASSERT (lock != NULL);

	return lock->holder == thread_current ();
}

/* One semaphore in a list. */
struct semaphore_elem {
	struct list_elem elem;              /* List element. */
	struct semaphore semaphore;         /* This semaphore. */
};

/* Initializes condition variable COND.  A condition variable
   allows one piece of code to signal a condition and cooperating
   code to receive the signal and act upon it. */
void
cond_init (struct condition *cond) {
	ASSERT (cond != NULL);

	list_init (&cond->waiters);
}

/* Atomically releases LOCK and waits for COND to be signaled by
   some other piece of code.  After COND is signaled, LOCK is
   reacquired before returning.  LOCK must be held before calling
   this function.

   The monitor implemented by this function is "Mesa" style, not
   "Hoare" style, that is, sending and receiving a signal are not
   an atomic operation.  Thus, typically the caller must recheck
   the condition after the wait completes and, if necessary, wait
   again.

   A given condition variable is associated with only a single
   lock, but one lock may be associated with any number of
   condition variables.  That is, there is a one-to-many mapping
   from locks to condition variables.

   This function may sleep, so it must not be called within an
   interrupt handler.  This function may be called with
   interrupts disabled, but interrupts will be turned back on if
   we need to sleep. */

bool cmp_sem_priority(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED) {
	struct semaphore_elem *a_sema_elem = list_entry(a, struct semaphore_elem, elem);
	struct thread *a_thread = list_entry(list_begin(&(a_sema_elem->semaphore.waiters)), struct thread, elem);
	struct semaphore_elem *b_sema_elem = list_entry(b, struct semaphore_elem, elem);
	struct thread *b_thread = list_entry(list_begin(&(b_sema_elem->semaphore.waiters)), struct thread, elem);

	return a_thread->priority > b_thread->priority;
}

void
cond_wait (struct condition *cond, struct lock *lock) {
	struct semaphore_elem waiter;

	ASSERT (cond != NULL);
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (lock_held_by_current_thread (lock));

	sema_init (&waiter.semaphore, 0);
	// list_push_back (&cond->waiters, &waiter.elem);
	list_insert_ordered(&cond->waiters, &waiter.elem, cmp_sem_priority, NULL);
	lock_release (lock);
	sema_down (&waiter.semaphore);
	lock_acquire (lock);
}

/* If any threads are waiting on COND (protected by LOCK), then
   this function signals one of them to wake up from its wait.
   LOCK must be held before calling this function.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
void
cond_signal (struct condition *cond, struct lock *lock UNUSED) {
	ASSERT (cond != NULL);
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (lock_held_by_current_thread (lock));

	if (!list_empty (&cond->waiters)) {
		list_sort(&cond->waiters, cmp_sem_priority, NULL);
		sema_up (&list_entry (list_pop_front (&cond->waiters), struct semaphore_elem, elem)->semaphore);
	}
}

/* Wakes up all threads, if any, waiting on COND (protected by
   LOCK).  LOCK must be held before calling this function.

   An interrupt handler cannot acquire a lock, so it does not
   make sense to try to signal a condition variable within an
   interrupt handler. */
void
cond_broadcast (struct condition *cond, struct lock *lock) {
	ASSERT (cond != NULL);
	ASSERT (lock != NULL);

	while (!list_empty (&cond->waiters))
		cond_signal (cond, lock);
}