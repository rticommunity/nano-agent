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

#ifndef AgentDaemonArgs_h
#define AgentDaemonArgs_h

#include "nano/nano_agent.h"

#define NANO_AGENT_ARGS_CONFIG_L            "config"
#define NANO_AGENT_ARGS_CONFIG_S            "c"

#define NANO_AGENT_ARGS_TIMEOUT_L           "timeout"
#define NANO_AGENT_ARGS_TIMEOUT_S           "t"

#define NANO_AGENT_ARGS_AUTOATTACH_L        "auto-attach"
#define NANO_AGENT_ARGS_AUTOATTACH_S        "a"

#define NANO_AGENT_ARGS_VERBOSITY_L         "verbosity"
#define NANO_AGENT_ARGS_VERBOSITY_S         "v"

#define NANO_AGENT_ARGS_UDP_L               "udp"
#define NANO_AGENT_ARGS_UDP_S               "U"

#define NANO_AGENT_ARGS_UDP_PORT_L          "udp-port"
#define NANO_AGENT_ARGS_UDP_PORT_S          "Up"

#define NANO_AGENT_ARGS_GENERATEID_L        "generate-id"
#define NANO_AGENT_ARGS_GENERATEID_S        "g"

#define NANO_AGENT_ARGS_KIND_L              "kind"
#define NANO_AGENT_ARGS_KIND_S              "k"

#define NANO_AGENT_ARGS_SERIAL_L            "serial"
#define NANO_AGENT_ARGS_SERIAL_S            "S"

#define NANO_AGENT_ARGS_SERIAL_DEV_L        "serial-dev"
#define NANO_AGENT_ARGS_SERIAL_DEV_S        "Sd"

#define NANO_AGENT_ARGS_SERIAL_DEV_1_L      "serial-dev-1"
#define NANO_AGENT_ARGS_SERIAL_DEV_1_S      "Sd1"

#define NANO_AGENT_ARGS_SERIAL_DEV_2_L      "serial-dev-2"
#define NANO_AGENT_ARGS_SERIAL_DEV_2_S      "Sd2"

#define NANO_AGENT_ARGS_SERIAL_SPEED_L      "serial-speed"
#define NANO_AGENT_ARGS_SERIAL_SPEED_S      "Ss"

#define NANO_AGENT_ARGS_SERIAL_SPEED_1_L    "serial-speed-1"
#define NANO_AGENT_ARGS_SERIAL_SPEED_1_S    "Ss1"

#define NANO_AGENT_ARGS_SERIAL_SPEED_2_L    "serial-speed-2"
#define NANO_AGENT_ARGS_SERIAL_SPEED_2_S    "Ss2"

#define NANO_AGENT_ARGS_SERIAL_TIMEOUT_L    "serial-timeout"
#define NANO_AGENT_ARGS_SERIAL_TIMEOUT_S    "St"

#define NANO_AGENT_ARGS_SERIAL_ADDRESS_L    "serial-address"
#define NANO_AGENT_ARGS_SERIAL_ADDRESS_S    "Sa"

#define NANO_AGENT_ARGS_ACKNACKPERIOD_L     "acknack-period"
#define NANO_AGENT_ARGS_ACKNACKPERIOD_S     "ack"

#define NANO_AGENT_ARGS_HEARTBEATPERIOD_L   "heartbeat-period"
#define NANO_AGENT_ARGS_HEARTBEATPERIOD_S   "hb"

#define NANO_AGENT_ARGS_READSTART_L         "read-start"
#define NANO_AGENT_ARGS_READSTART_S         "rs"

#define NANO_AGENT_ARGS_HTTP_L              "http"
#define NANO_AGENT_ARGS_HTTP_S              "http"

#define NANO_AGENT_ARGS_HELP_L              "help"
#define NANO_AGENT_ARGS_HELP_S              "h"

