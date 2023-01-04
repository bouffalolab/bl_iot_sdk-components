#include <assert.h>
#include <stdlib.h>

#include <openthread/platform/memory.h>

void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return calloc(aNum, aSize);
}

void otPlatFree(void *aPtr)
{
    free(aPtr);
}
