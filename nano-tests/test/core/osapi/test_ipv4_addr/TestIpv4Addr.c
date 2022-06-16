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
#include "TestIpv4Addr.h"

#if NANO_FEAT_TRANSPORT_IPV4
static void test_NANO_OSAPI_Ipv4Addr_from_octets(void)
{
  NANO_Ipv4Addr addr = 0;
  NANO_u8 * addr_ptr = (NANO_u8*)&addr;
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 127, 0, 0, 1);
  TEST_ASSERT_EQUAL_HEX8(0x7f, addr_ptr[0]);
  TEST_ASSERT_EQUAL_HEX8(0x00, addr_ptr[1]);
  TEST_ASSERT_EQUAL_HEX8(0x00, addr_ptr[2]);
  TEST_ASSERT_EQUAL_HEX8(0x01, addr_ptr[3]);
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 255, 255, 255, 255);
  TEST_ASSERT_EQUAL_HEX8(0xff, addr_ptr[0]);
  TEST_ASSERT_EQUAL_HEX8(0xff, addr_ptr[1]);
  TEST_ASSERT_EQUAL_HEX8(0xff, addr_ptr[2]);
  TEST_ASSERT_EQUAL_HEX8(0xff, addr_ptr[3]);
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 192, 168, 1, 1);
  TEST_ASSERT_EQUAL_HEX8(0xc0, addr_ptr[0]);
  TEST_ASSERT_EQUAL_HEX8(0xa8, addr_ptr[1]);
  TEST_ASSERT_EQUAL_HEX8(0x01, addr_ptr[2]);
  TEST_ASSERT_EQUAL_HEX8(0x01, addr_ptr[3]);
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 10, 10, 15, 16);
  TEST_ASSERT_EQUAL_HEX8(0x0a, addr_ptr[0]);
  TEST_ASSERT_EQUAL_HEX8(0x0a, addr_ptr[1]);
  TEST_ASSERT_EQUAL_HEX8(0x0f, addr_ptr[2]);
  TEST_ASSERT_EQUAL_HEX8(0x10, addr_ptr[3]);
}

static void test_NANO_OSAPI_Ipv4Addr_is_multicast(void)
{
  NANO_Ipv4Addr addr = 0;
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 127, 0, 0, 1);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 255, 255, 255, 255);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 192, 168, 1, 1);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 10, 10, 15, 16);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 224, 0, 0, 125);
  TEST_ASSERT_TRUE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 224, 0, 0, 125);
  TEST_ASSERT_TRUE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 228, 10, 11, 12);
  TEST_ASSERT_TRUE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 239, 255, 0, 1);
  TEST_ASSERT_TRUE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 239, 255, 0, 2);
  TEST_ASSERT_TRUE(NANO_OSAPI_Ipv4Addr_is_multicast(&addr));
}

static void test_NANO_OSAPI_Ipv4Addr_is_loopback(void)
{
  NANO_Ipv4Addr addr = 0;
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 127, 0, 0, 1);
  TEST_ASSERT_TRUE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 127, 1, 1, 1);
  TEST_ASSERT_TRUE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 255, 255, 255, 255);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 192, 168, 1, 1);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 10, 10, 15, 16);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 224, 0, 0, 125);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 224, 0, 0, 125);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 228, 10, 11, 12);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 239, 255, 0, 1);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
  NANO_OSAPI_Ipv4Addr_from_octets(&addr, 239, 255, 0, 2);
  TEST_ASSERT_FALSE(NANO_OSAPI_Ipv4Addr_is_loopback(&addr));
}

#if NANO_FEAT_TRANSPORT_STRING
static void test_NANO_OSAPI_Ipv4Addr_from_str(void)
{
  NANO_Ipv4Addr addr = 0;
  NANO_u8 * const addr_ptr = (NANO_u8*)&addr;
  NANO_OSAPI_Ipv4Addr_from_str(&addr, "127.0.0.1");
  TEST_ASSERT_EQUAL_HEX8(0x7f, addr_ptr[0]);
  TEST_ASSERT_EQUAL_HEX8(0x00, addr_ptr[1]);
  TEST_ASSERT_EQUAL_HEX8(0x00, addr_ptr[2]);
  TEST_ASSERT_EQUAL_HEX8(0x01, addr_ptr[3]);
  NANO_OSAPI_Ipv4Addr_from_str(&addr, "10.10.15.16");
  TEST_ASSERT_EQUAL_HEX8(0x0a, addr_ptr[0]);
  TEST_ASSERT_EQUAL_HEX8(0x0a, addr_ptr[1]);
  TEST_ASSERT_EQUAL_HEX8(0x0f, addr_ptr[2]);
  TEST_ASSERT_EQUAL_HEX8(0x10, addr_ptr[3]);
}
#endif /* NANO_FEAT_TRANSPORT_STRING */

