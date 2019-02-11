
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "mem.h"
#include "object.h"

void *object_alloc(const char *name, void *array, unsigned elems, unsigned sz)
{
    unsigned idx = 0;
    while (idx < elems &&
            ((struct object *)((uint8_t *)array + idx * sz))->valid)
        ++idx;
    if (idx == elems) {
        WARN("%s: ERROR: failed to alloc object %s: out of mem\r\n", __func__, name);
        return NULL;
    }
    INFO("OBJECT: alloced obj %s of sz %u\r\n", name, sz);
    struct object *obj = (struct object *)((uint8_t *)array + idx * sz);
    bzero(obj, sz);
    obj->valid = 1;
    assert(idx <= ~(typeof(obj->index))0);
    obj->index = idx;
    return obj;
}

void object_free(void *obj, unsigned sz)
{
    bzero(obj, sz);
}
