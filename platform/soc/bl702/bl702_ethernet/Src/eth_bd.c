
#include <string.h>

#include <utils_log.h>
#include <utils_list.h>

#include <emac_reg.h>
#include <bl_irq.h>
#include <bl702_common.h>
#include <bl702_glb.h>
#include <bl_emac.h>

#include <bl_timer.h>
#include <bl_gpio.h>

#include <lwip/netifapi.h>
#include <lwip/mem.h>
#include <netif/etharp.h>
#include <ethernetif.h>
#if LWIP_IPV6
#include <lwip/ethip6.h>
#include <lwip/dhcp6.h>
#endif /* LWIP_IPV6 */

#include <FreeRTOS.h>
#include <task.h>

#include "eth_bd.h"

#if defined(log_info)
#undef log_info
#define log_info(...) do {}while(0)
#endif
#define printf(...)  do {}while(0)
#define MSG(...)     do {}while(0)

#define USER_EMAC_OUTSIDE_CLK  (1)

EMAC_Handle_Type ethHandle = {0};
static EMAC_Handle_Type *thiz = NULL;
eth_context *ctx = NULL;
TaskHandle_t DequeueTaskHandle = NULL;
struct netif eth_mac = {0};
#define ETH_MAX_BUFFER_SIZE        (1536)
#if 0
uint32_t tx_buf[ETH_TXNB][(ETH_MAX_BUFFER_SIZE + 3)/4];/* Ethernet Transmit Buffers */
uint32_t rx_buf[ETH_RXNB][(64 + ETH_MAX_BUFFER_SIZE + 3)/4] = {0};/* Ethernet Transmit Buffers */
#else 
uint8_t tx_buf[ETH_TXNB][ETH_MAX_BUFFER_SIZE] __ALIGNED__(4); /* Ethernet Transmit Buffers */
uint8_t rx_buf[ETH_RXNB][ETH_MAX_BUFFER_SIZE + 64] __ALIGNED__(4)={0}; /* Ethernet Receive Buffers */
#endif

static eth_callback p_eth_callback = NULL;
static bool is_link_up = false;

int EMAC_DMABDListInit(EMAC_Handle_Type *handle,uint8_t *txBuff, uint32_t txBuffCount,uint8_t *rxBuff, uint32_t rxBuffCount)
{
    uint32_t i = 0;

    /* Set the Ethernet handler env */
    handle->bd = (EMAC_BD_Desc_Type *)(EMAC_BASE+EMAC_DMA_DESC_OFFSET);
    handle->txIndexEMAC = 0;
    handle->txIndexCPU  = 0;
    handle->txBuffLimit = txBuffCount - 1;
    /* The receive descriptors' address starts right after the last transmit BD. */
    handle->rxIndexEMAC = txBuffCount;
    handle->rxIndexCPU  = txBuffCount;
    handle->rxBuffLimit = txBuffCount + rxBuffCount - 1;
    for(i=0; i < txBuffCount; i++){
        /* Get the pointer on the ith member of the Tx Desc list */
        //handle->bd[i].Buffer= (NULL == txBuff) ? 0 : (uint32_t)((uint8_t *)txBuff+(ETH_MAX_BUFFER_SIZE*i)); //(uint32_t)tx_desc[i].buf;
        handle->bd[i].Buffer= (NULL == txBuff) ? 0 : (uint32_t)(txBuff+(ETH_MAX_BUFFER_SIZE*i)); //(uint32_t)tx_desc[i].buf;
        handle->bd[i].C_S_L = 0;
    }
    handle->bd[handle->txBuffLimit].C_S_L |= EMAC_BD_FIELD_MSK(TX_WR);

    for(i = txBuffCount; i < (txBuffCount + rxBuffCount); i++){
        /* Get the pointer on the ith member of the Rx Desc list */
        handle->bd[i].Buffer= (NULL == rxBuff) ? 0 : (uint32_t)(rxBuff + (ETH_MAX_BUFFER_SIZE + 64) * (i - txBuffCount) + 64);
        handle->bd[i].C_S_L = (ETH_MAX_BUFFER_SIZE << 16) |
                               EMAC_BD_FIELD_MSK(RX_IRQ) |
                               EMAC_BD_FIELD_MSK(RX_E);
    }
    /* For the last RX DMA Descriptor, it should be wrap back */
    handle->bd[handle->rxBuffLimit].C_S_L |= EMAC_BD_FIELD_MSK(RX_WR);
    
    /* For the TX DMA Descriptor, it will wrap to 0 according to EMAC_TX_BD_NUM*/
    BL_WR_REG(EMAC_BASE, EMAC_TX_BD_NUM,txBuffCount);
    
    return SUCCESS;
}

