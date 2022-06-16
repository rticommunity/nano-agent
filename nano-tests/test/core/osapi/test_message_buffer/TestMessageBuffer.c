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
#include "TestMessageBuffer.h"

static void test_NANO_MessageBuffer_initialize_empty(void)
{
  NANO_u32 mbuf_len = 0;
  NANO_MessageBuffer mbuf = NANO_MESSAGEBUFFER_INITIALIZER;

  TEST_ASSERT_NULL(mbuf.data);

  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_data_offset(&mbuf));
  NANO_MessageBuffer_data_len_msg(&mbuf, &mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(0, mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(0, NANO_MessageBuffer_data_len(&mbuf));

  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_inline_data_head_ptr(&mbuf) == (NANO_u8*)&mbuf.data);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_inline_data_ptr(&mbuf) == (NANO_u8*)&mbuf.data);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_head_ptr(&mbuf) == mbuf.data);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_ptr(&mbuf) == mbuf.data);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next(&mbuf));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf));
}

static void test_NANO_MessageBuffer_initialize_external(void)
{
#define MBUF_SIZE   16
  NANO_u32 mbuf_len = 0;
  NANO_MessageBuffer mbuf = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_u8 mbuf_extdata[MBUF_SIZE];

  TEST_ASSERT_NULL(mbuf.data);
  TEST_ASSERT_FALSE(NANO_MessageBuffer_flags_inline(&mbuf));
  NANO_MessageBuffer_set_external_data(&mbuf, mbuf_extdata, MBUF_SIZE);
  TEST_ASSERT_FALSE(NANO_MessageBuffer_flags_inline(&mbuf));

  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_data_offset(&mbuf));
  NANO_MessageBuffer_data_len_msg(&mbuf, &mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, NANO_MessageBuffer_data_len(&mbuf));

  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_head_ptr(&mbuf) == mbuf_extdata);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_ptr(&mbuf) == mbuf_extdata);
#undef MBUF_SIZE
}

static void test_NANO_MessageBuffer_initialize_inline(void)
{
#define MBUF_SIZE   16
  NANO_u32 mbuf_len = 0;
  NANO_MessageBufferData mbuf_inline[NANO_MESSAGEBUFFER_INLINE_SIZE(MBUF_SIZE)] = { 0 };
  NANO_MessageBuffer * mbuf = (NANO_MessageBuffer *)&mbuf_inline;

  TEST_ASSERT_FALSE(NANO_MessageBuffer_flags_inline(mbuf));
  NANO_MessageBuffer_flags_set_inline(mbuf);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_flags_inline(mbuf));

  NANO_MessageBuffer_set_data_len(mbuf, MBUF_SIZE);
  NANO_MessageBuffer_data_len_msg(mbuf, &mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, NANO_MessageBuffer_data_len(mbuf));

  TEST_ASSERT_TRUE(
    ((NANO_u8*)&mbuf_inline + NANO_OSAPI_MEMBER_OFFSET(NANO_MessageBuffer,data)) ==
      NANO_MessageBuffer_inline_data_head_ptr(mbuf));
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_inline_data_head_ptr(mbuf) ==
      NANO_MessageBuffer_inline_data_ptr(mbuf));

  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_head_ptr(mbuf) ==
      NANO_MessageBuffer_inline_data_head_ptr(mbuf));
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_ptr(mbuf) ==
      NANO_MessageBuffer_inline_data_ptr(mbuf));
#undef MBUF_SIZE
}

