#include "edimcoder.h"

void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    assert(buf_cap(buf) <= (SIZE_MAX - 1) / 2);
    size_t new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf)) / elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap * elem_size;
    
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = xrealloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = xmalloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}


void *xcalloc(size_t num_elems, size_t elem_size) {
    void *ptr = calloc(num_elems, elem_size);
    if (!ptr) {
        perror("xcalloc failed");
        exit(1);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        perror("xrealloc failed");
        exit(1);
    }
    return ptr;
}

void *xmalloc(size_t num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        perror("xmalloc failed");
        exit(1);
    }
    return ptr;
}


void fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("FATAL: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    exit(1);
}
