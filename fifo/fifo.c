/*!
 *=====================================================================================================================
 *
 *  @file	fifoah.
 *
 *  @brief	FIFO implementation 
 *
 *=====================================================================================================================
 */
#include "fifo.h"
#include <stdlib.h> 


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void fifo_free_all_elements(fifo_t *q, void (*free_fn)(void *)) 
 *
 *  @brief	Free all FIFO elements 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
static 
void fifo_free_all_elements(fifo_t *q, void (*free_fn)(void *)) 
{
    if (!q || !free_fn) return;
    for (size_t i = 0; i < q->size; i++) 
    {
        void *p = q->buf[(q->head + i) % q->cap];
        free_fn(p);
    }
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		fifo_t *fifo_create(size_t capacity) 
 *
 *  @brief	Create FIFO
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
fifo_t *fifo_create(size_t capacity) 
{
    if (capacity == 0) return NULL;

    fifo_t *q = (fifo_t *)calloc(1, sizeof(*q));
    if (!q) return NULL;

    q->buf = (void **)calloc(capacity, sizeof(void *));
    if (!q->buf) 
    {
        free(q);
        return NULL;
    }

    q->cap  = capacity;
    q->head = 0;
    q->size = 0;
    return q;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void fifo_destroy(fifo_t *q, void (*free_fn)(void *elem)) 
 *
 *  @brief	Destroy FIFO
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void fifo_destroy(fifo_t *q, void (*free_fn)(void *elem)) 
{
    if (!q) return;
    fifo_free_all_elements(q, free_fn);
    free(q->buf);
    free(q);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void fifo_clear(fifo_t *q, void (*free_fn)(void *elem)) 
 *
 *  @brief	Clear FIFO
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void fifo_clear(fifo_t *q, void (*free_fn)(void *elem)) 
{
    if (!q) return;
    fifo_free_all_elements(q, free_fn);

    /* Reset indices; buffer content doesn't need to be zeroed for correctness */
    q->head = 0;
    q->size = 0;
}

/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		size_t fifo_capacity(const fifo_t *q) 
 *
 *  @brief	Return FIFO capacity 
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
size_t fifo_capacity(const fifo_t *q) 
{ 
   return q ? q->cap  : 0; 
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		size_t fifo_size(const fifo_t *q)     
 *
 *  @brief	Return FIFO size
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
size_t fifo_size(const fifo_t *q)     
{ 
   return q ? q->size : 0; 
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		bool fifo_is_empty(const fifo_t *q) 
 *
 *  @brief	Return true if FIFO is empty
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool fifo_is_empty(const fifo_t *q) 
{ 
   return !q || q->size == 0; 
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		bool fifo_is_full (const fifo_t *q) 
 *
 *  @brief	Return true if FIFO is full
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool fifo_is_full (const fifo_t *q) 
{ 
   return  q && q->size == q->cap; 
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		bool fifo_push_tail(fifo_t *q, void *elem) 
 *
 *  @brief	Push element into tail (newest)
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool fifo_push_tail(fifo_t *q, void *elem) 
{
   if (!q || fifo_is_full(q)) return false;

   size_t idx = (q->head + q->size) % q->cap;
   q->buf[idx] = elem;
   q->size++;
   return true;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		bool fifo_push_head(fifo_t *q, void *elem) 
 *
 *  @brief	Push element into head (oldest)
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
bool fifo_push_head(fifo_t *q, void *elem) 
{
   if (!q || fifo_is_full(q)) return false;

   /* Move head back by 1 (wrapping) and place element there */
   q->head = (q->head + q->cap - 1) % q->cap;
   q->buf[q->head] = elem;
   q->size++;
   return true;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void *fifo_pull_head(fifo_t *q) 
 *
 *  @brief	Pulling FIFO from head (oldest)
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void *fifo_pull_head(fifo_t *q) 
{
   if (!q || fifo_is_empty(q)) return NULL;

   void *elem = q->buf[q->head];
   /* Optional: q->buf[q->head] = NULL; */
   q->head = (q->head + 1) % q->cap;
   q->size--;
   return elem;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void *fifo_pull_tail(fifo_t *q) 
 *
 *  @brief	Pulling element from tail (newest)
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void *fifo_pull_tail(fifo_t *q) 
{
   if (!q || fifo_is_empty(q)) return NULL;

   size_t tail_idx = (q->head + q->size - 1) % q->cap;
   void *elem = q->buf[tail_idx];
   /* Optional: q->buf[tail_idx] = NULL; */
   q->size--;
   return elem;
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void *fifo_peek(const fifo_t *q, size_t index) 
 *
 *  @brief	Peek at FIFO at index (must be less than q-size
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void *fifo_peek(const fifo_t *q, size_t index) 
{
   if (!q) return NULL;
   if (index >= q->size) return NULL;
   size_t idx = (q->head + index) % q->cap;
   return q->buf[idx];
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void *fifo_peek_head(const fifo_t *q) 
 *
 *  @brief	Peek at head (oldest)
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void *fifo_peek_head(const fifo_t *q) 
{
   return fifo_peek(q, 0);
}


/*!
 *---------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		void *fifo_peek_tail(const fifo_t *q) 
 *
 *  @brief	Peek at tail (newest)
 *
 *---------------------------------------------------------------------------------------------------------------------
 */
void *fifo_peek_tail(const fifo_t *q) 
{
   if (!q || q->size == 0) return NULL;
   return fifo_peek(q, q->size - 1);
}

