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
#include "virt_net_protocol.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pkg_protocol.h"
#include "virt_net.h"

static int validate_command(const uint8_t *payload, size_t payload_length)
{
    struct pkg_protocol_cmd command;
    size_t expected_length;

    if (payload_length < sizeof(command)) {
        return -1;
    }

    memcpy(&command, payload, sizeof(command));
    switch (command.cmd) {
    case VIRT_NET_CTRL_MAC_IND:
        expected_length = sizeof(command) + 6U;
        break;
    case VIRT_NET_CTRL_GET_LINK_STATUS_IND:
        expected_length = sizeof(netbus_fs_link_status_ind_cmd_msg_t);
        break;
    case VIRT_NET_CTRL_GET_DEV_VERSION_IND:
        expected_length = sizeof(command) + sizeof(uint32_t);
        break;
    case VIRT_NET_CTRL_AP_CONNECTED_IND:
    case VIRT_NET_CTRL_AP_DISCONNECTED_IND:
        expected_length = sizeof(command);
        break;
    case VIRT_NET_CTRL_SLAVE_READY_IND:
    case VIRT_NET_CTRL_SLAVE_HEARTBEAT:
        expected_length = sizeof(command) + sizeof(uint8_t);
        break;
    case VIRT_NET_CTRL_SCAN_IND:
        {
            uint16_t record_count;
            const size_t records_offset =
                offsetof(netbus_fs_scan_ind_cmd_msg_t, records);

            if (payload_length < records_offset) {
                return -1;
            }

            memcpy(&record_count,
                   payload + offsetof(netbus_fs_scan_ind_cmd_msg_t, num),
                   sizeof(record_count));
            expected_length = records_offset +
                (size_t)record_count * sizeof(struct bflbwifi_scan_record);
        }
        break;
    default:
        return 0;
    }

    return payload_length == expected_length ? 0 : -1;
}

int virt_net_frame_validate(const void *frame, size_t frame_length)
{
    struct pkg_protocol header;
    const uint8_t *bytes = (const uint8_t *)frame;

    if (frame == NULL || frame_length < sizeof(header)) {
        return -1;
    }

    memcpy(&header, bytes, sizeof(header));
    if ((size_t)header.length != frame_length - sizeof(header)) {
        return -1;
    }

    switch (header.type) {
    case PKG_DATA_FRAME:
        return 0;
    case PKG_CMD_FRAME:
        return validate_command(bytes + sizeof(header), header.length);
    default:
        return -1;
    }
}
