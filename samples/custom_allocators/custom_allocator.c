// Simple INI Reader Samples - Custom Allocators
// Author: Sebastian Jones
//
// A trivial and basically useless custom allocator designed to show
// how to use your custom allocator with SIR.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct AllocatorContext
{
    char *mem;
    size_t size, pos;
}
AllocatorContext;

void *custom_alloc(AllocatorContext *ctx, size_t size);
void *custom_realloc(AllocatorContext *ctx, void *mem, size_t size);
void custom_free(AllocatorContext *ctx, void *mem);

#define SIR_MALLOC(ctx, size)        custom_alloc(ctx, size)
#define SIR_FREE(ctx, mem)           custom_free(ctx, mem)
#define SIR_REALLOC(ctx, mem, size)  custom_realloc(ctx, mem, size)

#define SIMPLE_INI_READER_IMPLEMENTATION

#include "../../simple_ini_reader.h"

void *custom_alloc(AllocatorContext *ctx, size_t size)
{
    assert(ctx);
    assert(size > 0);
    assert(size < ctx->size - ctx->pos);

    void *mem = ctx->mem + ctx->pos;

    ctx->pos += size;

    printf("custom_alloc %3i bytes\n", (int)size);

    return mem;
}

void *custom_realloc(AllocatorContext *ctx, void *mem, size_t size)
{
    void *new_mem = custom_alloc(ctx, size);

    memcpy(new_mem, mem, size);

    return new_mem;
}

void custom_free(AllocatorContext *ctx, void *mem)
{
    return;
}

int main(int argc, char **argv)
{
    AllocatorContext ctx;
    ctx.size = 4000000;
    ctx.pos  = 0;
    ctx.mem  = malloc(ctx.size);

    SirIni ini = sir_load_from_file("custom_allocator.ini", 0, &ctx);

    if (!ini) // Couldn't malloc the ini struct
        return 1;

    if (sir_has_error(ini))
    {
        printf("%s\n", ini->error);
        return 1;
    }

    printf("total bytes used: %i/%i bytes\n", (int)ctx.pos, (int)ctx.size);

    printf("\n");
    
    sir_free_ini(ini);

    free(ctx.mem);

    return 0;
}
