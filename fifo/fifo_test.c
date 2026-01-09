#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Example user-defined struct */
typedef struct {
    int  id;
    char name[32];
} item_t;

static item_t *item_new(int id, const char *name) {
    item_t *p = (item_t *)malloc(sizeof(*p));
    assert(p);
    p->id = id;
    snprintf(p->name, sizeof(p->name), "%s", name ? name : "");
    return p;
}

static void item_free(void *p) {
    free(p);
}

static void dump_fifo(const fifo_t *q, const char *tag) {
    printf("\n[%s] cap=%zu size=%zu empty=%d full=%d\n",
           tag,
           fifo_capacity(q),
           fifo_size(q),
           (int)fifo_is_empty(q),
           (int)fifo_is_full(q));

    for (size_t i = 0; i < fifo_size(q); i++) {
        item_t *it = (item_t *)fifo_peek(q, i);
        printf("  idx %zu: id=%d name=%s\n", i, it->id, it->name);
    }
}

int main(void) {
    fifo_t *q = fifo_create(3);
    assert(q);
    assert(fifo_is_empty(q));
    assert(!fifo_is_full(q));
    assert(fifo_size(q) == 0);

    /* Push to tail */
    assert(fifo_push_tail(q, item_new(1, "one")));
    assert(fifo_push_tail(q, item_new(2, "two")));
    assert(fifo_size(q) == 2);

    /* Peek checks (0 = oldest) */
    item_t *a = (item_t *)fifo_peek(q, 0);
    item_t *b = (item_t *)fifo_peek(q, 1);
    assert(a && a->id == 1);
    assert(b && b->id == 2);

    /* Push to head (becomes oldest) */
    assert(fifo_push_head(q, item_new(0, "zero")));
    assert(fifo_is_full(q));
    assert(fifo_size(q) == 3);

    dump_fifo(q, "after push_tail(1,2) and push_head(0)");

    /* Pull newest from tail */
    item_t *t = (item_t *)fifo_pull_tail(q);
    assert(t && t->id == 2);
    item_free(t);
    assert(fifo_size(q) == 2);
    assert(!fifo_is_full(q));

    /* Now oldest should be 0, newest should be 1 */
    assert(((item_t *)fifo_peek_head(q))->id == 0);
    assert(((item_t *)fifo_peek_tail(q))->id == 1);

    /* Push another to tail (wrap-around exercise) */
    assert(fifo_push_tail(q, item_new(3, "three")));
    assert(fifo_is_full(q));
    dump_fifo(q, "after pulling tail and pushing tail(3)");

    /* Pull oldest from head */
    item_t *h = (item_t *)fifo_pull_head(q);
    assert(h && h->id == 0);
    item_free(h);

    /* Pull oldest again */
    h = (item_t *)fifo_pull_head(q);
    assert(h && h->id == 1);
    item_free(h);

    /* Remaining should be id=3 */
    assert(fifo_size(q) == 1);
    assert(((item_t *)fifo_peek(q, 0))->id == 3);

    /* Clear (free remaining items) */
    fifo_clear(q, item_free);
    assert(fifo_is_empty(q));

    /* Validate pull from empty */
    assert(fifo_pull_head(q) == NULL);
    assert(fifo_pull_tail(q) == NULL);

    /* Fill again, then destroy with free_fn */
    assert(fifo_push_tail(q, item_new(10, "ten")));
    assert(fifo_push_tail(q, item_new(11, "eleven")));
    dump_fifo(q, "before destroy");

    fifo_destroy(q, item_free);
    printf("\nAll tests passed.\n");
    return 0;
}

