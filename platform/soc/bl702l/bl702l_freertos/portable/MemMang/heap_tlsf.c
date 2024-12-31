#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tlsf.h"
#include "FreeRTOS.h"
#include "bl_irq.h"


#ifndef MAX_HEAP_REGION_NUM
#define MAX_HEAP_REGION_NUM        2
#endif

#ifndef MIN_EVER_FREE_HEAP_ENABLE
#define MIN_EVER_FREE_HEAP_ENABLE  0
#endif


typedef struct BlockInfo {
    size_t xFreeSize;
} BlockInfo_t;


static tlsf_t tlsf;
static pool_t pool[MAX_HEAP_REGION_NUM];
#if( MIN_EVER_FREE_HEAP_ENABLE != 0 )
static size_t xMinimumEverFreeBytesRemaining;
#endif


static void tlsf_pool_walker(void *ptr, size_t size, int used, void *user)
{
    BlockInfo_t *pxBlockInfo = (BlockInfo_t *)user;

    if(!used){
        pxBlockInfo->xFreeSize += size;
    }
}

static size_t tlsf_get_free_size(void)
{
    BlockInfo_t xBlockInfo = {0};

    for(int i=0; i<MAX_HEAP_REGION_NUM; i++){
        if(pool[i]){
            tlsf_walk_pool(pool[i], tlsf_pool_walker, &xBlockInfo);
        }else{
            break;
        }
    }

    return xBlockInfo.xFreeSize;
}


void *pvPortMalloc(size_t xWantedSize)
{
    int flag = bl_irq_save();
    void *pvReturn = tlsf_malloc(tlsf, xWantedSize);
#if( MIN_EVER_FREE_HEAP_ENABLE != 0 )
    size_t xFreeSize = tlsf_get_free_size();
    if(xFreeSize < xMinimumEverFreeBytesRemaining){
        xMinimumEverFreeBytesRemaining = xFreeSize;
    }
#endif
    bl_irq_restore(flag);

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
    if(pvReturn == NULL){
        extern void vApplicationMallocFailedHook(void);
        vApplicationMallocFailedHook();
    }
#endif

    return pvReturn;
}

void *pvPortCalloc(size_t numElements, size_t sizeOfElement)
{
    size_t xWantedSize = numElements * sizeOfElement;
    void *pvReturn = pvPortMalloc(xWantedSize);

    if(pvReturn){
        memset(pvReturn, 0, xWantedSize);
    }

    return pvReturn;
}

void *pvPortRealloc(void *pv, size_t xWantedSize)
{
    int flag = bl_irq_save();
    void *pvReturn = tlsf_realloc(tlsf, pv, xWantedSize);
#if( MIN_EVER_FREE_HEAP_ENABLE != 0 )
    size_t xFreeSize = tlsf_get_free_size();
    if(xFreeSize < xMinimumEverFreeBytesRemaining){
        xMinimumEverFreeBytesRemaining = xFreeSize;
    }
#endif
    bl_irq_restore(flag);

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
    if(pvReturn == NULL){
        extern void vApplicationMallocFailedHook( void );
        vApplicationMallocFailedHook();
    }
#endif

    return pvReturn;
}

void vPortFree(void *pv)
{
    int flag = bl_irq_save();
    tlsf_free(tlsf, pv);
    bl_irq_restore(flag);
}

size_t xPortGetBlockSize(void *pv)
{
    return tlsf_block_size(pv);
}

size_t xPortGetFreeHeapSize(void)
{
    int flag = bl_irq_save();
    size_t xFreeSize = tlsf_get_free_size();
    bl_irq_restore(flag);

    return xFreeSize;
}

#if( MIN_EVER_FREE_HEAP_ENABLE != 0 )
size_t xPortGetMinimumEverFreeHeapSize(void)
{
    return xMinimumEverFreeBytesRemaining;
}
#endif

void vPortDefineHeapRegions(const HeapRegion_t * const pxHeapRegions)
{
    const HeapRegion_t *pxHeapRegion;

    for(int i=0; i<MAX_HEAP_REGION_NUM; i++){
        pxHeapRegion = &pxHeapRegions[i];
        if(pxHeapRegion->pucStartAddress && pxHeapRegion->xSizeInBytes){
            if(i == 0){
                tlsf = tlsf_create_with_pool(pxHeapRegion->pucStartAddress, pxHeapRegion->xSizeInBytes);
                pool[i] = tlsf_get_pool(tlsf);
            }else{
                pool[i] = tlsf_add_pool(tlsf, (char *)pxHeapRegion->pucStartAddress + tlsf_size(), pxHeapRegion->xSizeInBytes - tlsf_size());
            }
        }else{
            break;
        }
    }

#if( MIN_EVER_FREE_HEAP_ENABLE != 0 )
    xMinimumEverFreeBytesRemaining = tlsf_get_free_size();
#endif
}
