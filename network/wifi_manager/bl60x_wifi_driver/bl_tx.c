/*
 * Copyright (c) 2016-2023 Bouffalolab.
 *
 * This file is part of
 *     *** Bouffalolab Software Dev Kit ***
 *      (see www.bouffalolab.com).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of Bouffalo Lab nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdio.h>
#include <netif/etharp.h>
#include <bl_os_private.h>

#include "bl_tx.h"
#include "bl_irqs.h"
#include "bl_utils.h"

#define RETRY_LIMIT_REACHED_BIT    (1 << 16)
#define LIFETIME_EXPIRED_BIT       (1 << 17)
#define FRAME_REPUSHABLE_PS_BIT    (1 << 19)
#define FRAME_REPUSHABLE_CHAN_BIT  (1 << 20)
#define FRAME_SUCCESSFUL_TX_BIT    (1 << 23)
#define DESC_DONE_TX_BIT           (1 << 31)
#define DESC_DONE_SW_TX_BIT        (1 << 30)

enum {
    TX_READY           = (0),
    TX_PAUSED_FOR_PS   = (1 << 0),
    TX_PAUSED_FOR_CHAN = (1 << 1),
};

extern struct bl_hw wifi_hw;
int internel_cal_size_tx_hdr = sizeof(struct bl_txhdr);
static struct {
    struct utils_list waiting_list;
    struct utils_list pending_list;
    uint8_t state;
} tx_cntrl[NX_VIRT_DEV_MAX] = {0};

#ifdef CFG_NETBUS_WIFI_ENABLE
int update_tx_pbuf_free_cnt_to_scratch_reg(void);
#endif

static int bl_check_tx_ok(uint8_t vif_type, uint8_t is_groupcast, uint32_t value)
{
    /* TODO: More bit to check */
    /* For handled packet, whether handled OK */
    if (1 == vif_type) {
        if (value & FRAME_SUCCESSFUL_TX_BIT) {
            return 1;
        }
    } else {
        if (!is_groupcast) {
            if (value & FRAME_SUCCESSFUL_TX_BIT) {
                return 1;
            }
        } else {
            if (value & DESC_DONE_TX_BIT) {
                return 1;
            }
        }
    }

    return 0;
}

int bl_txdatacfm(void *pthis, void *host_id)
{
    struct bl_txhdr *txhdr = NULL;
    struct bl_tx_cfm custom_cfm;
    struct ethhdr *eth = NULL;
    union bl_hw_txstatus bl_txst;
    uint8_t vif_type = 0, is_groupcast = 0, tx_vif_id = 0;
    int ret = 0;

    /* ATTENTION: About return value
     * 1. return ( -1), only for unhandled pakcet (status==0)
     * 2. return (>=0), handled packet, if there is a cb
     *    a. unicast, MUST FRAME_SUCCESSFUL_TX_BIT, return (1), other to c.
     *    b. groupcast, MUST DESC_DONE_TX_BIT, return (1), other to c.
     *    c. return (0)
     */
#if defined(CFG_CHIP_BL808) || defined(CFG_CHIP_BL606P)
    /* Host_id is txdesc_host->eth_packet */
    void *buf = ((struct txbuf_host*)host_id)->buf;
    txhdr = (struct bl_txhdr*)(buf + RWNX_HWTXHDR_ALIGN_PADS((uint32_t)buf));
    eth = (struct ethhdr *)((uint32_t)buf + PBUF_LINK_ENCAPSULATION_HLEN);
#else
    /* Host_id is pbuf */
    struct pbuf *p = (struct pbuf*)host_id;
    txhdr = (struct bl_txhdr*)(((uint32_t)p->payload) + RWNX_HWTXHDR_ALIGN_PADS((uint32_t)p->payload));
    eth = (struct ethhdr *)(((uint32_t)p->payload) + PBUF_LINK_ENCAPSULATION_HLEN);
#endif

    /* Read status in the TX control header */
    bl_txst = txhdr->status;
    if (bl_txst.value == 0) {
        bl_os_printf("[TX] FW return status is NULL!!!\n\r");
    }

    /* whether is sta and is_groupcast */
    vif_type = txhdr->vif_type;
    tx_vif_id = (NX_VIRT_DEV_MAX > 1) ? vif_type : 0;
    if (0 == vif_type) {
        is_groupcast = (eth->h_dest[0] & 0x01) ? 1 : 0;
    }

    /* Get tx result */
    ret = bl_check_tx_ok(vif_type, is_groupcast, bl_txst.value);

    /* Try to re-push it to the first of tx_list, if not handle OK */
    /* TODO: only consider tx pause reason (chan, ps) for now */
    do {
        if ((!ret) && (txhdr->repush < 2))
        {
            if (bl_txst.value & RETRY_LIMIT_REACHED_BIT)
            {
            } else if (bl_txst.value & FRAME_REPUSHABLE_CHAN_BIT) {
                tx_cntrl[tx_vif_id].state = TX_PAUSED_FOR_CHAN;
            } else if (bl_txst.value & FRAME_REPUSHABLE_PS_BIT) {
                tx_cntrl[tx_vif_id].state = TX_PAUSED_FOR_PS;
            } else {
                break;
            }

            txhdr->repush++;
            utils_list_push_back(&tx_cntrl[tx_vif_id].pending_list, &(txhdr->item));
            return 0;
        }
    } while (0);

    /* Get customer cb param */
    custom_cfm = txhdr->custom_cfm;

    /* Release buf */
#if defined(CFG_CHIP_BL808) || defined(CFG_CHIP_BL606P)
    ipc_host_txbuf_free((struct txbuf_host *)host_id);
#else
    pbuf_free(p);
#endif

    /* Notify tx status */
    if (custom_cfm.cb) {
        custom_cfm.cb(custom_cfm.cb_arg, ret > 0);
    }

    return ret;
}

