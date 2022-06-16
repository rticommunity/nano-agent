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
#ifndef nano_core_test_h
#define nano_core_test_h
#include "nano/nano_core.h"
#include "unity.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *                                Dll Support
 ******************************************************************************/
#if NANO_PLATFORM == NANO_PLATFORM_WINDOWS

    #if defined(NANOTEST_DLL_EXPORT)
        #define NANOTESTDllExport __declspec( dllexport )
    #else
        #define NANOTESTDllExport
    #endif /* NANOTEST_DLL_EXPORT */

    #if defined(NANOTEST_DLL_VARIABLE) 
        #if defined(NANOTEST_DLL_EXPORT)
            #define NANOTESTDllVariable __declspec( dllexport )
        #else
            #define NANOTESTDllVariable __declspec( dllimport )
        #endif /* NANO_DLL_EXPORT */
    #else 
        #define NANOTESTDllVariable
    #endif /* NANO_DLL_VARIABLE */

#else /* NANO_PLATFORM != NANO_PLATFORM_WINDOWS */
/*i
 * @brief Export function symbols when building Dll.
 */
#define NANOTESTDllExport
/*i
 * @brief Export variable symbols when building Dll, or import them when
 * linking user code.
 */
#define NANOTESTDllVariable
#endif /* NANO_PLATFORM == NANO_PLATFORM_WINDOWS */


#define TEST_ASSERT_RC(expected_, actual_) \
  TEST_ASSERT_EQUAL_INT_MESSAGE(expected_,actual_,"invalid retcode")

#define TEST_ASSERT_EQUAL_TIME(sec_, nsec_, t_) \
{\
  TEST_ASSERT_EQUAL_INT32_MESSAGE(sec_, (t_)->sec, "invalid seconds");\
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(nsec_, (t_)->nanosec, "invalid nanoseconds");\
}

#define TEST_ASSERT_EQUAL_TIME_INFINITE(t_) \
{\
  static const NANO_Time t_infinite_ = NANO_TIME_INITIALIZER_INFINITE;\
  TEST_ASSERT_EQUAL_TIME(t_infinite_.sec, t_infinite_.nanosec, (t_));\
}

#ifndef NANO_TEST_VERSION_MAJOR
#define NANO_TEST_VERSION_MAJOR       0
#endif /* NANO_TEST_VERSION_MAJOR */
#ifndef NANO_TEST_VERSION_MINOR
#define NANO_TEST_VERSION_MINOR       1
#endif /* NANO_TEST_VERSION_MINOR */
#ifndef NANO_TEST_VERSION_RELEASE
#define NANO_TEST_VERSION_RELEASE     0
#endif /* NANO_TEST_VERSION_RELEASE */

NANOTESTDllExport
const char *
NANOTEST_version_string();

#ifdef NANO_HAVE_UDPV4
NANOTESTDllExport
void NANOTEST_set_host_ip_address(const NANO_Ipv4Addr addr);
NANO_Ipv4Addr NANOTEST_host_ip_address();

NANOTESTDllExport
void NANOTEST_set_agent_ip_address(const NANO_Ipv4Addr addr);
NANO_Ipv4Addr NANOTEST_agent_ip_address();
#endif /* NANO_HAVE_UDPV4 */

#if NANO_PLATFORM == NANO_PLATFORM_WINDOWS
/* Include synchapi.h for SleepEx() */
#include <synchapi.h>
#define NANOTEST_HAVE_SLEEP
#elif NANO_PLATFORM_IS_POSIX
#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#define NANOTEST_HAVE_NANOSLEEP
#elif (_XOPEN_SOURCE >= 500) && ! (_POSIX_C_SOURCE >= 200809L)\
        || /* Glibc since 2.19: */ _DEFAULT_SOURCE \
        || /* Glibc versions <= 2.19: */ _BSD_SOURCE
#define NANOTEST_HAVE_USLEEP
#endif
#define NANOTEST_HAVE_SLEEP
#elif NANO_PLATFORM == NANO_PLATFORM_ARDUINO
#define NANOTEST_HAVE_SLEEP
#endif

#ifdef NANOTEST_HAVE_SLEEP
NANOTESTDllExport
void NANOTEST_sleep(const NANO_u64 ms);
#endif /* NANOTEST_HAVE_SLEEP */

/******************************************************************************
 * Helpers for Platform.io Unit Testing
 ******************************************************************************/