static void EMAC_GPIO_Init(void)
{
    GLB_GPIO_Type emacPins[]={GLB_GPIO_PIN_0,GLB_GPIO_PIN_1,GLB_GPIO_PIN_2,
                        GLB_GPIO_PIN_7,GLB_GPIO_PIN_8,
                        GLB_GPIO_PIN_18,GLB_GPIO_PIN_19,GLB_GPIO_PIN_20,GLB_GPIO_PIN_21,GLB_GPIO_PIN_22};

    GLB_SWAP_EMAC_CAM_Pin(GLB_EMAC_CAM_PIN_EMAC);

    GLB_GPIO_Func_Init(GPIO_FUN_ETHER_MAC,(GLB_GPIO_Type *)emacPins,sizeof(emacPins) / sizeof(GLB_GPIO_Type));
}

int EMAC_BD_Init(void)
{
    int err = SUCCESS;
    thiz = &ethHandle;
    ctx = pvPortMalloc(sizeof(eth_context));
    memset(ctx, 0, sizeof(eth_context));
    utils_list_init(&ctx->unsent);
    /* init the BDs in emac with buffer address */
    //err = EMAC_DMABDListInit(thiz, NULL, ETH_TXNB, (uint8_t*)rx_buf, ETH_RXNB);
    err = EMAC_DMABDListInit(thiz, (uint8_t*)tx_buf, ETH_TXNB, (uint8_t*)rx_buf, ETH_RXNB);
    printf("bd init success\r\n");

    return err;
}

static inline uint8_t emac_bd_get_cur_rx_buffer_active(void)
{
    uint32_t bd = 0;

    bd = BL_RD_REG(EMAC_BASE, EMAC_TX_BD_NUM);

    bd &= EMAC_RXBDPTR_MSK;
    bd >>= EMAC_RXBDPTR_POS;

    return (uint8_t)(bd & 0xff);
}

static inline uint8_t emac_bd_get_cur_tx_buffer_active(void)
{
    uint32_t bd = 0;

    bd = BL_RD_REG(EMAC_BASE, EMAC_TX_BD_NUM);
    bd &= EMAC_TXBDPTR_MSK;
    bd >>= EMAC_TXBDPTR_POS;

    return (uint8_t)(bd & 0xff);
}

