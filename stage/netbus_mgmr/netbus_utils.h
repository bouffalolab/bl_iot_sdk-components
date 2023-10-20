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