#if NANO_PLATFORM != NANO_PLATFORM_ARDUINO
#define NANOTEST_RUNNER_MAIN(test_fn_)
#define NANOTEST_RUNNER_FN(test_fn_, test_entries_) \
int test_fn_(void)\
{\
  UNITY_BEGIN();\
  test_entries_\
  return UNITY_END();\
}
#else /* NANO_PLATFORM == NANO_PLATFORM_ARDUINO */
void NANOTEST_init_platform_arduino();

#define NANOTEST_RUNNER_FN(test_fn_, test_entries_) \
int test_fn_(void)\
{\
  test_entries_\
  return 0;\
}

#define NANOTEST_RUNNER_MAIN(test_fn_) \
static bool NANOTEST_f_test_done = false;\
void setup()\
{\
  NANOTEST_init_platform_arduino();\
  UNITY_BEGIN();\
}\
void loop()\
{\
  if (!NANOTEST_f_test_done)\
  {\
    NANOTEST_f_test_done = true;\
    test_fn_();\
    UNITY_END();\
  }\
}
#endif /* NANO_PLATFORM == NANO_PLATFORM_ARDUINO */

/******************************************************************************
 * Detect test features based on NANO platform and feature flags
 ******************************************************************************/
#ifndef NANOTEST_FEAT_UDP_ECHO
#define NANOTEST_FEAT_UDP_ECHO                0
#endif /* NANOTEST_FEAT_UDP_ECHO */

#ifndef NANOTEST_FEAT_ARDUINO_SERIAL
#define NANOTEST_FEAT_ARDUINO_SERIAL \
  (NANO_PLATFORM == NANO_PLATFORM_ARDUINO)
#endif /* NANOTEST_FEAT_ARDUINO_SERIAL */

#ifndef NANOTEST_FEAT_ARDUINO_WIFI
#define NANOTEST_FEAT_ARDUINO_WIFI \
  (NANO_PLATFORM == NANO_PLATFORM_ARDUINO &&\
    (defined(ESP8266) || defined(ESP32)))
#endif /* NANOTEST_FEAT_ARDUINO_WIFI */

#ifndef NANOTEST_LIMIT_ARDUINO_INIT_DELAY
#define NANOTEST_LIMIT_ARDUINO_INIT_DELAY     1000
#endif /* NANOTEST_LIMIT_ARDUINO_INIT_DELAY */

#ifndef NANOTEST_LIMIT_ARDUINO_SERIAL_PORT
#define NANOTEST_LIMIT_ARDUINO_SERIAL_PORT    Serial
#endif /* NANOTEST_LIMIT_ARDUINO_SERIAL_PORT */

#ifndef NANOTEST_LIMIT_ARDUINO_SERIAL_SPEED
#define NANOTEST_LIMIT_ARDUINO_SERIAL_SPEED   115200
#endif /* NANOTEST_LIMIT_ARDUINO_SERIAL_SPEED */

#ifndef NANOTEST_LIMIT_ARDUINO_WIFI_SSID
#define NANOTEST_LIMIT_ARDUINO_WIFI_SSID      ""
#endif /* NANOTEST_LIMIT_ARDUINO_WIFI_SSID */

#ifndef NANOTEST_LIMIT_ARDUINO_WIFI_KEY
#define NANOTEST_LIMIT_ARDUINO_WIFI_KEY       ""
#endif /* NANOTEST_LIMIT_ARDUINO_WIFI_KEY */

#ifndef NANOTEST_LIMIT_UDP_PORT_1
#define NANOTEST_LIMIT_UDP_PORT_1             8401
#endif /* NANOTEST_LIMIT_UDP_PORT_1 */

#ifndef NANOTEST_LIMIT_UDP_PORT_2
#define NANOTEST_LIMIT_UDP_PORT_2             8402
#endif /* NANOTEST_LIMIT_UDP_PORT_2 */

#ifndef NANOTEST_LIMIT_HOST_IP_ADDRESS
#define NANOTEST_LIMIT_HOST_IP_ADDRESS       0x7f000001
#endif /* NANOTEST_LIMIT_HOST_IP_ADDRESS */

#ifndef NANOTEST_LIMIT_AGENT_IP_ADDRESS
#define NANOTEST_LIMIT_AGENT_IP_ADDRESS      0x7f000001
#endif /* NANOTEST_LIMIT_AGENT_IP_ADDRESS */

#ifdef __cplusplus
}	/* extern "C" */
#endif

#endif /* nano_core_test_h */