void EMAC_TX_Done_Callback(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (NULL == DequeueTaskHandle) {
        return;
    }

    xTaskNotifyFromISR(DequeueTaskHandle, 0x01, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void EMAC_TX_Error_Callback(int index)
{
    /* handle error */
    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(TX_UR)) {
        printf("TX error EMAC_BD_TX_UR_MASK @%d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(TX_RTRY)) {
        printf("TX error EMAC_BD_TX_RTRY_MASK @%d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(TX_RL)) {
        printf("TX error EMAC_BD_TX_RL_MASK @%d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(TX_LC)) {
        printf("TX error EMAC_BD_TX_LC_MASK @%d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(TX_DF)) {
        printf("TX error EMAC_BD_TX_DF_MASK @%d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(TX_CS)) {
        printf("TX error EMAC_BD_TX_CS_MASK @%d\r\n", index);
    }

    thiz->bd[index].C_S_L &= ~0xff;
}

#if 0
static void rx_free_custom(struct pbuf *p)
{
#if 0
    eth_pbuf_custom_t *my_pbuf = (eth_pbuf_custom_t *)p;
    EMAC_BD_Desc_Type *bd;
    bd = my_pbuf->bd;
    bd->C_S_L |= EMAC_BD_FIELD_MSK(RX_E);
    printf("release bd idx %ld\r\n", my_pbuf->idx);

#else
    eth_pbuf_custom_t *my_pbuf = (eth_pbuf_custom_t *)p;
    EMAC_BD_Desc_Type *bd;

    /*put back BD buffer to the front of the ring*/
    bd = &thiz->bd[thiz->rxIndexEMAC];
    bd->Buffer = ((uint32_t)my_pbuf) + 64;//XXX magic number
    thiz->rxIndexEMAC++;
    if (thiz->rxIndexEMAC > thiz->rxBuffLimit) {
        /* wrap back */
        thiz->rxIndexEMAC = thiz->txBuffLimit + 1;
        /* the last BD */
        bd->C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16) | EMAC_BD_FIELD_MSK(RX_WR));
    } else {
        bd->C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16));
    }
#endif
}
#endif

static struct pbuf *low_level_input(struct netif *netif)
{
    uint16_t pkt_len;
    uint16_t max_len;
    uint32_t temval;
    struct pbuf *h = NULL;
    //uint8_t *payload;
    EMAC_BD_Desc_Type *bd;
    bd = &thiz->bd[thiz->rxIndexCPU];

    if(bd->C_S_L & EMAC_BD_FIELD_MSK(RX_E)){
        printf("RX BD is empty\r\n");
        h = NULL;
    } else {
        temval = BL_RD_REG(EMAC_BASE, EMAC_PACKETLEN);
        max_len = BL_GET_REG_BITS_VAL(temval, EMAC_MAXFL);
        pkt_len = (bd->C_S_L & EMAC_BD_FIELD_MSK(RX_LEN)) >> BD_RX_LEN_POS;
        printf("max_len %u, min_len %lu, pkt_len %u\r\n", max_len,
                BL_GET_REG_BITS_VAL(temval, EMAC_MINFL), pkt_len);
        //check length
        if (pkt_len > max_len) {
            MSG("pkt is too huge %d\r\n", pkt_len);
            return NULL;
        }
        if (bd->C_S_L & 0xFF) {
            MSG("RX bd %x\r\n", bd->C_S_L & 0xFF);
        }
        if ((bd->C_S_L >>16) == ETH_MAX_BUFFER_SIZE) {
            MSG("Bug now...\r\n");
        }
#if 0
        //TODO more check
        payload = (uint8_t *)bd->Buffer;
        printf("input idx %d\r\n", thiz->rxIndexCPU);
        eth_pbuf_custom_t *my_pbuf = (eth_pbuf_custom_t *)(payload - 64);
        my_pbuf->p.custom_free_function = rx_free_custom;
        my_pbuf->bd = bd;
        my_pbuf->idx = thiz->rxIndexCPU;
        h = pbuf_alloced_custom(PBUF_RAW, pkt_len, PBUF_REF, &my_pbuf->p, payload, pkt_len); 
        bd->C_S_L = ETH_MAX_BUFFER_SIZE;
        if ((++thiz->rxIndexCPU) > thiz->rxBuffLimit) {
            /* wrap back */
            thiz->rxIndexCPU = thiz->txBuffLimit + 1;
        }
#else

#if PBUF_POOL_SIZE
        h = pbuf_alloc(PBUF_RAW, pkt_len, PBUF_POOL);
#else
        h = pbuf_alloc(PBUF_RAW, pkt_len, PBUF_RAM);
#endif
        if (h) {
            pbuf_take(h, (const void*)bd->Buffer, pkt_len);
        }

        if ((++thiz->rxIndexCPU) > thiz->rxBuffLimit) {
            /* wrap back */
            thiz->rxIndexCPU = thiz->txBuffLimit + 1;
            bd->C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16) | EMAC_BD_FIELD_MSK(RX_WR));
        } else {
            bd->C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16));
        }
#endif
    }
    printf("h addr %p\r\n", h);
    return h;
}

static unsigned int rx_counter;
static inline err_t bl702ethernetif_input(struct netif *netif)
{
    err_t err = ERR_OK;
    struct pbuf *p;
    EMAC_BD_Desc_Type *bd;

    /* move received packet into a new pbuf */
    do {

        bd = &thiz->bd[thiz->rxIndexCPU];
        if(bd->C_S_L & EMAC_BD_FIELD_MSK(RX_E)) {

            printf("low level input idx %d, empty\r\n", thiz->rxIndexCPU);

            if (thiz->rxIndexCPU == thiz->rxBuffLimit) {
                thiz->rxIndexCPU = thiz->txBuffLimit + 1;
                bd->C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16) | EMAC_BD_FIELD_MSK(RX_WR));
            } else {
                bd->C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16));
                thiz->rxIndexCPU ++;
            }
            continue;
        }

        printf("low level input idx %d\r\n", thiz->rxIndexCPU);
        p = low_level_input(netif);

        rx_counter++;
        /* no packet could be read, silently ignore this */
        if (p == NULL) return ERR_MEM;

        /* entry point to the LwIP stack */
        err = netif->input(p, netif);

        if (err != ERR_OK)
        {
            printf("ethernetif_input: IP input error\n");
            pbuf_free(p);
        }
    } while (emac_bd_get_cur_rx_buffer_active() != thiz->rxIndexCPU);

    return err;
}

