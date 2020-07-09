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

#ifndef nano_agent_config_h
#define nano_agent_config_h


#ifndef NANO_FEAT_AGENT
#define NANO_FEAT_AGENT     1
#else
#if !NANO_FEAT_AGENT
#error "NANO_FEAT_AGENT must be enabled when including this header"
#endif /* NANO_FEAT_AGENT */
#endif /* NANO_FEAT_AGENT */

#ifndef NANO_FEAT_ASSERT_LIVELINESS_ON_DATA
#define NANO_FEAT_ASSERT_LIVELINESS_ON_DATA             0
#endif /* NANO_FEAT_ASSERT_LIVELINESS_ON_DATA */

/******************************************************************************
 * Default Properties
 ******************************************************************************/


#ifndef NANO_AGENT_DEFAULT_CLIENT_SESSION_TIMEOUT
#define NANO_AGENT_DEFAULT_CLIENT_SESSION_TIMEOUT           NANO_TIME_INITIALIZER_INFINITE
#endif /* NANO_AGENT_DEFAULT_CLIENT_SESSION_TIMEOUT */

#ifndef NANO_AGENT_DEFAULT_AUTO_CLIENT_MAPPING
#define NANO_AGENT_DEFAULT_AUTO_CLIENT_MAPPING              NANO_BOOL_TRUE
#endif /* NANO_AGENT_DEFAULT_AUTO_CLIENT_MAPPING */

#ifndef NANO_AGENT_DEFAULT_CONFIRM_ALL_REQUESTS
#define NANO_AGENT_DEFAULT_CONFIRM_ALL_REQUESTS             NANO_BOOL_FALSE
#endif /* NANO_AGENT_DEFAULT_AUTO_CLIENT_MAPPING */

#ifndef NANO_AGENT_DEFAULT_AUTO_DELETE_RESOURCES
#define NANO_AGENT_DEFAULT_AUTO_DELETE_RESOURCES            NANO_BOOL_TRUE
#endif /* NANO_AGENT_DEFAULT_AUTO_DELETE_RESOURCES */

#ifndef NANO_AGENT_DEFAULT_HEARTBEAT_PERIOD
#define NANO_AGENT_DEFAULT_HEARTBEAT_PERIOD                 { 0, 100000000 }
#endif /* NANO_AGENT_DEFAULT_HEARTBEAT_PERIOD */

#ifndef NANO_AGENT_DEFAULT_TRANSPORT_MTU_MAX
#define NANO_AGENT_DEFAULT_TRANSPORT_MTU_MAX               NANO_U16_MAX
#endif /* NANO_AGENT_DEFAULT_TRANSPORT_MTU_MAX */

#ifndef NANO_AGENT_DEFAULT_SERIAL_RECV_TIMEOUT
#define NANO_AGENT_DEFAULT_SERIAL_RECV_TIMEOUT             60000
#endif /* NANO_AGENT_DEFAULT_TRANSPORT_MTU_MAX */

/******************************************************************************
 * Resource Limits & Defaults
 ******************************************************************************/

#ifndef NANO_LIMIT_READSTARTPOINT_DEFAULT
#define NANO_LIMIT_READSTARTPOINT_DEFAULT                  NDDSA_READSTARTPOINT_FIRST_CACHED
#endif /* NANO_LIMIT_READSTARTPOINT_DEFAULT */

/******************************************************************************
 * External Library: Nano Core
 ******************************************************************************/
#include "nano/nano_core.h"


#endif /* nano_agent_config_h */