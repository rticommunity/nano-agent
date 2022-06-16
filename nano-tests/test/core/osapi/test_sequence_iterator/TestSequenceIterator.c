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
#include "TestSequenceIterator.h"

struct SeqElement {
  NANO_u32 a;
  NANO_u32 b;
  NANO_u32 c;
  NANO_u32 d;
};

static struct SeqElement fv_sequence_buffer_obj[4] = {
  {1, 1, 1, 1},
  {2, 2, 2, 2},
  {3, 3, 3, 3},
  {4, 4, 4, 4}
};

static NANO_usize fv_sequence_iter_call_count = 0;

static NANO_RetCode test_NANO_SequenceIterator_deserialize_DeserializeSeqElement(
  NANO_SequenceIterator *const self,
  const NANO_Sequence *const seq,
  void *const next_el)
{
  struct SeqElement * seq_el = (struct SeqElement*)next_el;
  fv_sequence_iter_call_count += 1;

  TEST_ASSERT_TRUE(
    NANO_SequenceIterator_check_capacity_align(
      self, sizeof(struct SeqElement), sizeof(NANO_u32)));
  NANO_OSAPI_Memory_copy(
    seq_el, NANO_SequenceIterator_head(self), sizeof(struct SeqElement));
  NANO_SequenceIterator_consume(self, sizeof(struct SeqElement));

  return NANO_RETCODE_OK;
}

static void test_NANO_SequenceIterator_API(void)
{
  NANO_SequenceIterator seq_iter = NANO_SEQUENCEITERATOR_INITIALIZER;
  NANO_Sequence seq = NANO_SEQUENCE_INITIALIZER;
  NANO_RetCode rc = NANO_RETCODE_ERROR;
  struct SeqElement next = {0,0,0,0};
  NANO_bool has_next = NANO_BOOL_FALSE;
  NANO_usize i = 0;

  NANO_Sequence_initialize(&seq, sizeof(struct SeqElement));
  TEST_ASSERT_TRUE(NANO_Sequence_initialized(&seq));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(struct SeqElement), NANO_Sequence_element_size(&seq));
  NANO_Sequence_set_serialized_buffer(&seq,
    fv_sequence_buffer_obj,
    sizeof(fv_sequence_buffer_obj),
    sizeof(fv_sequence_buffer_obj)/sizeof(struct SeqElement),
    NANO_BOOL_TRUE /* little_endian */);

  TEST_ASSERT_EQUAL_UINT64(
    sizeof(fv_sequence_buffer_obj)/sizeof(struct SeqElement),
    NANO_Sequence_length(&seq));
  TEST_ASSERT_EQUAL_UINT64(0, NANO_Sequence_maximum(&seq));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(fv_sequence_buffer_obj), NANO_Sequence_serialized_size(&seq));
  TEST_ASSERT_NULL(NANO_Sequence_contiguous_buffer(&seq));
  TEST_ASSERT_EQUAL_UINT64(
    0, NANO_SequenceIterator_deserialized_size(&seq_iter));
  TEST_ASSERT_EQUAL_UINT64(
    0, NANO_SequenceIterator_remaining(&seq_iter));
  NANO_Sequence_to_iterator(&seq,
    test_NANO_SequenceIterator_deserialize_DeserializeSeqElement,
    &seq_iter);
  TEST_ASSERT_EQUAL_UINT64(
    0, NANO_SequenceIterator_deserialized_size(&seq_iter));
  TEST_ASSERT_EQUAL_UINT64(
    sizeof(fv_sequence_buffer_obj), NANO_SequenceIterator_remaining(&seq_iter));
  TEST_ASSERT_TRUE(
    NANO_SequenceIterator_check_capacity(
      &seq_iter, sizeof(fv_sequence_buffer_obj) - 1));
  TEST_ASSERT_TRUE(
    NANO_SequenceIterator_check_capacity(
      &seq_iter, sizeof(fv_sequence_buffer_obj)));
  TEST_ASSERT_FALSE(
    NANO_SequenceIterator_check_capacity(
      &seq_iter, sizeof(fv_sequence_buffer_obj) + 1));

  for (fv_sequence_iter_call_count = 0, i = 0; i < 4; i++)
  {
    rc = NANO_SequenceIterator_next(&seq_iter, &next, &has_next);
    TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
    TEST_ASSERT_TRUE(has_next);
    TEST_ASSERT_EQUAL_UINT64(
      sizeof(struct SeqElement) * (i+1),
      NANO_SequenceIterator_deserialized_size(&seq_iter));
    TEST_ASSERT_EQUAL_UINT64(
      sizeof(fv_sequence_buffer_obj) - (sizeof(struct SeqElement) * (i+1)),
      NANO_SequenceIterator_remaining(&seq_iter));
    TEST_ASSERT_EQUAL_UINT32((i+1), fv_sequence_iter_call_count);
    TEST_ASSERT_EQUAL_UINT32((i+1), next.a);
    TEST_ASSERT_EQUAL_UINT32((i+1), next.b);
    TEST_ASSERT_EQUAL_UINT32((i+1), next.c);
    TEST_ASSERT_EQUAL_UINT32((i+1), next.d);
  }

  rc = NANO_SequenceIterator_next(&seq_iter, &next, &has_next);
  TEST_ASSERT_RC(NANO_RETCODE_OK, rc);
  TEST_ASSERT_FALSE(has_next);
}

NANOTEST_RUNNER_FN(test_NANO_SequenceIterator,
{
  RUN_TEST(test_NANO_SequenceIterator_API);
})

NANOTEST_RUNNER_MAIN(test_NANO_SequenceIterator)
