/*
 * Copyright (c) 2016-2026 Bouffalolab.
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
#ifndef VIRT_NET_PENDING_H
#define VIRT_NET_PENDING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VIRT_NET_PENDING_CAPACITY 32U
#define VIRT_NET_PENDING_UNSOLICITED_TOKEN UINT16_MAX

enum virt_net_pending_state {
    VIRT_NET_PENDING_FREE = 0,
    VIRT_NET_PENDING_WAITING,
    VIRT_NET_PENDING_COMPLETING,
};

enum virt_net_pending_claim_result {
    VIRT_NET_PENDING_NO_MATCH = 0,
    VIRT_NET_PENDING_CLAIMED,
};

typedef int (*virt_net_pending_complete_fn)(void *context,
                                            const void *payload,
                                            size_t payload_len);

struct virt_net_pending_entry {
    enum virt_net_pending_state state;
    uint16_t token;
    uint16_t expected_cmd;
    void *waiter;
    void *context;
    virt_net_pending_complete_fn complete;
};

struct virt_net_pending_table {
    struct virt_net_pending_entry entries[VIRT_NET_PENDING_CAPACITY];
    uint16_t next_token;
};

/*
 * This module performs no locking. Initialize the table before publishing it,
 * then hold the same caller-owned lock around every operation below.
 *
 * Tokens use the full 16-bit wire field and never use UINT16_MAX, which is the
 * unsolicited-command sentinel. The wire format cannot distinguish a response
 * delayed across a complete 65,535-token cycle from a current response that
 * reuses the same token and expected command.
 */
void virt_net_pending_init(struct virt_net_pending_table *table,
                           uint16_t first_token);

/*
 * waiter, complete, and token_out are required; context may be NULL. On
 * failure, the table and token_out remain unchanged.
 */
int virt_net_pending_acquire(struct virt_net_pending_table *table,
                             uint16_t expected_cmd,
                             void *waiter,
                             void *context,
                             virt_net_pending_complete_fn complete,
                             uint16_t *token_out);

/* snapshot_out is required; a rejected claim does not change the entry. */
enum virt_net_pending_claim_result virt_net_pending_claim(
    struct virt_net_pending_table *table,
    uint16_t token,
    uint16_t response_cmd,
    struct virt_net_pending_entry *snapshot_out);

/*
 * WAITING means cancellation won and the entry was freed. COMPLETING means a
 * completion already claimed the entry and cancellation left it unchanged.
 * FREE means no matching live entry exists.
 */
enum virt_net_pending_state virt_net_pending_cancel(
    struct virt_net_pending_table *table,
    uint16_t token);

/*
 * Finish only a matching COMPLETING entry, return its waiter, and free it.
 * waiter_out is required; a NULL output leaves the entry unchanged.
 */
int virt_net_pending_finish(struct virt_net_pending_table *table,
                            uint16_t token,
                            void **waiter_out);

#ifdef __cplusplus
}
#endif

#endif