static void test_NANO_MessageBuffer_flags(void)
{
  NANO_MessageBuffer mbuf = NANO_MESSAGEBUFFER_INITIALIZER;
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_INLINE));
  TEST_ASSERT_FALSE(NANO_MessageBuffer_flags_inline(&mbuf));
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_USER_0));
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_USER_1));
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_USER_2));
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_USER_3));
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_USER_4));
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_USER_5));
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_USER_6));

  /* Check that we can set/reset/set the "userdata" field */
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA));
  TEST_ASSERT_FALSE(NANO_MessageBuffer_flags_has_userdata(&mbuf));
  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_userdata(&mbuf));
  NANO_MessageBuffer_set_userdata(&mbuf, 0xE0E0);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_flags_has_userdata(&mbuf));
  TEST_ASSERT_EQUAL_UINT16(0xE0E0, NANO_MessageBuffer_userdata(&mbuf));
  /* userdata can also be reset by resetting the flag.
     This doesn't clear the data though */
  NANO_MessageBuffer_flags_unset(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA);
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA));
  TEST_ASSERT_FALSE(NANO_MessageBuffer_flags_has_userdata(&mbuf));
  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_userdata(&mbuf));
  NANO_MessageBuffer_flags_set(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_flags_has_userdata(&mbuf));
  TEST_ASSERT_EQUAL_UINT16(0xE0E0, NANO_MessageBuffer_userdata(&mbuf));
  /* reset_userdata() also clears the field */
  NANO_MessageBuffer_reset_userdata(&mbuf);
  TEST_ASSERT_FALSE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA));
  TEST_ASSERT_FALSE(NANO_MessageBuffer_flags_has_userdata(&mbuf));
  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_userdata(&mbuf));
  NANO_MessageBuffer_flags_set(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_flags_has_userdata(&mbuf));
  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_userdata(&mbuf));
  NANO_MessageBuffer_set_userdata(&mbuf, 0xAAAA);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_flags(&mbuf, NANO_MESSAGEBUFFERFLAGS_HAS_USERDATA));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_flags_has_userdata(&mbuf));
  TEST_ASSERT_EQUAL_UINT16(0xAAAA, NANO_MessageBuffer_userdata(&mbuf));

  /* Check that we can set the type flag */
  TEST_ASSERT_EQUAL_UINT8(
    NANO_MESSAGEBUFFERTYPE_DEFAULT, NANO_MessageBuffer_type(&mbuf));
  NANO_MessageBuffer_set_type(&mbuf, NANO_MESSAGEBUFFERTYPE_USER_0);
  TEST_ASSERT_EQUAL_UINT8(
    NANO_MESSAGEBUFFERTYPE_USER_0, NANO_MessageBuffer_type(&mbuf));
  NANO_MessageBuffer_set_type(&mbuf, NANO_MESSAGEBUFFERTYPE_USER_1);
  TEST_ASSERT_EQUAL_UINT8(
    NANO_MESSAGEBUFFERTYPE_USER_1, NANO_MessageBuffer_type(&mbuf));

  /* Check that we can set other individual flags */
#define test_set_unset_flag(f_) \
{\
  TEST_ASSERT_FALSE(\
    NANO_MessageBuffer_flags(&mbuf, f_));\
  NANO_MessageBuffer_flags_set(&mbuf, f_);\
  TEST_ASSERT_TRUE(\
    NANO_MessageBuffer_flags(&mbuf, f_));\
  NANO_MessageBuffer_flags_unset(&mbuf, f_);\
  TEST_ASSERT_FALSE(\
    NANO_MessageBuffer_flags(&mbuf, f_));\
}

  test_set_unset_flag(NANO_MESSAGEBUFFERFLAGS_USER_0);
  test_set_unset_flag(NANO_MESSAGEBUFFERFLAGS_USER_1);
  test_set_unset_flag(NANO_MESSAGEBUFFERFLAGS_USER_2);
  test_set_unset_flag(NANO_MESSAGEBUFFERFLAGS_USER_3);
  test_set_unset_flag(NANO_MESSAGEBUFFERFLAGS_USER_4);
  test_set_unset_flag(NANO_MESSAGEBUFFERFLAGS_USER_5);
  test_set_unset_flag(NANO_MESSAGEBUFFERFLAGS_USER_6);
}

