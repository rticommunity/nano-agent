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
#include "TestTime.h"

static void test_NANO_Time_initialize(void)
{
  NANO_RetCode rc = NANO_RETCODE_ERROR;
  /* Initialize a NANO_Time with an already normalized value (i.e. a value where
    the nanoseconds component is < 1s). */
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  rc = NANO_Time_initialize(&t1, 0, 0);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME(0, 0, &t1);

  rc = NANO_Time_initialize(&t1, 1, 2);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME(1, 2, &t1);

  rc = NANO_Time_initialize(&t1, 2147483647, 999999999);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME(2147483647, 999999999, &t1);

  /* Verify that the nanosec component gets automatically normalized */
  rc = NANO_Time_initialize(&t1, 0, 1000000000);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME(1, 0, &t1);

  rc = NANO_Time_initialize(&t1, 2147483646, 1999999999);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME(2147483647, 999999999, &t1);

  /* Verify that overflows are detected and the call fails accordingly */
  rc = NANO_Time_initialize(&t1, 2147483647, 1000000000);
  TEST_ASSERT_RC(NANO_RETCODE_OVERFLOW_DETECTED, rc);

  /* Verify that infinite values are normalized to a common constant */
  rc = NANO_Time_initialize(&t1, -1, 0);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1);

  rc = NANO_Time_initialize(&t1, -1, 1);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1);

  rc = NANO_Time_initialize(&t1, -2, 0);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1);

  rc = NANO_Time_initialize(&t1, -2, 1);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1);

  rc = NANO_Time_initialize(&t1, (-2147483647 - 1), 0xffffffff);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1);
}

static void test_NANO_Time_normalize(void)
{
  NANO_RetCode rc = NANO_RETCODE_ERROR;
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_Time t1_norm = NANO_TIME_INITIALIZER;

  rc = NANO_Time_initialize(&t1, 0, 0);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);

  /* Verify that the nanosec component gets automatically normalized */
  t1.sec = 0;
  t1.nanosec = 1000000000;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME(1, 0, &t1_norm);

  t1.sec = 2147483646;
  t1.nanosec = 1999999999;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME(2147483647, 999999999, &t1_norm);

  /* Verify that overflows are detected and the call fails accordingly */
  t1.sec = 2147483647;
  t1.nanosec = 1000000000;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OVERFLOW_DETECTED, rc);

  /* Verify that infinite values are normalized to a common constant */
  t1.sec = -1;
  t1.nanosec = 0;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1_norm);

  t1.sec = -1;
  t1.nanosec = 1;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1_norm);

  t1.sec = -2;
  t1.nanosec = 0;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1_norm);

  t1.sec = -2;
  t1.nanosec = 1;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1_norm);

  t1.sec = (-2147483647 - 1);
  t1.nanosec = 0xffffffff;
  rc = NANO_Time_normalize(&t1, &t1_norm);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_TIME_INFINITE(&t1_norm);
}

static void test_NANO_Time_to_sec(void)
{
  NANO_RetCode rc = NANO_RETCODE_ERROR;
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_u32 t1_sec = 0;

  /* Infinite values cannot be converted */
  t1.sec = -1;
  t1.nanosec = 0;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_PRECONDITION_NOT_MET, rc);

  t1.sec = (-2147483647 - 1);
  t1.nanosec = 0xffffffff;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_PRECONDITION_NOT_MET, rc);

  /* Values are converted regardless of whether they are normalized or not */
  t1.sec = 0;
  t1.nanosec = 0;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(0, t1_sec);

  t1.sec = 1;
  t1.nanosec = 0;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(1, t1_sec);

  t1.sec = 1;
  t1.nanosec = 1000000000;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(2, t1_sec);

  t1.sec = 2147483647;
  t1.nanosec = 0;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(2147483647, t1_sec);

  t1.sec = 2147483646;
  t1.nanosec = 1000000000;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(2147483647, t1_sec);

  /* nanosec component is rounded up if >= 0.5s */
  t1.sec = 0;
  t1.nanosec = 499999999;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(0, t1_sec);

  t1.sec = 0;
  t1.nanosec = 500000000;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(1, t1_sec);

  t1.sec = 0;
  t1.nanosec = 999999999;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(1, t1_sec);

  t1.sec = 2147483647;
  t1.nanosec = 499999999;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(2147483647, t1_sec);

  t1.sec = 2147483646;
  t1.nanosec = 500000000;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(2147483647, t1_sec);

  t1.sec = 2147483646;
  t1.nanosec = 999999999;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(2147483647, t1_sec);

  /* Overflows are detected and reported only if the value is not normalized */
  t1.sec = 2147483647;
  t1.nanosec = 500000000;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(0x80000000, t1_sec);

  t1.sec = 2147483647;
  t1.nanosec = 999999999;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_EQUAL_UINT32(0x80000000, t1_sec);

  t1.sec = 2147483647;
  t1.nanosec = 1000000000;
  rc = NANO_Time_to_sec(&t1, &t1_sec);
  TEST_ASSERT_RC(NANO_RETCODE_OVERFLOW_DETECTED, rc);
}

