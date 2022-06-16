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

#if NANO_PLATFORM == NANO_PLATFORM_ARDUINO
#include <Arduino.h>
#endif /* NANO_PLATFORM == NANO_PLATFORM_ARDUINO */

#define xstr(s) str(s)
#define str(s) #s

static const char * const NANO_f_Test_version =
  xstr(NANO_TEST_VERSION_MAJOR)
  "." xstr(NANO_TEST_VERSION_MINOR)
  "." xstr(NANO_TEST_VERSION_RELEASE);

const char * NANOTEST_version_string()
{
  return NANO_f_Test_version;
}

#ifdef NANO_HAVE_UDPV4
NANO_Ipv4Addr NANOTEST_f_host_ip_address = 0;
void
NANOTEST_set_host_ip_address(const NANO_Ipv4Addr addr)
{
  NANOTEST_f_host_ip_address = addr;
}
NANO_Ipv4Addr
NANOTEST_host_ip_address()
{
  if (NANOTEST_f_host_ip_address == 0)
  {
    NANOTEST_f_host_ip_address = NANO_u32_to_be(NANOTEST_LIMIT_HOST_IP_ADDRESS);
  }
  return NANOTEST_f_host_ip_address;
}

NANO_Ipv4Addr NANOTEST_f_agent_ip_address = 0;
void
NANOTEST_set_agent_ip_address(const NANO_Ipv4Addr addr)
{
  NANOTEST_f_agent_ip_address = addr;
}
NANO_Ipv4Addr
NANOTEST_agent_ip_address()
{
  if (NANOTEST_f_agent_ip_address == 0)
  {
    NANOTEST_f_agent_ip_address = NANO_u32_to_be(NANOTEST_LIMIT_AGENT_IP_ADDRESS);
  }
  return NANOTEST_f_agent_ip_address;
}
#endif /* NANO_HAVE_UDPV4 */

#ifdef NANOTEST_HAVE_SLEEP
NANOTESTDllExport
void NANOTEST_sleep(const NANO_u64 ms)
{
#if NANO_PLATFORM_IS_POSIX
  NANO_Time ts = NANO_TIME_INITIALIZER;
  NANO_Time_from_millis(&ts, ms);
#if defined(HAVE_NANOSLEEP) /* use nanosleep() if available */
  {  
    struct timespec tout = { 0, 0 };
    tout.tv_sec = ts.sec;
    tout.tv_nsec = ts.nanosec;
    nanosleep(&tout, NULL);
  }
#elif defined(HAVE_USLEEP) /* otherwise fall back to usleep() or... */
  (void)ts;
  usleep(ms * 1000);
#else  /* finally resort to sleep() */
  if (ts.nanosec > 500000000)
  {
    ts.sec += 1;
  }
  sleep(ts.sec);
#endif /* HAVE_NANOSLEEP */
#elif NANO_PLATFORM == NANO_PLATFORM_WINDOWS
  SleepEx(ms, 0);
#elif NANO_PLATFORM == NANO_PLATFORM_ARDUINO
  delay(ms);
#else
  #error "no implementation for sleep() on this platform"
#endif
}
#endif /* NANOTEST_HAVE_SLEEP */

#if NANO_PLATFORM == NANO_PLATFORM_ARDUINO
#if NANOTEST_FEAT_ARDUINO_WIFI
#if defined(ESP8266)
#include "ESP8266WiFi.h"
#elif defined(ESP32)
#include <WiFi.h>
#else
#include <WiFi.h>
#endif
#endif /* NANOTEST_FEAT_ARDUINO_WIFI */

void NANOTEST_init_platform_arduino()
{
  delay(2500);
  NANO_LOG_SET_LEVEL(LEVEL_DEBUG)
#if NANOTEST_FEAT_ARDUINO_SERIAL
  NANOTEST_LIMIT_ARDUINO_SERIAL_PORT.begin(NANOTEST_LIMIT_ARDUINO_SERIAL_SPEED);
  delay(NANOTEST_LIMIT_ARDUINO_INIT_DELAY);
  while (!NANOTEST_LIMIT_ARDUINO_SERIAL_PORT &&
    !NANOTEST_LIMIT_ARDUINO_SERIAL_PORT.available())
  {
    delay(NANOTEST_LIMIT_ARDUINO_INIT_DELAY);
  }
#endif /* NANOTEST_FEAT_ARDUINO_SERIAL */
#if NANOTEST_FEAT_ARDUINO_WIFI
  WiFi.begin(NANOTEST_LIMIT_ARDUINO_WIFI_SSID, NANOTEST_LIMIT_ARDUINO_WIFI_KEY);
  delay(NANOTEST_LIMIT_ARDUINO_INIT_DELAY);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(NANOTEST_LIMIT_ARDUINO_INIT_DELAY);
  }
  NANOTEST_set_host_ip_address(WiFi.localIP().v4());
#endif /* NANOTEST_FEAT_ARDUINO_WIFI */
  // NANO_LOG_Writer_set_level(NANO_LOG_WRITER, NANO_LOG_LEVEL_TRACE_FN);
}

#ifndef PIO_UNIT_TESTING
void setup()
{

}

void loop()
{

}
#endif /* PIO_UNIT_TESTING */
#endif /* NANO_PLATFORM == NANO_PLATFORM_ARDUINO */