static void test_NANO_MessageBuffer_data_offset(void)
{
#define MBUF_SIZE   16
  NANO_u32 mbuf_len = 0;
  NANO_MessageBufferData mbuf_inline[NANO_MESSAGEBUFFER_INLINE_SIZE(MBUF_SIZE)] = { 0 };
  NANO_u8 mbuf_extdata[MBUF_SIZE];
  NANO_MessageBuffer * mbuf = (NANO_MessageBuffer *)&mbuf_inline;

  NANO_MessageBuffer_flags_set_inline(mbuf);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_flags_inline(mbuf));
  NANO_MessageBuffer_set_data_len(mbuf, MBUF_SIZE);
  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_data_offset(mbuf));
  NANO_MessageBuffer_set_data_offset(mbuf, MBUF_SIZE / 2);
  TEST_ASSERT_EQUAL_UINT16(MBUF_SIZE / 2, NANO_MessageBuffer_data_offset(mbuf));

  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_head_ptr(mbuf) ==
      NANO_MessageBuffer_inline_data_head_ptr(mbuf));
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_ptr(mbuf) ==
      (NANO_MessageBuffer_inline_data_head_ptr(mbuf) +
        NANO_MessageBuffer_data_offset(mbuf)));

  NANO_MessageBuffer_data_len_msg(mbuf, &mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, NANO_MessageBuffer_data_len(mbuf));

  NANO_MessageBuffer_set_external_data(mbuf, mbuf_extdata, MBUF_SIZE);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_head_ptr(mbuf) == mbuf_extdata);
  TEST_ASSERT_TRUE(
    NANO_MessageBuffer_data_ptr(mbuf) ==
      mbuf_extdata + NANO_MessageBuffer_data_offset(mbuf));

  NANO_MessageBuffer_data_len_msg(mbuf, &mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, NANO_MessageBuffer_data_len(mbuf));

  NANO_MessageBuffer_set_data_offset(mbuf, 0);
  TEST_ASSERT_EQUAL_UINT16(0, NANO_MessageBuffer_data_offset(mbuf));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_data_ptr(mbuf) == mbuf_extdata);

  NANO_MessageBuffer_data_len_msg(mbuf, &mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, mbuf_len);
  TEST_ASSERT_EQUAL_UINT32(MBUF_SIZE, NANO_MessageBuffer_data_len(mbuf));
#undef MBUF_SIZE
}

void test_NANO_MessageBuffer_chain(void)
{
  NANO_MessageBuffer mbuf1 = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer mbuf2 = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer mbuf3 = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer mbuf4 = NANO_MESSAGEBUFFER_INITIALIZER;

  TEST_ASSERT_NULL(NANO_MessageBuffer_next(&mbuf1));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));
  NANO_MessageBuffer_append(&mbuf1, &mbuf2);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next(&mbuf1) == &mbuf2);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next(&mbuf2));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf2));

  NANO_MessageBuffer_append(&mbuf1, &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next(&mbuf1) == &mbuf3);

  NANO_MessageBuffer_append_msg(&mbuf1, &mbuf2);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next(&mbuf1) == &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf1) == &mbuf2);

  NANO_MessageBuffer_unlink(&mbuf1);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next(&mbuf1));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf1) == &mbuf2);

  NANO_MessageBuffer_append(&mbuf2, &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next(&mbuf2) == &mbuf3);

  NANO_MessageBuffer_unlink_msg(&mbuf1);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));
  NANO_MessageBuffer_append(&mbuf1, &mbuf4);
  NANO_MessageBuffer_append_msg(&mbuf2, &mbuf1);

  TEST_ASSERT_TRUE(NANO_MessageBuffer_next(&mbuf1) == &mbuf4);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf2) == &mbuf1);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf4));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf3));
}

NANOTEST_RUNNER_FN(test_NANO_MessageBuffer,
{
  RUN_TEST(test_NANO_MessageBuffer_initialize_empty);
  RUN_TEST(test_NANO_MessageBuffer_initialize_external);
  RUN_TEST(test_NANO_MessageBuffer_initialize_inline);
  RUN_TEST(test_NANO_MessageBuffer_flags);
  RUN_TEST(test_NANO_MessageBuffer_data_offset);
  RUN_TEST(test_NANO_MessageBuffer_chain);
})

NANOTEST_RUNNER_MAIN(test_NANO_MessageBuffer)
