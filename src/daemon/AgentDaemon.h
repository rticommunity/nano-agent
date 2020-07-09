/******************************************************************************
 *
 * (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software solely in combination with RTI Connext DDS. Licensee
 * may redistribute copies of the Software provided that all such copies are
 * subject to this License. The Software is provided "as is", with no warranty
 * of any type, including any warranty for fitness for any purpose. RTI is
 * under no obligation to maintain or support the Software. RTI shall not be
 * liable for any incidental or consequential damages arising out of the use or
 * inability to use the Software. For purposes of clarity, nothing in this
 * License prevents Licensee from using alternate versions of DDS, provided
 * that Licensee may not combine or link such alternate versions of DDS with
 * the Software.
 *
 ******************************************************************************/


#ifndef AgentDaemon_h
#define AgentDaemon_h

#include "nano/nano_agent.h"

#include "AgentDaemonArgs.h"

typedef struct NANO_AgentDaemonI
{
    NANO_XRCE_Agent *agent;
    NANO_XRCE_Udpv4AgentTransport transport_udpv4;
    NANO_XRCE_SerialAgentTransport transport_serial;
    NANO_XRCE_SerialAgentTransport transport_serial_1;
    NANO_XRCE_SerialAgentTransport transport_serial_2;
    D2S2_Agent *server;
    struct RTIOsapiSemaphore *sem_exit;
    NANO_AgentDaemonArgs args;
} NANO_AgentDaemon;

#define NANO_AGENTDAEMON_INITIALIZER \
{\
    NULL, /* agent */\
    NANO_XRCE_UDPV4AGENTTRANSPORT_INITIALIZER, /* transport_udpv4 */\
    NANO_XRCE_SERIALAGENTTRANSPORT_INITIALIZER, /* transport_serial */\
    NANO_XRCE_SERIALAGENTTRANSPORT_INITIALIZER, /* transport_serial_1 */\
    NANO_XRCE_SERIALAGENTTRANSPORT_INITIALIZER, /* transport_serial_2 */\
    NULL, /* server */\
    NULL,  /* sem_exit */\
    NANO_AGENTDAEMONARGS_INITIALIZER /* args */\
}

extern NANO_AgentDaemon NANO_AgentDaemon_g_self;

NANO_AgentDaemon*
NANO_AgentDaemon_get_instance();

#define NANO_AgentDaemon_get_instance() \
    (&NANO_AgentDaemon_g_self)

NANO_RetCode
NANO_AgentDaemon_initialize(
    NANO_AgentDaemon *const self,
    const int argc,
    const char **const argv);

void
NANO_AgentDaemon_finalize(NANO_AgentDaemon *const self);

NANO_RetCode
NANO_AgentDaemon_enable(NANO_AgentDaemon *const self);

void
NANO_AgentDaemon_wait_for_exit(NANO_AgentDaemon *const self);

NANO_RetCode
NANO_AgentDaemon_register_udpv4(
    NANO_AgentDaemon *const self,
    NANO_XRCE_AgentTransportProperties *const properties);

NANO_RetCode
NANO_AgentDaemon_parse_args(
    NANO_AgentDaemon *const self,
    const int argc,
    const char **const argv);


#endif /* AgentDaemon_h */