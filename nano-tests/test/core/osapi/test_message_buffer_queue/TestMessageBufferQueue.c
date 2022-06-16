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
#include "TestMessageBufferQueue.h"

static void test_NANO_MessageBufferQueue_API(void)
{
  NANO_MessageBuffer mbuf1 = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer mbuf2 = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer mbuf3 = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer mbuf4 = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_MessageBuffer * mbuf = NULL;
  NANO_MessageBufferQueue queue = NANO_MESSAGEBUFFERQUEUE_INITIALIZER;
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_NULL(NANO_MessageBufferQueue_head(&queue));
  TEST_ASSERT_NULL(NANO_MessageBufferQueue_tail(&queue));
  TEST_ASSERT_EQUAL_INT32(0, NANO_MessageBufferQueue_size(&queue));

  NANO_MessageBufferQueue_insert_first(&queue, &mbuf1);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_head(&queue) == &mbuf1);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_tail(&queue) == &mbuf1);
  TEST_ASSERT_FALSE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(1, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));

  NANO_MessageBufferQueue_insert_first(&queue, &mbuf2);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_head(&queue) == &mbuf2);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_tail(&queue) == &mbuf1);
  TEST_ASSERT_FALSE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(2, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf2) == &mbuf1);

  NANO_MessageBufferQueue_insert_last(&queue, &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_head(&queue) == &mbuf2);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_tail(&queue) == &mbuf3);
  TEST_ASSERT_FALSE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(3, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf2) == &mbuf1);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf1) == &mbuf3);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf3));

  NANO_MessageBufferQueue_insert_last(&queue, &mbuf4);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_head(&queue) == &mbuf2);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_tail(&queue) == &mbuf4);
  TEST_ASSERT_FALSE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(4, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf2) == &mbuf1);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf1) == &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf3) == &mbuf4);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf4));

  NANO_MessageBufferQueue_extract(&queue, &mbuf1);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_head(&queue) == &mbuf2);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_tail(&queue) == &mbuf4);
  TEST_ASSERT_FALSE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(3, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf2) == &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf3) == &mbuf4);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf4));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));

  NANO_MessageBufferQueue_pop_first(&queue, &mbuf);
  TEST_ASSERT_TRUE(&mbuf2 == mbuf);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_head(&queue) == &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_tail(&queue) == &mbuf4);
  TEST_ASSERT_FALSE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(2, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_TRUE(NANO_MessageBuffer_next_msg(&mbuf3) == &mbuf4);
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf4));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf2));

  NANO_MessageBufferQueue_pop_last(&queue, &mbuf);
  TEST_ASSERT_TRUE(&mbuf4 == mbuf);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_head(&queue) == &mbuf3);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_tail(&queue) == &mbuf3);
  TEST_ASSERT_FALSE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(1, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf3));

  NANO_MessageBufferQueue_insert_first(&queue, &mbuf2);
  NANO_MessageBufferQueue_insert_first(&queue, &mbuf1);
  NANO_MessageBufferQueue_insert_last(&queue, &mbuf4);
  TEST_ASSERT_EQUAL_INT32(4, NANO_MessageBufferQueue_size(&queue));
  NANO_MessageBufferQueue_drop(&queue);
  TEST_ASSERT_TRUE(NANO_MessageBufferQueue_empty(&queue));
  TEST_ASSERT_EQUAL_INT32(0, NANO_MessageBufferQueue_size(&queue));
  TEST_ASSERT_NULL(NANO_MessageBufferQueue_head(&queue));
  TEST_ASSERT_NULL(NANO_MessageBufferQueue_tail(&queue));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf1));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf2));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf3));
  TEST_ASSERT_NULL(NANO_MessageBuffer_next_msg(&mbuf4));
}

NANOTEST_RUNNER_FN(test_NANO_MessageBufferQueue,
{
  RUN_TEST(test_NANO_MessageBufferQueue_API);
})

NANOTEST_RUNNER_MAIN(test_NANO_MessageBufferQueue)
