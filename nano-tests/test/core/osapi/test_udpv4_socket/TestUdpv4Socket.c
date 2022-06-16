/* Copyright 2022 Real-Time Innovations, Inc. (RTI)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "nano/nano_core_test.h"
#include "TestUdpv4Socket.h"

#define xstr(s) str(s)
#define str(s) #s

#ifdef NANO_HAVE_UDPV4
static void test_NANO_OSAPI_Udpv4Socket_open_close(void)
{
  NANO_RetCode rc = NANO_RETCODE_ERROR;
  NANO_OSAPI_Udpv4Socket socket = NANO_OSAPI_UDPV4SOCKET_INITIALIZER;
  NANO_OSAPI_Udpv4SocketProperties socket_props =
    NANO_OSAPI_UDPV4SOCKETPROPERTIES_INITIALIZER;
  NANO_Ipv4Addr socket_addr = 0;
  socket_addr = NANOTEST_host_ip_address();
  rc = NANO_OSAPI_Udpv4Socket_open(
    &socket, (const NANO_u8*)&socket_addr, 7402, &socket_props);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  rc = NANO_OSAPI_Udpv4Socket_close(&socket);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
}

static void test_NANO_OSAPI_Udpv4Socket_send_recv(void)
{
  NANO_MessageBufferData rx_buf[NANO_MESSAGEBUFFER_INLINE_SIZE(8)] = {0};
  NANO_MessageBuffer tx_mbuf = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer * const rx_mbuf = (NANO_MessageBuffer*)rx_buf;
  NANO_RetCode rc = NANO_RETCODE_ERROR;
  NANO_OSAPI_Udpv4Socket socket1 = NANO_OSAPI_UDPV4SOCKET_INITIALIZER;
#if !NANOTEST_FEAT_UDP_ECHO
  NANO_OSAPI_Udpv4Socket socket2 = NANO_OSAPI_UDPV4SOCKET_INITIALIZER;
#endif /* !NANOTEST_FEAT_UDP_ECHO */
  NANO_OSAPI_Udpv4Socket * socket_rx = NULL;
  NANO_OSAPI_Udpv4SocketProperties socket_props =
    NANO_OSAPI_UDPV4SOCKETPROPERTIES_INITIALIZER;
  NANO_Ipv4Addr socket_addr_tx = 0;
  NANO_Ipv4Addr tx_addr = 0;
  NANO_u16 tx_port = 0;
  NANO_usize rx_size = 0;
  NANO_usize i = 0;
  NANO_u16 expected_port = NANOTEST_LIMIT_UDP_PORT_1;
  static const NANO_Ipv4Addr socket_addr_any = 0;
  static const NANO_u64 canary = 0xAAAAAAAAAAAAAAAAULL;

  NANO_MessageBuffer_set_external_data(
    &tx_mbuf, (NANO_u8*)&canary, sizeof(canary));
  NANO_MessageBuffer_flags_set_inline(rx_mbuf);
  NANO_MessageBuffer_set_data_len(rx_mbuf, 8);

  rc = NANO_OSAPI_Udpv4Socket_open(
    &socket1,
    (const NANO_u8*)&socket_addr_any,
    NANOTEST_LIMIT_UDP_PORT_1,
    &socket_props);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);


#if !NANOTEST_FEAT_UDP_ECHO
  rc = NANO_OSAPI_Udpv4Socket_open(
    &socket2,
    (const NANO_u8*)&socket_addr_any,
    NANOTEST_LIMIT_UDP_PORT_2,
    &socket_props);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);

  socket_addr_tx = NANOTEST_host_ip_address();\
  socket_rx = &socket2;
#else /* !NANOTEST_FEAT_UDP_ECHO */
  /* Messages will be sent and received back from a echo server. The server must
     reply using a UDP socket bound to the same port as the sender. */
  socket_addr_tx = NANOTEST_agent_ip_address();
  socket_rx = &socket1;
  expected_port = NANOTEST_LIMIT_UDP_PORT_2;
#endif /* NANOTEST_FEAT_UDP_ECHO */

#define MAX_TRIES 10

  for (i = 0;;i++)
  {
    if (i >= MAX_TRIES)
    {
      /* max tries reached, fail test */
      TEST_ASSERT_TRUE_MESSAGE(0,
        "failed to send and receive UDP message after "
        xstr(MAX_TRIES) " tentatives");
    }
    rc = NANO_OSAPI_Udpv4Socket_send(
      &socket1,
      (const NANO_u8*)&socket_addr_tx,
      NANOTEST_LIMIT_UDP_PORT_2,
      &tx_mbuf);
    TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
    NANO_MessageBuffer_set_data_len(rx_mbuf, 8);
    NANO_OSAPI_Memory_zero(
      NANO_MessageBuffer_data_ptr(rx_mbuf),
      NANO_MessageBuffer_data_len(rx_mbuf));
    rc = NANO_OSAPI_Udpv4Socket_recv(
      socket_rx, (NANO_u8*)&tx_addr, &tx_port, rx_mbuf, &rx_size);
    TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
    if (rx_size == 0)
    {
      static const NANO_u64 zero = 0;
      TEST_ASSERT_EQUAL_UINT32(0, tx_addr);
      TEST_ASSERT_EQUAL_UINT16(0, tx_port);
      TEST_ASSERT_EQUAL_UINT64(
        0, NANO_MessageBuffer_data_offset(rx_mbuf));
      TEST_ASSERT_EQUAL_UINT64(
        0, NANO_MessageBuffer_data_len(rx_mbuf));
      TEST_ASSERT_EQUAL_UINT8_ARRAY(
        &zero, NANO_MessageBuffer_data_ptr(rx_mbuf), sizeof(zero));
#ifdef NANOTEST_HAVE_SLEEP
      NANOTEST_sleep(1000);
#endif /* NANOTEST_HAVE_SLEEP */
      continue;
    } else {
      break;
    }
  }
  TEST_ASSERT_EQUAL_UINT32(socket_addr_tx, tx_addr);
  TEST_ASSERT_EQUAL_UINT16(expected_port, tx_port);
  TEST_ASSERT_EQUAL_UINT64(sizeof(canary), rx_size);
  TEST_ASSERT_EQUAL_UINT64(
    0, NANO_MessageBuffer_data_offset(rx_mbuf));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(canary), NANO_MessageBuffer_data_len(rx_mbuf));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    &canary, NANO_MessageBuffer_data_ptr(rx_mbuf), sizeof(canary));
  rc = NANO_OSAPI_Udpv4Socket_close(&socket1);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
#if !NANOTEST_FEAT_UDP_ECHO
  rc = NANO_OSAPI_Udpv4Socket_close(&socket2);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
#endif /* !NANOTEST_FEAT_UDP_ECHO */
}

#define test_NANO_OSAPI_Udpv4Socket_ENTRIES \
{\
  RUN_TEST(test_NANO_OSAPI_Udpv4Socket_open_close);\
  RUN_TEST(test_NANO_OSAPI_Udpv4Socket_send_recv);\
}
#else
#define test_NANO_OSAPI_Udpv4Socket_ENTRIES
#endif /* NANO_HAVE_UDPV4 */

NANOTEST_RUNNER_FN(test_NANO_OSAPI_Udpv4Socket,
  test_NANO_OSAPI_Udpv4Socket_ENTRIES)

NANOTEST_RUNNER_MAIN(test_NANO_OSAPI_Udpv4Socket)
