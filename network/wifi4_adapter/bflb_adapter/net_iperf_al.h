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

#ifndef NET_IPERF_AL_H_
#define NET_IPERF_AL_H_

/*
 * DEFINITIONS
 ****************************************************************************************
 */
// Forward declarations of iperf stream
struct fhost_iperf_stream;

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
/*
 * FUNCTIONS
 ****************************************************************************************
 */
#define net_al_if_t struct netif*
#define inet_buf_tx_t struct pbuf

/// Interface types
typedef enum
{
    /// STA interface
    MGMR_VIF_STA = 0,
    /// AP interface
    MGMR_VIF_AP
};

//#define fhost_to_net_if(x) 

#define FHOST_IPERF_STACK_SIZE (4096)
#define FHOST_IPERF_PRIORITY   (28)
#define fhost_print(x, fmt, ...) printf(fmt, ##__VA_ARGS__)
#define ASSERT_ERR
#define co_ntohl    ntohl
#define co_htonl    htonl

/**
 ****************************************************************************************
 * @brief Open UDP connection as client and connect to UDP server.
 * Initialize UDP transmission and send UDP datagram to server. After sending a burst of
 * UDP frames, wait in order to meet bandwidth constraints.
 * The last UDP datagram needs to have a negative packet id to indicate the server that
 * transmission is over.
 *
 * @param[in] iperf_stream_ptr  Pointer to iperf stream
 * @return  0 if successful, -1 otherwise.
 ****************************************************************************************
 **/
int net_iperf_udp_client_run(struct fhost_iperf_stream* iperf_stream_ptr);

/**
 ****************************************************************************************
 * @brief Open UDP connection as server and listen to UDP port. Set packet reception
 * callback function to handle UDP packets. When a packet with ID < 0 is
 * received, the server report is sent to the client and statistics are printed.
 *
 * @param[in] iperf_stream_ptr  Pointer to iperf stream
 * @return  0 if successful, -1 otherwise.
 ****************************************************************************************
 **/
int net_iperf_udp_server_run(struct fhost_iperf_stream* iperf_stream_ptr);

/**
 ****************************************************************************************
 * @brief Open TCP connection as server and listen to TCP port. Wait for TCP traffic
 * to end before returning
 *
 * @param[in] iperf_stream_ptr  Pointer to iperf stream
 * @return  0 if successful, -1 otherwise.
 ****************************************************************************************
 **/
int net_iperf_tcp_server_run(struct fhost_iperf_stream* iperf_stream_ptr);

/**
 ****************************************************************************************
 * @brief Open UDP connection as client and connect to UDP server.
 * Initialize UDP transmission and send UDP datagram to server. After sending a burst of
 * UDP frames, wait in order to meet bandwidth constraints.
 * The last UDP datagram needs to have a negative packet id to indicate the server that
 * transmission is over.
 *
 * @param[in] stream  Pointer to iperf stream
 * @return  0 if successful, -1 otherwise.
 ****************************************************************************************
 **/
int net_iperf_tcp_client_run(struct fhost_iperf_stream* stream);

/**
 ****************************************************************************************
 * @brief TCP closing function. Remove TCP callbacks, print final stats,
 * close TCP protocol control block and wake-up iperf task.
 *
 * @param[in] stream  Pointer to iperf stream
 ****************************************************************************************
 **/
void net_iperf_tcp_close(struct fhost_iperf_stream *stream);
#endif //NET_IPERF_AL_H_
