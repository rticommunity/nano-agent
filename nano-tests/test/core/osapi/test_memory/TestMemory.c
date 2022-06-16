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
#include <stdint.h>

#include "nano/nano_core_test.h"
#include "TestMemory.h"

#if UINTPTR_MAX == 0xffffffff
/* 32-bit */
#define PTR_INT   NANO_u32
#elif UINTPTR_MAX == 0xffffffffffffffff
/* 64-bit */
#define PTR_INT   NANO_u64
#else
#error "failed to detect pointer size for architecture"
#endif

static void test_NANO_OSAPI_Memory_API(void)
{
  NANO_u8
    b0[32] = {0},
    b1[32] = {0};
  const NANO_u8
    cmp1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    cmp2[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  NANO_i32 cmp_res = 0;
  NANO_u8 * ptr = NULL;

  NANO_OSAPI_Memory_set(b0 + (sizeof(b0) / 2), 1, sizeof(b0) / 2);
  cmp_res = NANO_OSAPI_Memory_compare(b0, cmp1, sizeof(b0) / 2);
  TEST_ASSERT_TRUE(cmp_res == 0);
  cmp_res = NANO_OSAPI_Memory_compare(b0 + (sizeof(b0)/2), cmp2, sizeof(b0) / 2);
  TEST_ASSERT_TRUE(cmp_res == 0);
  cmp_res = NANO_OSAPI_Memory_compare(b0, b1, sizeof(b0));
  TEST_ASSERT_TRUE(cmp_res > 0);
  cmp_res = NANO_OSAPI_Memory_compare(b1, b0, sizeof(b0));
  TEST_ASSERT_TRUE(cmp_res < 0);
  NANO_OSAPI_Memory_copy(b1, b0, sizeof(b0));
  cmp_res = NANO_OSAPI_Memory_compare(b0, b1, sizeof(b0));
  TEST_ASSERT_TRUE(cmp_res == 0);
  cmp_res = NANO_OSAPI_Memory_compare(b1, b0, sizeof(b0));
  TEST_ASSERT_TRUE(cmp_res == 0);
  ptr = NANO_OSAPI_Memory_move(b0, b0 + (sizeof(b0)/2), sizeof(b0)/2);
  TEST_ASSERT_TRUE(ptr == (NANO_u8*)b0);
  NANO_OSAPI_Memory_zero(b0 + (sizeof(b0)/2), sizeof(b0)/2);
  cmp_res = NANO_OSAPI_Memory_compare(b0, cmp2, sizeof(b0) / 2);
  TEST_ASSERT_TRUE(cmp_res == 0);
  cmp_res = NANO_OSAPI_Memory_compare(b0 + (sizeof(b0)/2), cmp1, sizeof(b0) / 2);
  TEST_ASSERT_TRUE(cmp_res == 0);
}

static void test_NANO_OSAPI_Memory_align(void)
{
  NANO_u8 * ptr = (NANO_u8*)1;
  TEST_ASSERT_EQUAL_UINT64(2, NANO_OSAPI_Memory_align_size_up(1, 2));
  TEST_ASSERT_EQUAL_UINT64(2, NANO_OSAPI_Memory_align_size_up(2, 2));
  TEST_ASSERT_EQUAL_UINT64(4, NANO_OSAPI_Memory_align_size_up(3, 2));
  TEST_ASSERT_EQUAL_UINT64(4, NANO_OSAPI_Memory_align_size_up(4, 2));

  TEST_ASSERT_EQUAL_UINT64(4, NANO_OSAPI_Memory_align_size_up(1, 4));
  TEST_ASSERT_EQUAL_UINT64(4, NANO_OSAPI_Memory_align_size_up(2, 4));
  TEST_ASSERT_EQUAL_UINT64(4, NANO_OSAPI_Memory_align_size_up(3, 4));
  TEST_ASSERT_EQUAL_UINT64(4, NANO_OSAPI_Memory_align_size_up(4, 4));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(5, 4));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(6, 4));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(7, 4));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(8, 4));

  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(1, 8));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(2, 8));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(3, 8));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(4, 8));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(5, 8));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(6, 8));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(7, 8));
  TEST_ASSERT_EQUAL_UINT64(8, NANO_OSAPI_Memory_align_size_up(8, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(9, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(10, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(11, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(12, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(13, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(14, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(15, 8));
  TEST_ASSERT_EQUAL_UINT64(16, NANO_OSAPI_Memory_align_size_up(16, 8));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_EQUAL_HEX64(0x02, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u16));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_EQUAL_HEX64(0x02, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u16));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u16));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u16));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));
  ptr = (NANO_u8*)0x05;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));
  ptr = (NANO_u8*)0x06;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));
  ptr = (NANO_u8*)0x07;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));
  ptr = (NANO_u8*)0x08;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u32));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x05;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x06;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x07;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x08;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x09;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x0A;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x0B;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x0C;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x0D;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x0E;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x0F;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));
  ptr = (NANO_u8*)0x10;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align(ptr, NANO_u64));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_EQUAL_HEX64(0x02, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 2));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_EQUAL_HEX64(0x02, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 2));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 2));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 2));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_EQUAL_HEX64(0x04, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));
  ptr = (NANO_u8*)0x05;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));
  ptr = (NANO_u8*)0x06;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));
  ptr = (NANO_u8*)0x07;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));
  ptr = (NANO_u8*)0x08;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 4));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x05;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x06;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x07;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x08;
  TEST_ASSERT_EQUAL_HEX64(0x08, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x09;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x0A;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x0B;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x0C;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x0D;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x0E;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x0F;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));
  ptr = (NANO_u8*)0x10;
  TEST_ASSERT_EQUAL_HEX64(0x10, (PTR_INT)NANO_OSAPI_Memory_align_ptr(ptr, 8));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 2));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_TRUE(NANO_OSAPI_Memory_aligned(ptr, 2));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 2));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_TRUE(NANO_OSAPI_Memory_aligned(ptr, 2));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 4));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 4));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 4));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_TRUE(NANO_OSAPI_Memory_aligned(ptr, 4));
  ptr = (NANO_u8*)0x05;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 4));
  ptr = (NANO_u8*)0x06;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 4));
  ptr = (NANO_u8*)0x07;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 4));
  ptr = (NANO_u8*)0x08;
  TEST_ASSERT_TRUE(NANO_OSAPI_Memory_aligned(ptr, 4));

  ptr = (NANO_u8*)0x01;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x02;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x03;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x04;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x05;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x06;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x07;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x08;
  TEST_ASSERT_TRUE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x09;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x0A;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x0B;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x0C;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x0D;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x0E;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x0F;
  TEST_ASSERT_FALSE(NANO_OSAPI_Memory_aligned(ptr, 8));
  ptr = (NANO_u8*)0x10;
  TEST_ASSERT_TRUE(NANO_OSAPI_Memory_aligned(ptr, 8));
}

NANOTEST_RUNNER_FN(test_NANO_OSAPI_Memory,
{
  RUN_TEST(test_NANO_OSAPI_Memory_API);
  RUN_TEST(test_NANO_OSAPI_Memory_align);
})

NANOTEST_RUNNER_MAIN(test_NANO_OSAPI_Memory)
