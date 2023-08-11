#ifndef __OPENTHREAD_BR_H__
#define __OPENTHREAD_BR_H__

#include "otbr_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*statChangedCallback_t)(otChangedFlags);

void otbr_netif_process(otInstance *aInstance);

struct netif * otbr_getBackboneNetif(void);
struct netif * otbr_getThreadNetif(void);

extern otInstance *otrGetInstance();

void otbr_instance_init(void * aBackboneNetif);
void otbr_instance_routing_init(void);
void otbrInstance_addStateChangedCallback(statChangedCallback_t callback);

void otbr_netif_init(void);
err_t otbr_netif_output6(uint8_t *aBuffer, uint32_t aLength);

#ifdef __cplusplus
}
#endif

#endif /* __OPENTHREAD_BR_H__ */
