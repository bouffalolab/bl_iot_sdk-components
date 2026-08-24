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
#include "virt_net_pending.h"

#include <string.h>

static struct virt_net_pending_entry *find_entry(
    struct virt_net_pending_table *table,
    uint16_t token)
{
    unsigned int i;

    for (i = 0; i < VIRT_NET_PENDING_CAPACITY; i++) {
        if (table->entries[i].state != VIRT_NET_PENDING_FREE &&
            table->entries[i].token == token) {
            return &table->entries[i];
        }
    }

    return NULL;
}

static void clear_entry(struct virt_net_pending_entry *entry)
{
    memset(entry, 0, sizeof(*entry));
}

static struct virt_net_pending_entry *find_free_entry(
    struct virt_net_pending_table *table)
{
    unsigned int i;

    for (i = 0; i < VIRT_NET_PENDING_CAPACITY; i++) {
        if (table->entries[i].state == VIRT_NET_PENDING_FREE) {
            return &table->entries[i];
        }
    }

    return NULL;
}

static uint16_t take_next_token(struct virt_net_pending_table *table)
{
    uint16_t token = table->next_token;

    table->next_token = (uint16_t)(token + 1U);
    if (token == VIRT_NET_PENDING_UNSOLICITED_TOKEN) {
        token = 0U;
        table->next_token = 1U;
    }

    return token;
}

void virt_net_pending_init(struct virt_net_pending_table *table,
                           uint16_t first_token)
{
    if (table == NULL) {
        return;
    }

    memset(table, 0, sizeof(*table));
    table->next_token = first_token;
}

int virt_net_pending_acquire(struct virt_net_pending_table *table,
                             uint16_t expected_cmd,
                             void *waiter,
                             void *context,
                             virt_net_pending_complete_fn complete,
                             uint16_t *token_out)
{
    struct virt_net_pending_entry *entry;
    uint16_t initial_next_token;
    uint16_t token;
    unsigned int attempt;

    if (table == NULL || waiter == NULL || complete == NULL ||
        token_out == NULL) {
        return -1;
    }

    entry = find_free_entry(table);
    if (entry == NULL) {
        return -1;
    }

    initial_next_token = table->next_token;
    /*
     * A free slot leaves at most 31 active tokens. take_next_token() emits
     * distinct non-sentinel values over these 32 attempts, so one is free.
     */
    for (attempt = 0; attempt < VIRT_NET_PENDING_CAPACITY; attempt++) {
        token = take_next_token(table);
        if (find_entry(table, token) != NULL) {
            continue;
        }

        entry->state = VIRT_NET_PENDING_WAITING;
        entry->token = token;
        entry->expected_cmd = expected_cmd;
        entry->waiter = waiter;
        entry->context = context;
        entry->complete = complete;
        *token_out = token;
        return 0;
    }

    table->next_token = initial_next_token;
    return -1;
}

enum virt_net_pending_claim_result virt_net_pending_claim(
    struct virt_net_pending_table *table,
    uint16_t token,
    uint16_t response_cmd,
    struct virt_net_pending_entry *snapshot_out)
{
    struct virt_net_pending_entry *entry;

    if (table == NULL || snapshot_out == NULL) {
        return VIRT_NET_PENDING_NO_MATCH;
    }

    entry = find_entry(table, token);
    if (entry == NULL || entry->state != VIRT_NET_PENDING_WAITING ||
        entry->expected_cmd != response_cmd) {
        return VIRT_NET_PENDING_NO_MATCH;
    }

    entry->state = VIRT_NET_PENDING_COMPLETING;
    *snapshot_out = *entry;

    return VIRT_NET_PENDING_CLAIMED;
}

enum virt_net_pending_state virt_net_pending_cancel(
    struct virt_net_pending_table *table,
    uint16_t token)
{
    struct virt_net_pending_entry *entry;
    enum virt_net_pending_state state;

    if (table == NULL) {
        return VIRT_NET_PENDING_FREE;
    }

    entry = find_entry(table, token);
    if (entry == NULL) {
        return VIRT_NET_PENDING_FREE;
    }

    state = entry->state;
    if (state == VIRT_NET_PENDING_WAITING) {
        clear_entry(entry);
    }

    return state;
}

int virt_net_pending_finish(struct virt_net_pending_table *table,
                            uint16_t token,
                            void **waiter_out)
{
    struct virt_net_pending_entry *entry;
    void *waiter;

    if (table == NULL || waiter_out == NULL) {
        return -1;
    }
    *waiter_out = NULL;

    entry = find_entry(table, token);
    if (entry == NULL || entry->state != VIRT_NET_PENDING_COMPLETING) {
        return -1;
    }

    waiter = entry->waiter;
    clear_entry(entry);
    *waiter_out = waiter;

    return 0;
}
