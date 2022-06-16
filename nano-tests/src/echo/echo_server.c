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
#include <stdio.h>
#include <stdlib.h>
#include "nano/nano_core_test.h"

NANO_bool echo_server_f_active = NANO_BOOL_FALSE;
NANO_OSAPI_Udpv4Socket * echo_server_f_exit_socket = NULL;
NANO_Ipv4Addr echo_server_f_exit_socket_addr = 0;
NANO_u16 echo_server_f_exit_socket_port = 0;
const NANO_u8 echo_server_f_exit_flag[4] = {0xAA, 0xBB, 0xCC, 0xDD};

NANO_u8 echo_server_f_rx_buffer[65535] = { 0 };

static void echo_server_exit_on_signal_handler()
{
  NANO_MessageBuffer mbuf = NANO_MESSAGEBUFFER_INITIALIZER;
  echo_server_f_active = NANO_BOOL_FALSE;


  if (NULL != echo_server_f_exit_socket)
  {
    NANO_MessageBuffer_set_external_data(&mbuf,
      (NANO_u8*) echo_server_f_exit_flag, sizeof(echo_server_f_exit_flag));
    NANO_OSAPI_Udpv4Socket_send(
      echo_server_f_exit_socket,
      (NANO_u8*)&echo_server_f_exit_socket_addr,
      echo_server_f_exit_socket_port,
      &mbuf);
  }
}

#if NANO_PLATFORM_IS_POSIX
#include <signal.h>

/* Signal handler functions */

static void echo_server_exit_on_signal()
{
  echo_server_exit_on_signal_handler();
}

#if _POSIX_C_SOURCE
static NANO_RetCode
echo_server_set_exit_on_signal(int sig)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    struct sigaction new_action;
    struct sigaction old_action;
    
    NANO_LOG_FN_ENTRY
    
    memset(&new_action, 0, sizeof (struct sigaction));
    new_action.sa_handler = echo_server_exit_on_signal;

    /* Temporarily block all other signals during handler execution */
    sigfillset(&new_action.sa_mask);

    if (sigaction(sig, NULL, &old_action) != 0)
    {
        NANO_LOG_ERROR_MSG("sigaction(old) FAILED")
        goto done;
    }

    /* Honor inherited SIG_IGN that's set by some shell's */
    if (old_action.sa_handler != SIG_IGN)
    {
        if (sigaction(sig, &new_action, NULL) != 0) {
            NANO_LOG_ERROR_MSG("sigaction(new) FAILED")
            goto done;
        }
    }
    
    rc = NANO_RETCODE_OK;
    
done:
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}
#else
NANO_RetCode
echo_server_set_exit_on_signal(int sig)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;

    NANO_LOG_FN_ENTRY
    
    signal(sig, echo_server_exit_on_signal);

    rc = NANO_RETCODE_OK;
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}
#endif /* _XOPEN_SOURCE >= 700 */
#elif NANO_PLATFORM == NANO_PLATFORM_WINDOWS
BOOL echo_server_exit_on_signal(DWORD cntrlType)
{
    echo_server_exit_on_signal_handler();
    return TRUE;
}
#endif /* NANO_PLATFORM == NANO_PLATFORM_WINDOWS */