static void test_NANO_Time_to_millis(void)
{
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_u64 t1_millis = 0;

  t1.sec = 0;
  t1.nanosec = 0;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(0, t1_millis);

  t1.sec = 1;
  t1.nanosec = 0;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(1000, t1_millis);

  t1.sec = 2147483647;
  t1.nanosec = 0;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(0x1F3FFFFFC18ULL /* 2147483647000 */, t1_millis);

  t1.sec = 0;
  t1.nanosec = 1000000;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(1, t1_millis);

  t1.sec = 0;
  t1.nanosec = 500000;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(1, t1_millis);

  t1.sec = 0;
  t1.nanosec = 499999;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(0, t1_millis);

  t1.sec = 0;
  t1.nanosec = 500000;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(1, t1_millis);

  t1.sec = 2147483647;
  t1.nanosec = 999000000;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(0x1F3FFFFFFFFULL /* 2147483647999 */, t1_millis);

  t1.sec = 2147483647;
  t1.nanosec = 999499999;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(0x1F3FFFFFFFFULL /* 2147483648000 */, t1_millis);

  t1.sec = 2147483647;
  t1.nanosec = 999500000;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(0x1F400000000ULL /* 2147483648000 */, t1_millis);

  t1.sec = 2147483647;
  t1.nanosec = 999999999;
  NANO_Time_to_millis(&t1, &t1_millis);
  TEST_ASSERT_EQUAL_UINT64(0x1F400000000ULL /* 2147483648000 */, t1_millis);
}

void test_NANO_Time_compare(void)
{
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_Time t2 = NANO_TIME_INITIALIZER;
  NANO_Time t1_inf = NANO_TIME_INITIALIZER_INFINITE;
  NANO_Time t2_inf = NANO_TIME_INITIALIZER_INFINITE;

  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t1, &t2));
  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t2, &t1));
  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t1_inf, &t2_inf));
  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t2_inf, &t1_inf));
  TEST_ASSERT_EQUAL_INT8(-1, NANO_Time_compare(&t1, &t1_inf));
  TEST_ASSERT_EQUAL_INT8(1, NANO_Time_compare(&t1_inf, &t1));
  t2_inf.sec = -2;
  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t1_inf, &t2_inf));
  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t2_inf, &t1_inf));
  t1_inf.nanosec = 1;
  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t1_inf, &t2_inf));
  TEST_ASSERT_EQUAL_INT8(0, NANO_Time_compare(&t2_inf, &t1_inf));
  t2.sec = 1;
  TEST_ASSERT_EQUAL_INT8(-1, NANO_Time_compare(&t1, &t2));
  TEST_ASSERT_EQUAL_INT8(1, NANO_Time_compare(&t2, &t1));
  t1.sec = 1;
  t1.nanosec = 1;
  TEST_ASSERT_EQUAL_INT8(1, NANO_Time_compare(&t1, &t2));
  TEST_ASSERT_EQUAL_INT8(-1, NANO_Time_compare(&t2, &t1));
  t1.sec = 0;
  t2.sec = 0;
  TEST_ASSERT_EQUAL_INT8(1, NANO_Time_compare(&t1, &t2));
  TEST_ASSERT_EQUAL_INT8(-1, NANO_Time_compare(&t2, &t1));
}

void test_NANO_Time_is_equal(void)
{
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_Time t2 = NANO_TIME_INITIALIZER;
  NANO_Time t1_inf = NANO_TIME_INITIALIZER_INFINITE;
  NANO_Time t2_inf = NANO_TIME_INITIALIZER_INFINITE;

  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t1, &t2));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t2, &t1));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t1_inf, &t2_inf));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t2_inf, &t1_inf));
  TEST_ASSERT_FALSE(NANO_Time_is_equal(&t1, &t1_inf));
  TEST_ASSERT_FALSE(NANO_Time_is_equal(&t1_inf, &t1));
  t2_inf.sec = -2;
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t1_inf, &t2_inf));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t2_inf, &t1_inf));
  t1_inf.nanosec = 1;
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t1_inf, &t2_inf));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t2_inf, &t1_inf));
  t2.sec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_equal(&t1, &t2));
  TEST_ASSERT_FALSE(NANO_Time_is_equal(&t2, &t1));
  t1.sec = 1;
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t1, &t2));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t2, &t1));
  t1.nanosec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_equal(&t1, &t2));
  TEST_ASSERT_FALSE(NANO_Time_is_equal(&t2, &t1));
  t2.nanosec = 1;
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t1, &t2));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t2, &t1));
  t1.sec = 0;
  t2.sec = 0;
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t1, &t2));
  TEST_ASSERT_TRUE(NANO_Time_is_equal(&t2, &t1));
}

