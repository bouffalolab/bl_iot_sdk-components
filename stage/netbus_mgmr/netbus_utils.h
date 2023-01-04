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
#ifndef __NETBUG_UTILS_H__
#define __NETBUG_UTILS_H__

#include <stdbool.h>
#include <stdint.h>
#include <lwip/def.h>
#include <lwip/prot/ethernet.h>
#include <lwip/prot/ip.h>
#include <lwip/prot/ip4.h>
#include <lwip/prot/udp.h>
#include <lwip/pbuf.h>

#define NPF_ETH_HDR_LEN         (14)
#define NPF_IP4_HDR_LEN         (20)
#define NPF_UDP4_HDR_LEN        (8)

static inline bool npf_is_arp(const struct pbuf *skb)
{
    const struct eth_hdr *hdr = (const struct eth_hdr *)skb->payload;
    if (hdr->type == PP_HTONS(ETHTYPE_ARP)) {
        return true;
    }
    return false;
}

static inline bool npf_is_ip4(const struct pbuf *skb)
{
    const struct eth_hdr *hdr = (const struct eth_hdr *)skb->payload;
    if (hdr->type == PP_HTONS(ETHTYPE_IP)) {
        return true;
    }
    return false;
}

static inline bool npf_is_dhcp(const struct pbuf *skb)
{
    const struct ip_hdr *iphdr =
        (const struct ip_hdr *)(skb->payload + NPF_ETH_HDR_LEN);
    const struct udp_hdr *udphdr =
        (const struct udp_hdr *)(skb->payload + NPF_ETH_HDR_LEN + NPF_IP4_HDR_LEN);
    const uint16_t srcport = lwip_ntohs(udphdr->src);
    const uint16_t dstport = lwip_ntohs(udphdr->dest);

    if ( ((srcport == 67 && dstport == 68) ||
         (srcport == 68 && dstport == 67)) &&
         (IPH_PROTO(iphdr) == IP_PROTO_UDP) &&
         npf_is_ip4(skb)
        ) {
        return true;
    }
    return false;
}

#endif