#if NANO_FEAT_TRANSPORT_AUTOCFG && \
    NANO_FEAT_TRANSPORT_IP && \
    defined(NANO_HAVE_IFCONFIG)
static void test_NANO_OSAPI_Ipv4Addr_list(void)
{
  NANO_Ipv4Addr addrs[4] = {0};
  NANO_usize addrs_len = 4;
  NANO_usize i = 0;
  NANO_RetCode rc = NANO_RETCODE_ERROR;

  rc = NANO_OSAPI_Ipv4Addr_list(addrs, &addrs_len);
  TEST_ASSERT_TRUE(addrs_len > 0);
  TEST_ASSERT_TRUE(addrs_len <= 4);
  for (i = 0; i < addrs_len; i++)
  {
    TEST_ASSERT_TRUE(addrs[i] != 0);
  }
}
#endif /* NANO_FEAT_TRANSPORT_AUTOCFG && \
          NANO_FEAT_TRANSPORT_IP && \
          defined(NANO_HAVE_IFCONFIG) */
#endif /* NANO_FEAT_TRANSPORT_IPV4 */

/******************************************************************************
 * Test entries declaration and runner definitions
 ******************************************************************************/
#if NANO_FEAT_TRANSPORT_IPV4 && NANO_FEAT_TRANSPORT_STRING
#define test_NANO_OSAPI_Ipv4Addr_ENTRIES_STRING \
{\
  RUN_TEST(test_NANO_OSAPI_Ipv4Addr_from_str);\
}
#else
#define test_NANO_OSAPI_Ipv4Addr_ENTRIES_STRING
#endif /* NANO_FEAT_TRANSPORT_IPV4 && NANO_FEAT_TRANSPORT_STRING*/

#if NANO_FEAT_TRANSPORT_IPV4 && \
    NANO_FEAT_TRANSPORT_AUTOCFG && \
    NANO_FEAT_TRANSPORT_IP && \
    defined(NANO_HAVE_IFCONFIG)
#define test_NANO_OSAPI_Ipv4Addr_ENTRIES_AUTOCFG \
{\
  RUN_TEST(test_NANO_OSAPI_Ipv4Addr_list);\
}
#else
#define test_NANO_OSAPI_Ipv4Addr_ENTRIES_AUTOCFG
#endif /* NANO_FEAT_TRANSPORT_IPV4 && \
          NANO_FEAT_TRANSPORT_AUTOCFG && \
          NANO_FEAT_TRANSPORT_IP && \
          defined(NANO_HAVE_IFCONFIG) */

#if NANO_FEAT_TRANSPORT_IPV4
#define test_NANO_OSAPI_Ipv4Addr_ENTRIES \
{\
  RUN_TEST(test_NANO_OSAPI_Ipv4Addr_from_octets);\
  RUN_TEST(test_NANO_OSAPI_Ipv4Addr_is_multicast);\
  RUN_TEST(test_NANO_OSAPI_Ipv4Addr_is_loopback);\
  test_NANO_OSAPI_Ipv4Addr_ENTRIES_STRING \
  test_NANO_OSAPI_Ipv4Addr_ENTRIES_AUTOCFG \
}
#else
#define test_NANO_OSAPI_Ipv4Addr_ENTRIES
#endif /* NANO_FEAT_TRANSPORT_IPV4 */

NANOTEST_RUNNER_FN(test_NANO_OSAPI_Ipv4Addr, test_NANO_OSAPI_Ipv4Addr_ENTRIES)

NANOTEST_RUNNER_MAIN(test_NANO_OSAPI_Ipv4Addr)
