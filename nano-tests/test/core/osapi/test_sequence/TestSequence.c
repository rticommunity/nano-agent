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
#include "TestSequence.h"

struct SeqElement {
  NANO_u32 a;
  NANO_u32 b;
  NANO_u32 c;
  NANO_u32 d;
};

static struct SeqElement fv_sequence_buffer_obj[4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

static NANO_u8 fv_sequence_buffer_prim[4] = {0,0,0,0};

static void test_NANO_Sequence_initialize(void)
{
  NANO_Sequence seq = NANO_SEQUENCE_INITIALIZER;

  TEST_ASSERT_FALSE(NANO_Sequence_initialized(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_element_size(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_length(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_maximum(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_serialized_size(&seq));
  TEST_ASSERT_FALSE(NANO_Sequence_flags(&seq, NANO_SEQUENCEFLAGS_SERIALIZED));
  TEST_ASSERT_FALSE(NANO_Sequence_flags(&seq, NANO_SEQUENCEFLAGS_ENDIANNESS));
  TEST_ASSERT_NULL(NANO_Sequence_contiguous_buffer(&seq));

  NANO_Sequence_initialize(&seq, sizeof(struct SeqElement));
  TEST_ASSERT_TRUE(NANO_Sequence_initialized(&seq));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(struct SeqElement), NANO_Sequence_element_size(&seq));
}

static void test_NANO_Sequence_contiguous_buffer_primitive(void)
{
  NANO_Sequence seq = NANO_SEQUENCE_INITIALIZER;

  NANO_Sequence_initialize(&seq, sizeof(NANO_u8));
  TEST_ASSERT_TRUE(NANO_Sequence_initialized(&seq));
  TEST_ASSERT_EQUAL_UINT64(1, NANO_Sequence_element_size(&seq));
  NANO_Sequence_set_contiguous_buffer(&seq,
    fv_sequence_buffer_prim,
    sizeof(fv_sequence_buffer_prim),
    sizeof(fv_sequence_buffer_prim));
  TEST_ASSERT_EQUAL_UINT64(sizeof(fv_sequence_buffer_prim), NANO_Sequence_length(&seq));
  TEST_ASSERT_EQUAL_UINT64(sizeof(fv_sequence_buffer_prim), NANO_Sequence_maximum(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_serialized_size(&seq));
  TEST_ASSERT_TRUE(
    fv_sequence_buffer_prim == NANO_Sequence_contiguous_buffer(&seq));

  TEST_ASSERT_TRUE(
    fv_sequence_buffer_prim == NANO_Sequence_reference(&seq, 0));
  TEST_ASSERT_TRUE(
    fv_sequence_buffer_prim + 1 == NANO_Sequence_reference(&seq, 1));
  TEST_ASSERT_TRUE(
    fv_sequence_buffer_prim + 2 == NANO_Sequence_reference(&seq, 2));
  TEST_ASSERT_TRUE(
    fv_sequence_buffer_prim + 3 == NANO_Sequence_reference(&seq, 3));
  TEST_ASSERT_NULL(NANO_Sequence_reference(&seq, 4));
}

static void test_NANO_Sequence_contiguous_buffer_obj(void)
{
  NANO_Sequence seq = NANO_SEQUENCE_INITIALIZER;

  NANO_Sequence_initialize(&seq, sizeof(struct SeqElement));
  TEST_ASSERT_TRUE(NANO_Sequence_initialized(&seq));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(struct SeqElement), NANO_Sequence_element_size(&seq));
  NANO_Sequence_set_contiguous_buffer(&seq,
    fv_sequence_buffer_obj,
    sizeof(fv_sequence_buffer_obj)/sizeof(struct SeqElement),
    sizeof(fv_sequence_buffer_obj)/sizeof(struct SeqElement));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(fv_sequence_buffer_obj)/sizeof(struct SeqElement),
    NANO_Sequence_length(&seq));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(fv_sequence_buffer_obj)/sizeof(struct SeqElement),
    NANO_Sequence_maximum(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_serialized_size(&seq));
  TEST_ASSERT_TRUE(
    (NANO_u8*)fv_sequence_buffer_obj == NANO_Sequence_contiguous_buffer(&seq));

  TEST_ASSERT_TRUE(
    fv_sequence_buffer_obj == (struct SeqElement*)NANO_Sequence_reference(&seq, 0));
  TEST_ASSERT_TRUE(
    fv_sequence_buffer_obj + 1 == (struct SeqElement*)NANO_Sequence_reference(&seq, 1));
  TEST_ASSERT_TRUE(
    fv_sequence_buffer_obj + 2 == (struct SeqElement*)NANO_Sequence_reference(&seq, 2));
  TEST_ASSERT_TRUE(
    fv_sequence_buffer_obj + 3 == (struct SeqElement*)NANO_Sequence_reference(&seq, 3));
  TEST_ASSERT_NULL(NANO_Sequence_reference(&seq, 4));
}

static void test_NANO_Sequence_serialized_buffer(void)
{
  NANO_Sequence seq = NANO_SEQUENCE_INITIALIZER;

  NANO_Sequence_initialize(&seq, sizeof(NANO_u8));
  TEST_ASSERT_TRUE(NANO_Sequence_initialized(&seq));
  TEST_ASSERT_EQUAL_UINT64(1, NANO_Sequence_element_size(&seq));
  NANO_Sequence_set_serialized_buffer(&seq,
    fv_sequence_buffer_prim,
    sizeof(fv_sequence_buffer_prim),
    sizeof(fv_sequence_buffer_prim),
    NANO_BOOL_TRUE /* little_endian */);

  TEST_ASSERT_EQUAL_UINT64(
    sizeof(fv_sequence_buffer_prim), NANO_Sequence_length(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_maximum(&seq));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(fv_sequence_buffer_prim), NANO_Sequence_serialized_size(&seq));
  TEST_ASSERT_NULL(NANO_Sequence_contiguous_buffer(&seq));

  TEST_ASSERT_NULL(NANO_Sequence_reference(&seq, 0));
  TEST_ASSERT_NULL(NANO_Sequence_reference(&seq, 1));
  TEST_ASSERT_NULL(NANO_Sequence_reference(&seq, 2));
  TEST_ASSERT_NULL(NANO_Sequence_reference(&seq, 3));
}

NANOTEST_RUNNER_FN(test_NANO_Sequence,
{
  RUN_TEST(test_NANO_Sequence_initialize);
  RUN_TEST(test_NANO_Sequence_contiguous_buffer_primitive);
  RUN_TEST(test_NANO_Sequence_contiguous_buffer_obj);
  RUN_TEST(test_NANO_Sequence_serialized_buffer);
})

NANOTEST_RUNNER_MAIN(test_NANO_Sequence)