static void bl_tx_push(struct bl_hw *bl_hw, struct txdesc_host *txdesc_host, void *ptxbuf, struct bl_txhdr *txhdr)
{
    struct hostdesc *host = NULL;
    struct ethhdr *ethhdr = NULL;
    struct pbuf *p = NULL, *q = NULL;
    void* buf_ptr = NULL;
    uint16_t link_offset_len = 0, loop = 0;

    /* Get hostdesc */
    memset((void*)txdesc_host->pad_txdesc, 0, sizeof(txdesc_host->pad_txdesc));
    host = &(((struct txdesc_upper*)txdesc_host->pad_txdesc)->host);

#if defined(CFG_CHIP_BL808) || defined(CFG_CHIP_BL606P)
    struct txbuf_host *txbuf = (struct txbuf_host*)ptxbuf;
    if ((void*)txhdr->p != (void*)txbuf)
    {
        /* Get pbuf */
        p = (struct pbuf *)txhdr->p;
        /* Get ethernet header in pbuf */
        ethhdr = (struct ethhdr *)((uint8_t*)p->payload + PBUF_LINK_ENCAPSULATION_HLEN);
    }
    else
    {
        /* Get ethernet header in txbuf */
        ethhdr = (struct ethhdr *)((uint8_t*)txbuf->buf + PBUF_LINK_ENCAPSULATION_HLEN);
    }
#else
    /* Get pbuf */
    p = (struct pbuf *)txhdr->p;
    /* Get ethernet header in pbuf */
    ethhdr = (struct ethhdr *)((uint8_t*)p->payload + PBUF_LINK_ENCAPSULATION_HLEN);
#endif

    /* eth header is skipped in the header */
    link_offset_len = sizeof(struct ethhdr) + PBUF_LINK_ENCAPSULATION_HLEN;
    /* Fill-in the descriptor */
    memcpy(&host->eth_dest_addr, ethhdr->h_dest, ETH_ALEN);
    memcpy(&host->eth_src_addr, ethhdr->h_source, ETH_ALEN);
    host->ethertype   = ethhdr->h_proto;
    host->vif_type    = txhdr->vif_type;
    host->flags       = 0;
    host->packet_len  = txhdr->len - link_offset_len;
    host->packet_addr = (uint32_t)(0x11111111); // FIXME we use this magic for unvaild packet_addr
#if 0
    /* Fill-in staid and tid later in ipc_emb */
    host->staid;
    host->tid;
#endif

    /* 
     * Buffer transfer and descriptor tranform
     */
#if defined(CFG_CHIP_BL808) || defined(CFG_CHIP_BL606P)
    /* Get right buffer pointer */
    struct bl_txhdr *new_bl_txhdr = NULL;
    uint16_t len = 0;
    void* ptr    = (void*)txbuf->buf;
    buf_ptr      = (void*)txbuf;

    /* Whether this packet is the first time */
    if ((void*)txhdr->p != (void*)txbuf)
    {
        /* Copy buf from pbuf to txdesc_host->eth_packet */
        uint16_t align_src = RWNX_HWTXHDR_ALIGN_PADS((uint32_t)p->payload);
        uint16_t align_dst = RWNX_HWTXHDR_ALIGN_PADS((uint32_t)ptr);
        new_bl_txhdr = (struct bl_txhdr*)((uint8_t*)ptr+align_dst);

        /* Copy txhdr */
        memcpy(ptr + align_dst, p->payload + align_src, sizeof(struct bl_txhdr));
        /* Copy packet */
        for (q = p; q != NULL; q = q->next)
        {
            if (0 == loop)
            {
                memcpy(ptr + PBUF_LINK_ENCAPSULATION_HLEN,
                       q->payload + PBUF_LINK_ENCAPSULATION_HLEN,
                       q->len - PBUF_LINK_ENCAPSULATION_HLEN);
            }
            else
            {
                memcpy(ptr + len, q->payload, q->len);
            }
            len += q->len;
            loop++;
        }

        memcpy(new_bl_txhdr, txhdr, sizeof(struct bl_txhdr));
        new_bl_txhdr->p = (uint32_t*)txbuf;

        /* Free pbuf, dont need it anymore */
        pbuf_free(p);

#ifdef CFG_NETBUS_WIFI_ENABLE
        update_tx_pbuf_free_cnt_to_scratch_reg();
#endif
    }
    else
    {
        new_bl_txhdr = txhdr;
        len = new_bl_txhdr->len;
    }

    /* Packet to pbuf_chained_ptr */
    host->pbuf_chained_ptr[0] = (uint32_t)((uint8_t*)ptr + link_offset_len);
    host->pbuf_chained_len[0] = len - link_offset_len;
    host->status_addr = (uint32_t)(&(new_bl_txhdr->status));
#else
    /* Get right buffer pointer, NO Copy */
    buf_ptr = (void*)p;

    /* Pbuf to pbuf_chained_ptr */
    for (q = p; q != NULL; q = q->next)
    {
        if (0 == loop)
        {
            /* The first pbuf */
            host->pbuf_chained_ptr[loop] = (uint32_t)((uint8_t*)p->payload + link_offset_len);
            host->pbuf_chained_len[loop] = q->len - link_offset_len;
        }
        else
        {
            /* Chained pbuf after */
            host->pbuf_chained_ptr[loop] = (uint32_t)(q->payload);
            host->pbuf_chained_len[loop] = q->len;
        }
        loop++;
    }
    if (loop >= 4)
    {
        /* exceed the limit for pbuf chained */
        bl_os_log_warn("[TX] [PBUF] Please fix for bigger chained pbuf, total_len %d\r\n",
                        p->tot_len);
    }
    else if (loop > 2)
    {
        bl_os_log_warn("[TX] [LOOP] Chain Used %d\r\n", loop);
    }
    host->status_addr = (uint32_t)(&(txhdr->status));

    /* No pbuf_free */
#endif

    /* Give right buf ptr */
    host->pbuf_addr   = (uint32_t)buf_ptr;

    /* IPC push */
    ipc_host_txdesc_push(bl_hw->ipc_env, buf_ptr);

#ifdef CFG_BL_STATISTIC
    bl_hw->stats.cfm_balance++;
#endif
}