#define NANO_AGENT_ARG_LONG(ARG_) \
    xrce_str_concat(xrce_str_concat(NANO_AGENT_ARGS_,ARG_),_L)

#define NANO_AGENT_ARG_SHORT(ARG_) \
    xrce_str_concat(xrce_str_concat(NANO_AGENT_ARGS_,ARG_),_S)

#define NANO_AGENT_ARG_LONG_STR(ARG_) \
    "--" NANO_AGENT_ARG_LONG(ARG_)

#define NANO_AGENT_ARG_SHORT_STR(ARG_) \
    "-" NANO_AGENT_ARG_SHORT(ARG_)
    

#define NANO_AGENT_MATCH_ARG(str_, ARG_) \
    (strcmp(NANO_AGENT_ARG_LONG_STR(ARG_), str_) == 0 ||\
            strcmp(NANO_AGENT_ARG_SHORT_STR(ARG_), str_) == 0)

#define NANO_AGENT_ARGS_HELP_STR_P(ARG_,param_)\
    NANO_AGENT_ARG_SHORT_STR(ARG_) " " param_ ", "\
    NANO_AGENT_ARG_LONG_STR(ARG_) " " param_

#define NANO_AGENT_ARGS_HELP_STR(ARG_)\
    NANO_AGENT_ARG_SHORT_STR(ARG_) ", "\
    NANO_AGENT_ARG_LONG_STR(ARG_)


typedef struct NANO_AgentDaemonArgsI
{
    char *config_file_path;
    NANO_Time session_timeout;
    NANO_bool auto_attach_resources;
    NANO_bool show_help;
    char *gen_resource_id;
    D2S2_ResourceKind gen_resource_kind;
    NANO_u8 verbosity;
    NANO_u16 xrce_udp_port;
    NANO_bool xrce_udp_enable;
    NANO_bool xrce_serial_enable;
    const char *xrce_serial_dev;
    const char *xrce_serial_dev_1;
    const char *xrce_serial_dev_2;
    NANO_u32 xrce_serial_speed;
    NANO_u32 xrce_serial_speed_1;
    NANO_u32 xrce_serial_speed_2;
    NANO_i32 xrce_serial_timeout;
    NANO_XRCE_TransportLocatorSmall xrce_serial_address;
    NANO_i32 acknack_period;
    NANO_i32 heartbeat_period;
    NDDSA_ReadStartPoint read_start;
    NANO_bool http_enable;
} NANO_AgentDaemonArgs;

#define NANO_AGENTDAEMONARGS_INITIALIZER \
{\
    NULL, /* config_file_path */\
    NANO_TIME_INITIALIZER_INFINITE, /* session_timeout */\
    NANO_BOOL_FALSE, /* auto_attach_resources */\
    NANO_BOOL_FALSE, /* show_help */\
    NULL, /* gen_resource_id */\
    D2S2_RESOURCEKIND_UNKNOWN, /* gen_resource_kind */\
    0, /* verbosity */\
    0, /* xrce_udp_port */\
    NANO_BOOL_FALSE, /* xrce_udp_enable */\
    NANO_BOOL_FALSE, /* xrce_serial_enable */\
    NULL, /* xrce_serial_dev */\
    NULL, /* xrce_serial_dev_1 */\
    NULL, /* xrce_serial_dev_2 */\
    0, /* xrce_serial_speed */\
    0, /* xrce_serial_speed_1 */\
    0, /* xrce_serial_speed_2 */\
    -2, /* xrce_serial_timeout */\
    NANO_LIMIT_SERIALCLIENTTRANSPORT_AGENT_ADDRESS_DEFAULT, /* xrce_serial_address */\
    0, /* acknack_period */\
    0, /* heartbeat_period */\
    NANO_LIMIT_READSTARTPOINT_DEFAULT, /* read_start */\
    NANO_BOOL_FALSE /* http_enable */\
}

#endif /* AgentDaemonArgs_h */