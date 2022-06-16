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
#include "TestClock.h"

static void test_NANO_OSAPI_Clock_API(void)
{
  NANO_OSAPI_Clock clock = NANO_OSAPI_CLOCK_INITIALIZER;
  NANO_u64 ms = 0;

  NANO_OSAPI_Clock_initialize(&clock);
  NANO_OSAPI_Clock_millis(&clock, &ms);
  TEST_ASSERT_TRUE(ms > 0);
}

NANOTEST_RUNNER_FN(test_NANO_OSAPI_Clock,
{
  RUN_TEST(test_NANO_OSAPI_Clock_API);
})

NANOTEST_RUNNER_MAIN(test_NANO_OSAPI_Clock)