static void bl_tx_flow_control(struct ke_tx_fc *tx_fc_field)
{
    /* TODO:
     * LWIP   trigger: todo
     * FW cfm trigger: todo
     * FW fc  trigger: consider interface/reason for now,
     *                 consider interface/staid/ac/reason for future
     */
    uint16_t tx_vif_id = 0;

    for (uint8_t i = 0; i < 2; i++)
    {
        if (tx_fc_field->interface_bits & (1 << i))
        {
            tx_vif_id = (NX_VIRT_DEV_MAX > 1) ? i : 0;
            if (i == 0)
            {
                tx_cntrl[tx_vif_id].state &= ~(tx_fc_field->ap.reason_bits[0]);
            }
            else
            {
                tx_cntrl[tx_vif_id].state &= ~(tx_fc_field->sta.reason_bits);
            }
        }
    }
}

void bl_tx_try_flush(int param, struct ke_tx_fc *tx_fc_field)
{
    struct txdesc_host *txdesc_host = NULL;
    struct bl_txhdr          *txhdr = NULL;
    void                     *txbuf = NULL;

    /* Whether need to manipulate flow control */
    if (param && tx_fc_field)
    {
        bl_tx_flow_control(tx_fc_field);
    }

    // TX
    for (uint8_t i = 0; i < NX_VIRT_DEV_MAX; i++)
    {
        // whether paused
        if (tx_cntrl[i].state != TX_READY) {
            continue;
        }

        if (!utils_list_is_empty(&tx_cntrl[i].pending_list))
        {
            while (1)
            {
                /* Get txdesc_host */
                txdesc_host = (struct txdesc_host *)ipc_host_txdesc_get(wifi_hw.ipc_env);
                if (!txdesc_host)
                {
                    bl_os_log_warn("[TX] no more txdesc, wait!\n\r");
                    break;
                }

                /* First get packet from pending list */
                txhdr = (struct bl_txhdr *)utils_list_pop_front(&tx_cntrl[i].pending_list);
                if (!txhdr)
                {
                    break;
                }

                /* Just push */
                bl_tx_push(&wifi_hw, txdesc_host, (void*)txhdr->p, txhdr);
            }
        }

        if (!utils_list_is_empty(&tx_cntrl[i].waiting_list))
        {
            while (1)
            {
                /* Get txdesc_host */
                txdesc_host = (struct txdesc_host *)ipc_host_txdesc_get(wifi_hw.ipc_env);
                if (!txdesc_host)
                {
                    bl_os_log_warn("[TX] no more txdesc, wait!\n\r");
                    break;
                }

#if defined(CFG_CHIP_BL808) || defined(CFG_CHIP_BL606P)
                /* Get txbuf_host */
                txbuf = (struct txbuf_host *)ipc_host_txbuf_get(wifi_hw.ipc_env);
                if (!txbuf)
                {
                    bl_os_log_warn("[TX] no more txbuf, wait!\n\r");
                    break;
                }
#endif
                /* Get packet */
                bl_os_enter_critical();
                txhdr = (struct bl_txhdr *)utils_list_pop_front(&tx_cntrl[i].waiting_list);
                bl_os_exit_critical();
                if (!txhdr)
                {
#if defined(CFG_CHIP_BL808) || defined(CFG_CHIP_BL606P)
                    ipc_host_txbuf_free(txbuf);
#endif
                    break;
                }

                /* Packet push */
                bl_tx_push(&wifi_hw, txdesc_host, (void*)txbuf, txhdr);
            }
        }
    }
}