/* recv a pkt */
void EMAC_RX_Done_Callback(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (NULL == DequeueTaskHandle) {
        return;
    }

    xTaskNotifyFromISR(DequeueTaskHandle, 0x02, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void EMAC_RX_Error_Callback(int index)
{
    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(RX_OR)) {
        printf("EMAC RX OR Error @ %d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(RX_RE)) {
        printf("EMAC RX_RE Error @ %d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(RX_DN)) {
        printf("EMAC RX_DN Error @ %d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(RX_TL)) {
        printf("EMAC RX_TL Error @ %d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(RX_CRC)) {
        printf("EMAC RX_CRC Error @ %d\r\n", index);
    }

    if (thiz->bd[index].C_S_L & EMAC_BD_FIELD_MSK(RX_LC)) {
        printf("EMAC RX_LC Error @ %d\r\n", index);
    }

    if (index == thiz->rxBuffLimit) {
        thiz->bd[index].C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16) | EMAC_BD_FIELD_MSK(RX_WR));
    } else {
        thiz->bd[index].C_S_L = (EMAC_BD_FIELD_MSK(RX_IRQ) | EMAC_BD_FIELD_MSK(RX_E) | (ETH_MAX_BUFFER_SIZE << 16));
    }
}

void EMAC_RX_Busy_Callback(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (NULL == DequeueTaskHandle) {
        return;
    }

    xTaskNotifyFromISR(DequeueTaskHandle, 0x02, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    printf("RX busy %u @ %d\r\n", rx_counter, emac_bd_get_cur_rx_buffer_active());
}

void emac_irq_process(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(EMAC_BASE,EMAC_INT_MASK);

    if (SET == emac_getintstatus(EMAC_INT_TX_DONE) && !BL_IS_REG_BIT_SET(tmpVal,EMAC_TXB_M)) {
        //printf("EMAC_INT_TX_DONE\r\n");
        EMAC_TX_Done_Callback();
        emac_clrintstatus(EMAC_INT_TX_DONE);
    }

    if (SET == emac_getintstatus(EMAC_INT_TX_ERROR) && !BL_IS_REG_BIT_SET(tmpVal,EMAC_TXE_M)) {
        //printf("EMAC_INT_TX_ERROR\r\n");
        EMAC_TX_Error_Callback(thiz->txIndexCPU);
        emac_clrintstatus(EMAC_INT_TX_ERROR);
    }

    if (SET == emac_getintstatus(EMAC_INT_RX_DONE) && !BL_IS_REG_BIT_SET(tmpVal,EMAC_RXB_M)) {
        //printf("EMAC_INT_RX_DONE\r\n");
        EMAC_RX_Done_Callback();
        emac_clrintstatus(EMAC_INT_RX_DONE);
    }

    if (SET == emac_getintstatus(EMAC_INT_RX_ERROR) && !BL_IS_REG_BIT_SET(tmpVal,EMAC_RXE_M)) {
        //printf("EMAC_INT_RX_ERROR\r\n");
        EMAC_RX_Error_Callback(thiz->rxIndexCPU);
        emac_clrintstatus(EMAC_INT_RX_ERROR);
    }

    if (SET == emac_getintstatus(EMAC_INT_RX_BUSY) && !BL_IS_REG_BIT_SET(tmpVal,EMAC_BUSY_M)) {
        //printf("EMAC_INT_RX_BUSY\r\n");
        EMAC_RX_Busy_Callback();
        emac_clrintstatus(EMAC_INT_RX_BUSY);
    }

    if (SET == emac_getintstatus(EMAC_INT_TX_CTRL) && !BL_IS_REG_BIT_SET(tmpVal,EMAC_TXC_M)) {
        //printf("EMAC_INT_TX_CTRL\r\n");
        emac_clrintstatus(EMAC_INT_TX_CTRL);
    }

    if (SET == emac_getintstatus(EMAC_INT_RX_CTRL) && !BL_IS_REG_BIT_SET(tmpVal,EMAC_RXC_M)) {
        //printf("EMAC_INT_RX_CTRL\r\n");
        emac_clrintstatus(EMAC_INT_RX_CTRL);
    }
}

void EMAC_Interrupt_Init(void)
{
    log_info("EMAC_Interrupt_Init.\r\n");
    bl_irq_register(EMAC_IRQn, emac_irq_process);
    bl_irq_enable(EMAC_IRQn);

    emac_clrintstatus(EMAC_INT_ALL);
    emac_intmask(EMAC_INT_ALL, UNMASK);
    log_info("emac_intmask.\r\n");
}

//FIXME ugly extern functions
int emac_phy_reset(void);
int emac_phy_linkup(ETHPHY_CFG_Type *cfg);
int emac_phy_autonegotiate(ETHPHY_CFG_Type *cfg);

#if 0
static int _emac_phy_reset(ETHPHY_CFG_Type *cfg)
{
    uint32_t tmpVal;

    /* Set Phy Address */
    tmpVal=BL_RD_REG(EMAC_BASE, EMAC_MIIADDRESS);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,EMAC_FIAD,cfg->phyAddress);
    BL_WR_REG(EMAC_BASE, EMAC_MIIADDRESS,tmpVal);

    if (SUCCESS != emac_phy_reset()) {
        return ERROR;
    }
    return SUCCESS;
}
#endif

static int _emac_phy_autonegotiation(ETHPHY_CFG_Type *cfg)
{
    uint16_t phyReg;

    if(cfg->autoNegotiation){
        uint16_t regval = 0;
        uint16_t advertise = 0;
        uint16_t lpa = 0;
        uint32_t timeout = 0x3F0;

        if (SUCCESS != emac_phy_read(PHY_BSR, &regval) || !(PHY_LINKED_STATUS & regval)) {
            return ERROR;
        }

        if (SUCCESS != emac_phy_read(PHY_PHYID1, &regval)) {
            return ERROR;
        }

        if (SUCCESS != emac_phy_read(PHY_PHYID2, &regval)) {
            return ERROR;
        }

        if (SUCCESS != emac_phy_read(PHY_BCR, &regval)) {
            return ERROR;
        }

        regval &= ~PHY_AUTONEGOTIATION;
        regval &= ~(PHY_LOOPBACK | PHY_POWERDOWN);
        regval |= PHY_ISOLATE;
        if(emac_phy_write(PHY_BCR, regval) != SUCCESS){
            return ERROR;
        }

        /* set advertisement mode */
        advertise = PHY_ADVERTISE_100BASETXFULL | PHY_ADVERTISE_100BASETXHALF |
                 PHY_ADVERTISE_10BASETXFULL | PHY_ADVERTISE_10BASETXHALF |
                 PHY_ADVERTISE_8023;
        if(emac_phy_write(PHY_ADVERTISE, advertise) != SUCCESS){
            return ERROR;
        }

        BL702_Delay_MS(16);
        if (SUCCESS != emac_phy_read(PHY_BCR, &regval)) {
            return ERROR;
        }

        BL702_Delay_MS(16);
        regval |= (PHY_FULLDUPLEX_100M | PHY_AUTONEGOTIATION);
        if(emac_phy_write(PHY_BCR, regval) != SUCCESS){
            return ERROR;
        }

        BL702_Delay_MS(16);
        regval |= PHY_RESTART_AUTONEGOTIATION;
        regval &= ~PHY_ISOLATE;
        if(emac_phy_write(PHY_BCR, regval) != SUCCESS){
            return ERROR;
        }

        //BL702_Delay_MS(2000);
        while(1) {
            if (SUCCESS != emac_phy_read(PHY_BSR, &regval)) {
                return ERROR;
            }

            if (regval & PHY_AUTONEGO_COMPLETE) {
                /* complete */
                break;
            }

            if (!(--timeout)) {
                printf("%s:%d\r\n", __func__, __LINE__);
                return TIMEOUT;
            }
        }

        //BL702_Delay_MS(5000);
        if (SUCCESS != emac_phy_read(PHY_LPA, &lpa)) {
            printf("%s:%d\r\n", __func__, __LINE__);
            return ERROR;
        }

        if (((advertise & lpa) & PHY_ADVERTISE_100BASETXFULL) != 0)
        {
            /* 100BaseTX and Full Duplex */
            cfg->duplex = EMAC_MODE_FULLDUPLEX;
            cfg->speed = EMAC_SPEED_100M;
        }
        else if (((advertise & lpa) & PHY_ADVERTISE_10BASETXFULL) != 0)
        {
            /* 10BaseT and Full Duplex */
            cfg->duplex = EMAC_MODE_FULLDUPLEX;
            cfg->speed = EMAC_SPEED_10M;
        }
        else if (((advertise & lpa) & PHY_ADVERTISE_100BASETXHALF) != 0)
        {
            /* 100BaseTX and half Duplex */
            cfg->duplex = EMAC_MODE_HALFDUPLEX;
            cfg->speed = EMAC_SPEED_100M;
        }
        else if (((advertise & lpa) & PHY_ADVERTISE_10BASETXHALF) != 0)
        {
            /* 10BaseT and half Duplex */
            cfg->duplex = EMAC_MODE_HALFDUPLEX;
            cfg->speed = EMAC_SPEED_10M;
        }
        return SUCCESS;
    }else{
        if(emac_phy_read(PHY_BCR, &phyReg) != SUCCESS){
            return ERROR;
        }
        phyReg&=(~PHY_FULLDUPLEX_100M);
        if(cfg->speed == EMAC_SPEED_10M){
            if(cfg->duplex==EMAC_MODE_FULLDUPLEX){
                phyReg|=PHY_FULLDUPLEX_10M;
            }else{
                phyReg|=PHY_HALFDUPLEX_10M;
            }
        }else{
            if(cfg->duplex==EMAC_MODE_FULLDUPLEX){
                phyReg|=PHY_FULLDUPLEX_100M;
            }else{
                phyReg|=PHY_HALFDUPLEX_100M;
            }
        }
        if((emac_phy_write(PHY_BCR, phyReg)) != SUCCESS){
            return ERROR;
        }
    }
    return SUCCESS;
}

static int _emac_phy_setNegotiationConfig(ETHPHY_CFG_Type *cfg)
{
    uint32_t tmpVal;

    /* Set MAC duplex config */
    tmpVal=BL_RD_REG(EMAC_BASE, EMAC_MODE);
    tmpVal=BL_SET_REG_BITS_VAL(tmpVal,EMAC_FULLD,cfg->duplex);
    BL_WR_REG(EMAC_BASE, EMAC_MODE,tmpVal);

    return SUCCESS;
}

static int _emac_phy_linkup(ETHPHY_CFG_Type *cfg)
{
    emac_phy_linkup(cfg);

    return SUCCESS;
}

static void _emac_phy_is_link_up(void) 
{
    uint16_t phy_bsr = 0;

    if(NULL == p_eth_callback || is_link_up){
        return;
    }

    if (SUCCESS != emac_phy_read(PHY_BSR, &phy_bsr) || !(PHY_LINKED_STATUS & phy_bsr)) {
        return;
    }
    p_eth_callback(ETH_INIT_STEP_LINKUP);
    is_link_up = true;
}

static int _emac_phy_linkstatus(void)
{
    uint16_t phy_bsr = 0;

    //MSG("Read link\r\n");
    if (SUCCESS != emac_phy_read(PHY_BSR, &phy_bsr)) {
        printf("%s:%d\r\n", __func__, __LINE__);
        return ERROR;
    }

    if (!(PHY_LINKED_STATUS & phy_bsr)) {
        printf("%s:%d\r\n", __func__, __LINE__);

        if(p_eth_callback && is_link_up){
            p_eth_callback(ETH_INIT_STEP_LINKDOWN);
            is_link_up = false;
        }

        return ERROR;
    }

    if(p_eth_callback && false == is_link_up){
        p_eth_callback(ETH_INIT_STEP_LINKUP);
    }
    is_link_up = true;

    return SUCCESS;
}

static void _emac_phy_if_init(void)
{
#define EMAC_PHY_INIT_STEP_RESET                (0)
#define EMAC_PHY_INIT_STEP_NEGOTIATION          (1)
#define EMAC_PHY_INIT_STEP_CONFIG               (2)
#define EMAC_PHY_INIT_STEP_LINKUP               (3)
#define EMAC_PHY_INIT_STEP_SERCIVE_START        (4)
#define EMAC_PHY_INIT_STEP_LINKMAINTAIN         (5)
    static int internal_status = 0;

    int err = SUCCESS;
    static ETHPHY_CFG_Type phyCfg={
        .autoNegotiation=ENABLE,                    /*!< Speed and mode auto negotiation */
        .duplex=EMAC_MODE_FULLDUPLEX,             /*!< Duplex mode */
        .speed=EMAC_SPEED_100M,                   /*!< Speed mode */
        .phyAddress=0x1,                            /*!< PHY address */
    };

    //log_info("_emac_phy_if_init.\r\n");
    switch (internal_status) {
        case EMAC_PHY_INIT_STEP_RESET:
        {
            log_info("EMAC_PHY_INIT_STEP_RESET.\r\n");
            {
#if 0
                bl_gpio_enable_output(9, 1, 0);
                aos_msleep(10);
                bl_gpio_output_set(9, 0);
                aos_msleep(10);
                bl_gpio_output_set(9, 1);
                aos_msleep(10);
#endif
            }

            bool isAutoNegotiation = phyCfg.autoNegotiation;
            
            /** set phyCfg.autoNegotiation to false to skip negotiation in emac_phyinit */
            phyCfg.autoNegotiation = false;
            err = emac_phyinit(&phyCfg);
            if(err != SUCCESS)
            {
                log_info("emac_phyinit init err[%d].\r\n", err);
            }
            phyCfg.autoNegotiation = isAutoNegotiation;
            internal_status++;
        }
        break;
        case EMAC_PHY_INIT_STEP_NEGOTIATION:
        {
            log_info("EMAC_PHY_INIT_STEP_NEGOTIATION.\r\n");
            err = _emac_phy_autonegotiation(&phyCfg);
            if(err != SUCCESS)
            {
                log_info("EMAC_PHY_INIT_STEP_NEGOTIATION err[%d].\r\n", err);
                return;
            }
            internal_status++;
        }
        break;
        case EMAC_PHY_INIT_STEP_CONFIG:
        {
            log_info("EMAC_PHY_INIT_STEP_CONFIG.\r\n");
            err = _emac_phy_setNegotiationConfig(&phyCfg);
            if(err != SUCCESS){
                log_info("EMAC_PHY_INIT_STEP_CONFIG err[%d].\r\n", err);
            }
            internal_status++;
        }
        break;
        case EMAC_PHY_INIT_STEP_LINKUP:
        {
            log_info("EMAC_PHY_INIT_STEP_LINKUP.\r\n");
            err = _emac_phy_linkup(&phyCfg);
            log_info("speed %d\r\n", phyCfg.speed);
            if(TIMEOUT==err){
                MSG("PHY Init timeout\r\n");
                while(1);
            }
            if(ERROR==err){
                MSG("PHY Init error\r\n");
                while(1);
            }
            if(phyCfg.duplex==EMAC_MODE_FULLDUPLEX){
                MSG("EMAC_MODE_FULLDUPLEX\r\n");
            }else{
                MSG("EMAC_MODE_HALFDUPLEX\r\n");
            }
            if(phyCfg.speed==EMAC_SPEED_100M){
                MSG("EMAC_SPEED_100M\r\n");
            }else{
                MSG("EMAC_SPEED_50M\r\n");
            }
            MSG("PHY Init done\r\n");
            if(p_eth_callback){
                p_eth_callback(ETH_INIT_STEP_LINKUP);
            }
            internal_status++;
        }
        break;
        case EMAC_PHY_INIT_STEP_SERCIVE_START:
        {
            log_info("EMAC_PHY_INIT_STEP_SERCIVE_START start dhcp...\r\n");
            if(p_eth_callback){
                p_eth_callback(ETH_INIT_STEP_READY);
            }
            internal_status++;
        }
        break;
        case EMAC_PHY_INIT_STEP_LINKMAINTAIN:
        {
            //log_info("EMAC_PHY_INIT_STEP_LINKMAINTAIN.\r\n");
            _emac_phy_linkstatus();
        }
        break;
        default:
        {
            log_info("emac unkonwn internal status\r\n");
        }
    }
}

void borad_eth_init(uint8_t*addr)
{
    EMAC_CFG_Type emacCfg={
        .recvSmallFrame=ENABLE,                     /*!< Receive small frmae or not */
        .recvHugeFrame=DISABLE,                     /*!< Receive huge frmae(>64K bytes) or not */
        .padEnable=ENABLE,                          /*!< Enable padding for frame which is less than MINFL or not */
        .crcEnable=ENABLE,                          /*!< Enable hardware CRC or not */
        .noPreamble=DISABLE,                        /*!< Enable preamble or not */
        .recvBroadCast=ENABLE,                      /*!< Receive broadcast frame or not */
        .interFrameGapCheck=ENABLE,                 /*!< Check inter frame gap or not */
        .miiNoPreamble=ENABLE,                      /*!< Enable MII interface preamble or not */
        .miiClkDiv=49,                              /*!< MII interface clock divider from bus clock */
        .maxTxRetry=16,                             /*!< Maximum tx retry count */
        .interFrameGapValue=24,                     /*!< Inter frame gap vaule in clock cycles(default 24)*/
        .minFrameLen=64,                            /*!< Minimum frame length */
        .maxFrameLen=ETH_MAX_BUFFER_SIZE,           /*!< Maximum frame length */
        .collisionValid=16,                         /*!< Collision valid value */
    };

    memcpy(emacCfg.macAddr, addr, sizeof(emacCfg.macAddr));
    EMAC_GPIO_Init();

#if USER_EMAC_OUTSIDE_CLK  // use outside clk
    GLB_Set_ETH_REF_O_CLK_Sel(GLB_ETH_REF_CLK_OUT_OUTSIDE_50M);
#else  // use inside clk
    //enable audio clock for emac */
    PDS_Enable_PLL_Clk(PDS_PLL_CLK_48M);
    PDS_Set_Audio_PLL_Freq(AUDIO_PLL_50000000_HZ);
    GLB_Set_ETH_REF_O_CLK_Sel(GLB_ETH_REF_CLK_OUT_INSIDE_50M);
#endif

    GLB_AHB_Slave1_Clock_Gate(0,BL_AHB_SLAVE1_EMAC);
    EMAC_Interrupt_Init();
    emac_init(&emacCfg);
    MSG("emac_init sucess\r\n");
    EMAC_BD_Init();
    emac_enable();
    //bflb_platform_init_time();
    //bflb_platform_start_time();
}

void __attribute__((weak)) eth_user_init(struct netif *netif) 
{
    uint8_t *mac = netif->hwaddr, addr[8];

    extern int bl_wireless_mac_addr_get(uint8_t mac[8]);
    bl_wireless_mac_addr_get(addr);
    mac[0] = 0x18;
    mac[1] = 0xB0;
    mac[2] = 0x09;
    mac[3] = addr[5];
    mac[4] = addr[6];
    mac[5] = addr[7];
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */

static void low_level_init(struct netif *netif)
{
    netif->hwaddr_len = ETHARP_HWADDR_LEN;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST|NETIF_FLAG_ETHARP|NETIF_FLAG_LINK_UP|NETIF_FLAG_IGMP;
    eth_user_init(netif);

    printf("low level init\r\n");

#if LWIP_IPV6
    netif->flags |= (NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6);
    netif->output_ip6 = ethip6_output;
#endif
}

static inline void bl702ethernetif_output(struct netif *netif)
{
    struct unsent_item *item;
    struct pbuf *q, *p;
    EMAC_BD_Desc_Type *bd;
    int wrap_found, offset;

    bd = &thiz->bd[thiz->txIndexCPU];
    while (0 == (bd->C_S_L & EMAC_BD_FIELD_MSK(TX_RD)) && ctx->unsent_num) {

        __disable_irq();
        item = (struct unsent_item *)utils_list_pop_front(&ctx->unsent);
        ctx->unsent_num--;
        __enable_irq();

        p = item->p;
        p->len -= PBUF_LINK_ENCAPSULATION_HLEN;
        p->tot_len -= PBUF_LINK_ENCAPSULATION_HLEN;
        p->payload += PBUF_LINK_ENCAPSULATION_HLEN;

        if ((++thiz->txIndexCPU) > thiz->txBuffLimit) {
            /* wrap back */
            thiz->txIndexCPU = 0;
            wrap_found = 1;
        } else {
            wrap_found = 0;
        }

        offset = 0;
        for (q = p; q != NULL; q = q->next) {
            memcpy((uint8_t *)bd->Buffer + offset, (uint8_t*)q->payload, q->len);
            offset += q->len;
        }
        pbuf_free(p);

        if (offset >= ETH_MAX_BUFFER_SIZE) {
            printf ("bl702ethernetif_output %d >= %d\r\n", offset, ETH_MAX_BUFFER_SIZE);
        }

        bd->C_S_L = (wrap_found ? EMAC_BD_FIELD_MSK(TX_WR) : 0) | EMAC_TX_COMMON_FLAGS | p->tot_len << BD_TX_LEN_POS;
        bd = &thiz->bd[thiz->txIndexCPU];
    }
}

void unsent_recv_task(void *pvParameters)
{
    uint32_t NotifyValue;
    BaseType_t recv;
    struct netif *netif = (struct netif *)pvParameters;

    borad_eth_init(netif->hwaddr);

    log_info("unsent_recv_task.\r\n");
    while(1) {
        NotifyValue = 0;
        recv = xTaskNotifyWait(0, ULONG_MAX, &NotifyValue, 200);
        if (recv == pdTRUE) {

            _emac_phy_is_link_up();
            if (NotifyValue & (1 << 0)) {
                bl702ethernetif_output(netif);
            }
            if (NotifyValue & (1 << 1)) {
                bl702ethernetif_input(netif);
            }
        } else {
            /*XXX only work when we has no EVENT. Maybe buggy here??*/
            _emac_phy_if_init();
        }
    }
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct unsent_item *item;

    if (pbuf_header(p, PBUF_LINK_ENCAPSULATION_HLEN)) {
        printf("[TX] Reserve room failed for header\r\n");
        return ERR_IF;
    }
    item = (struct unsent_item *)(((uintptr_t)(p->payload + 3))&(~3));
    item->p = p;

    if (NULL == DequeueTaskHandle) {
        return ERR_IF;
    }
    
    pbuf_ref(p);

    __disable_irq();
    utils_list_push_back(&ctx->unsent, (struct utils_list_hdr *)&(item->hdr));
    ctx->unsent_num++;
    __enable_irq();
    xTaskNotify(DequeueTaskHandle, 0x01, eSetBits);

    return ERR_OK;
}

err_t eth_init(struct netif *netif)

{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    netif->name[0] = 'e';
    netif->name[1] = 't';
    netif->hostname = "702";
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;
    log_info("eth_init.\r\n");
    low_level_init(netif);
    xTaskCreate(unsent_recv_task, (const char *)"Ontput_Unsent_queue", 1024, netif, 29, &DequeueTaskHandle);

    return ERR_OK;
}

int ethernet_init(eth_callback cb)
{
    p_eth_callback = cb;
    return 0;
}