NANO_RetCode
echo_server(
  const NANO_Ipv4Addr bind_addr,
  const NANO_u16 bind_port)
{
  NANO_RetCode retcode = NANO_RETCODE_ERROR;
  NANO_OSAPI_Udpv4Socket socket_rx = NANO_OSAPI_UDPV4SOCKET_INITIALIZER;
  NANO_OSAPI_Udpv4Socket socket_tx = NANO_OSAPI_UDPV4SOCKET_INITIALIZER;
  NANO_OSAPI_Udpv4Socket * socket_tx_ptr = NULL;
  NANO_OSAPI_Udpv4SocketProperties socket_props =
    NANO_OSAPI_UDPV4SOCKETPROPERTIES_INITIALIZER;
  NANO_RetCode rc = NANO_RETCODE_ERROR;
  NANO_MessageBuffer rx_mbuf = NANO_MESSAGEBUFFER_INITIALIZER;
  NANO_Ipv4Addr tx_addr = 0;
  NANO_u16 tx_port = 0;
  NANO_u16 tx_socket_port = 0;
  NANO_usize rx_size = 0;
#if NANO_PLATFORM_IS_POSIX
  sigset_t set, oset;
#define MAX_SIGNALS     5
  int signals[MAX_SIGNALS] = { SIGTERM, SIGHUP, SIGINT, SIGABRT, SIGPIPE };
  int i = 0;
#endif

#if NANO_PLATFORM_IS_POSIX || NANO_PLATFORM == NANO_PLATFORM_WINDOWS
  socket_props.timeout_ms = 2000;
#endif /* NANO_PLATFORM_IS_POSIX || NANO_PLATFORM == NANO_PLATFORM_WINDOWS */

  echo_server_f_active = NANO_BOOL_TRUE;

/* Install signal handlers */
#if NANO_PLATFORM == NANO_PLATFORM_WINDOWS
  /* is a console app, set signal handler */
  if (!SetConsoleCtrlHandler(
    (PHANDLER_ROUTINE)echo_server_exit_on_signal, TRUE))
  {
    NANO_LOG_ERROR_MSG("FAILED to set signal handler")
    goto done;
  }
#elif NANO_PLATFORM_IS_POSIX
#if _XOPEN_SOURCE
  /*
    * Mask all signals during startup.
    */
  if (sigfillset(&set))
  {
    NANO_LOG_ERROR_MSG("FAILED to initialize signal mask")
    goto done;
  }
  if (pthread_sigmask(SIG_SETMASK, &set, &oset)) 
  {
    NANO_LOG_ERROR_MSG("FAILED to disable signals")
    goto done;
  }
#endif /* _XOPEN_SOURCE */
  /*
   * Install signals handlers
   */
  for (i = 0; i < MAX_SIGNALS; i++)
  {
    int sig = signals[i];
    NANO_LOG_DEBUG("setting exit handler", NANO_LOG_I32("signal", sig))
    NANO_CHECK_RC(
      echo_server_set_exit_on_signal(sig),
      NANO_LOG_ERROR("FAILED to set signal handler",
        NANO_LOG_I32("signal",sig)));
  }
#endif /* NANO_PLATFORM_IS_POSIX */

  NANO_MessageBuffer_set_external_data(
    &rx_mbuf, echo_server_f_rx_buffer, sizeof(echo_server_f_rx_buffer));

  rc = NANO_OSAPI_Udpv4Socket_open(
    &socket_rx,
    (const NANO_u8*)&bind_addr,
    bind_port,
    &socket_props);
  if (NANO_RETCODE_OK != rc)
  {
    NANO_LOG_ERROR("failed to open RX UDP socket",
      NANO_LOG_RC(rc))
    goto done;
  }

  echo_server_f_exit_socket = &socket_rx;
  echo_server_f_exit_socket_addr =
    (bind_addr == 0)? NANOTEST_host_ip_address() : bind_addr;
  echo_server_f_exit_socket_port = bind_port;

  while (echo_server_f_active)
  {
    printf("waiting for data...\n");
    NANO_MessageBuffer_set_data_len(&rx_mbuf, sizeof(echo_server_f_rx_buffer));
    rc = NANO_OSAPI_Udpv4Socket_recv(
      &socket_rx, (NANO_u8*)&tx_addr, &tx_port, &rx_mbuf, &rx_size);
    if (NANO_RETCODE_OK != rc)
    {
      NANO_LOG_ERROR("failed to receive data on RX UDP socket",
        NANO_LOG_RC(rc))
      goto done;
    }

    if (!echo_server_f_active)
    {
      printf("exiting on !active\n");
      continue;
    }

    if (rx_size > 0)
    {
      printf("received %lu bytes\n", rx_size);
      // if (tx_port != bind_port)
      // {
      //   socket_tx_ptr = &socket_tx;
      //   if (tx_socket_port != tx_port)
      //   {
      //     if (tx_socket_port != 0)
      //     {
      //       printf("closing tx_socket %d\n", tx_socket_port);
      //       tx_socket_port = 0;
      //       rc = NANO_OSAPI_Udpv4Socket_close(&socket_tx);
      //       if (NANO_RETCODE_OK != rc)
      //       {
      //         NANO_LOG_ERROR("failed to close TX UDP socket",
      //           NANO_LOG_RC(rc))
      //         goto done;
      //       }
      //     }
      //     printf("opening tx_socket %d\n", tx_port);
      //     rc = NANO_OSAPI_Udpv4Socket_open(
      //       &socket_tx,
      //       (const NANO_u8*)&bind_addr,
      //       tx_port,
      //       &socket_props);
      //     if (NANO_RETCODE_OK != rc)
      //     {
      //       NANO_LOG_ERROR("failed to open TX UDP socket",
      //         NANO_LOG_RC(rc))
      //       goto done;
      //     }
      //     tx_socket_port = tx_port;
      //   }
      // }
      // else
      // {
      //   printf("reusing rx_socket %d\n", tx_socket_port);
      //   socket_tx_ptr = &socket_rx;
      // }
      // printf("sending back %d bytes\n", NANO_MessageBuffer_data_len(&rx_mbuf));
      rc = NANO_OSAPI_Udpv4Socket_send(
        &socket_rx,
        (const NANO_u8*)&tx_addr,
        tx_port,
        &rx_mbuf);
      if (NANO_RETCODE_OK != rc)
      {
        NANO_LOG_ERROR("failed to echo UDP message",
          NANO_LOG_RC(rc)
          NANO_LOG_BYTES("tx_addr", &tx_addr, sizeof(tx_addr))
          NANO_LOG_U16("tx_port", tx_port)
          NANO_LOG_U16("bind_port", bind_port))
        goto done;
      }
    } else {
      printf("no data received\n");
    }
  }

  retcode = NANO_RETCODE_OK;

done:
  printf("finalizing echo server...\n");
  echo_server_f_exit_socket = NULL;
  if (NANO_OSAPI_Udpv4Socket_is_valid(&socket_rx))
  {
    rc = NANO_OSAPI_Udpv4Socket_close(&socket_rx);
    if (NANO_RETCODE_OK != rc)
    {
      NANO_LOG_ERROR("failed to close RX UDP socket",
        NANO_LOG_RC(rc))
    }
  }
  if (tx_socket_port != 0)
  {
    rc = NANO_OSAPI_Udpv4Socket_close(&socket_tx);
    if (NANO_RETCODE_OK != rc)
    {
      NANO_LOG_ERROR("failed to close TX UDP socket",
        NANO_LOG_RC(rc))
    }
  }

  return retcode;
}