#ifdef CFG_NETBUS_WIFI_ENABLE
err_t bl_output(struct bl_hw *bl_hw, struct netif *netif, struct pbuf *p, int is_sta, struct bl_tx_cfm *custom_cfm, uint8_t from_local)
#else
err_t bl_output(struct bl_hw *bl_hw, struct netif *netif, struct pbuf *p, int is_sta, struct bl_tx_cfm *custom_cfm)
#endif
{
    struct ethhdr  *ethhdr = NULL;
    struct bl_txhdr *txhdr = NULL;
    uint16_t align_offset = 0, link_desc_len = 0, tx_vif_id = 0;

    /* NULL protection */
    if (!bl_hw || !netif || !p)
    {
        bl_os_printf("[TX] NULL parameters!\r\n");
        return ERR_CONN;
    }

    ethhdr = (struct ethhdr *)(p->payload);

    /* Avoid call output when Wi-Fi is not ready */
    if (!(NETIF_FLAG_LINK_UP & netif->flags) && (ethhdr->h_proto != 0x8e88)) {
        bl_os_printf("[TX] wifi is down, return now\r\n");
        return ERR_CONN;
    }

    /* Make room in the header for tx */
    if (pbuf_header(p, PBUF_LINK_ENCAPSULATION_HLEN)) {
        bl_os_printf("[TX] Reserve room failed for header\r\n");
        return ERR_IF;
    }

    /* Check reserved len for link 
     *           PBUF_LINK_ENCAPSULATION_HLEN (48)
     * | align (8) | struct bl_txhdr (24) | reserved (16) |
     */
    align_offset = RWNX_HWTXHDR_ALIGN_PADS((uint32_t)p->payload);
    link_desc_len = align_offset + sizeof(struct bl_txhdr) + 16;
    if (link_desc_len > PBUF_LINK_ENCAPSULATION_HLEN) {
        bl_os_printf("[TX] link_header size is %ld vs header %u\r\n",
                     link_desc_len, PBUF_LINK_ENCAPSULATION_HLEN);
        return ERR_BUF;
    }

    /* Use aligned link_header */
    txhdr = (struct bl_txhdr *)(p->payload + align_offset);
    memset(txhdr, 0, sizeof(struct bl_txhdr));

    /* Fill-in txhdr */
    if (custom_cfm) {
        memcpy(&txhdr->custom_cfm, custom_cfm, sizeof(*custom_cfm));
    }
    txhdr->status.value = 0;
    txhdr->vif_type = is_sta;
    txhdr->p = (uint32_t*)p; // XXX pattention to this filed
    txhdr->len = p->tot_len;

    /* Ref this pbuf to avoid pbuf release */
#ifdef CFG_NETBUS_WIFI_ENABLE
    if (from_local) {
        pbuf_ref(p);
    }
#else
    pbuf_ref(p);
#endif

    /* Push packet into waiting_list */
    tx_vif_id = (NX_VIRT_DEV_MAX > 1) ? txhdr->vif_type : 0;
    bl_os_enter_critical();
    utils_list_push_back(&tx_cntrl[tx_vif_id].waiting_list, &(txhdr->item));
    if (TX_READY == tx_cntrl[tx_vif_id].state)
    {
        /* Trigger irq */
        bl_irq_handler();
    }
    bl_os_exit_critical();

    return ERR_OK;
}