void test_NANO_Time_is_normalized(void)
{
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_Time t1_inf = NANO_TIME_INITIALIZER_INFINITE;

  TEST_ASSERT_TRUE(NANO_Time_is_normalized(&t1));
  TEST_ASSERT_TRUE(NANO_Time_is_normalized(&t1_inf));

  t1.sec = 1;
  TEST_ASSERT_TRUE(NANO_Time_is_normalized(&t1));

  t1.nanosec = 1;
  TEST_ASSERT_TRUE(NANO_Time_is_normalized(&t1));

  t1.nanosec = 999999999;
  TEST_ASSERT_TRUE(NANO_Time_is_normalized(&t1));

  t1.nanosec = 1000000000;
  TEST_ASSERT_FALSE(NANO_Time_is_normalized(&t1));

  t1_inf.sec = -2;
  TEST_ASSERT_FALSE(NANO_Time_is_normalized(&t1_inf));

  t1_inf.sec = -1;
  t1_inf.nanosec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_normalized(&t1_inf));
}

void test_NANO_Time_is_infinite(void)
{
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_Time t1_inf = NANO_TIME_INITIALIZER_INFINITE;

  TEST_ASSERT_FALSE(NANO_Time_is_infinite(&t1));
  TEST_ASSERT_TRUE(NANO_Time_is_infinite(&t1_inf));

  t1.sec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_infinite(&t1));

  t1.sec = 0;
  t1.nanosec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_infinite(&t1));

  t1.sec = -1;
  t1.nanosec = 0;
  TEST_ASSERT_TRUE(NANO_Time_is_infinite(&t1));

  t1_inf.sec = -2;
  t1_inf.nanosec = 0;
  TEST_ASSERT_TRUE(NANO_Time_is_infinite(&t1_inf));

  t1_inf.sec = -2;
  t1_inf.nanosec = 1;
  TEST_ASSERT_TRUE(NANO_Time_is_infinite(&t1_inf));
}

void test_NANO_Time_is_zero(void)
{
  NANO_Time t1 = NANO_TIME_INITIALIZER;
  NANO_Time t1_inf = NANO_TIME_INITIALIZER_INFINITE;

  TEST_ASSERT_TRUE(NANO_Time_is_zero(&t1));
  TEST_ASSERT_FALSE(NANO_Time_is_zero(&t1_inf));

  t1.sec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_zero(&t1));

  t1.sec = 0;
  t1.nanosec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_zero(&t1));

  t1_inf.sec = -2;
  t1_inf.nanosec = 0;
  TEST_ASSERT_FALSE(NANO_Time_is_zero(&t1_inf));

  t1_inf.sec = -2;
  t1_inf.nanosec = 1;
  TEST_ASSERT_FALSE(NANO_Time_is_zero(&t1_inf));
}

void test_NANO_Time_from_millis(void)
{
  NANO_Time t1 = NANO_TIME_INITIALIZER;

  NANO_Time_from_millis(&t1, 1000);
  TEST_ASSERT_EQUAL_INT32(1, t1.sec);
  TEST_ASSERT_EQUAL_UINT32(0, t1.nanosec);

  NANO_Time_from_millis(&t1, 1999);
  TEST_ASSERT_EQUAL_INT32(1, t1.sec);
  TEST_ASSERT_EQUAL_UINT32(999000000, t1.nanosec);

  NANO_Time_from_millis(&t1, 2147483647999);
  TEST_ASSERT_TRUE(NANO_Time_is_normalized(&t1));
  TEST_ASSERT_EQUAL_INT32(2147483647, t1.sec);
  TEST_ASSERT_EQUAL_UINT32(999000000, t1.nanosec);

  NANO_Time_from_millis(&t1, 0);
  TEST_ASSERT_TRUE(NANO_Time_is_normalized(&t1));
  TEST_ASSERT_EQUAL_INT32(0, t1.sec);
  TEST_ASSERT_EQUAL_UINT32(0, t1.nanosec);
}

NANOTEST_RUNNER_FN(test_NANO_Time,
{
  RUN_TEST(test_NANO_Time_initialize);
  RUN_TEST(test_NANO_Time_normalize);
  RUN_TEST(test_NANO_Time_to_sec);
  RUN_TEST(test_NANO_Time_to_millis);
  RUN_TEST(test_NANO_Time_compare);
  RUN_TEST(test_NANO_Time_is_equal);
  RUN_TEST(test_NANO_Time_is_normalized);
  RUN_TEST(test_NANO_Time_is_infinite);
  RUN_TEST(test_NANO_Time_is_zero);
  RUN_TEST(test_NANO_Time_from_millis);
})

NANOTEST_RUNNER_MAIN(test_NANO_Time)
