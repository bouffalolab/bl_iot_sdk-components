#ifndef _VIRT_NET_SPI_H_
#define _VIRT_NET_SPI_H_

#include "virt_net.h"
#include "tp_spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef VIRT_NET_TASK_PRI 
#define VIRT_NET_TASK_PRI (19)
#endif

virt_net_t virt_net_spi_create(tp_spi_config_t *spi_config);

#ifdef __cplusplus
}
#endif

#endif
