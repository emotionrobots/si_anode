/*!
 *=====================================================================================================================
 *
 *  @file	fifo.h
 *
 *  @brief	FIFO header
 *
 *=====================================================================================================================
 */
#ifndef __FIFO_H__
#define __FIFO_H__

#include <stddef.h>  // size_t
#include <stdbool.h> // bool

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Implementation notes:
 * - Ring buffer storing void* elements.
 * - head = index of oldest element
 * - size = number of elements currently stored
 * - tail insertion index is (head + size) % capacity
 *
 * Peek index i maps to (head + i) % capacity.
 */
struct fifo {
    void   **buf;
    size_t   cap;
    size_t   head;
    size_t   size;
};

typedef struct fifo fifo_t;


fifo_t *fifo_create(size_t capacity);
void fifo_destroy(fifo_t *q, void (*free_fn)(void *elem));
void fifo_clear(fifo_t *q, void (*free_fn)(void *elem));

size_t fifo_capacity(const fifo_t *q);
size_t fifo_size(const fifo_t *q);
bool   fifo_is_empty(const fifo_t *q);
bool   fifo_is_full(const fifo_t *q);

bool fifo_push_tail(fifo_t *q, void *elem); /* enqueue at tail (newest) */
bool fifo_push_head(fifo_t *q, void *elem); /* insert at head (becomes oldest) */

void *fifo_pull_head(fifo_t *q); /* remove & return oldest */
void *fifo_pull_tail(fifo_t *q); /* remove & return newest */

void *fifo_peek(const fifo_t *q, size_t index);

void *fifo_peek_head(const fifo_t *q);
void *fifo_peek_tail(const fifo_t *q);

#ifdef __cplusplus
}
#endif

#endif /* __FIFO_H__ */