int main(int argc, const char ** argv)
{
  NANO_u32 user_val = 0;
  NANO_Ipv4Addr bind_addr = 0;
  NANO_u16 bind_port = NANOTEST_LIMIT_UDP_PORT_2;
  char *invalid_str = NULL;
  int i = 0;

  for (i = 1; i < argc; i++)
  {
    if (strcmp("--address", argv[i]) == 0 &&
      strcmp("-a", argv[i]) == 0)
    {
      if (i == (argc - 1))
      {
        return -1;
      }
      user_val = strtoul(argv[i + 1], &invalid_str, 0);
      if (invalid_str[0] != '\0')
      {
        return -1;
      }
      bind_addr = NANO_u32_to_be(user_val);
      i += 1;
    }
    else if (strcmp("--port", argv[i]) == 0 &&
      strcmp("-p", argv[i]) == 0)
    {
      if (i == (argc - 1))
      {
        return -1;
      }
      user_val = strtoul(argv[i], &invalid_str, 0);
      if (invalid_str[0] != '\0')
      {
        return -1;
      }
      if (bind_port > 65535)
      {
        return -1;
      }
      bind_port = (NANO_u16)user_val;
      i += 1;
    } else {
      return -1;
    }
  }

  if (NANO_RETCODE_OK != echo_server(bind_addr, bind_port))
  {
    return -1;
  }

  return 0;
}