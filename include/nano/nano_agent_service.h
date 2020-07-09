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

/**
 * @file nano_agent_service.h
 * @brief Nano Agent Service API.
 * 
 * This header file should be included to access the services provided by the
 * Nano Agent library.
 * 
 * @defgroup nanoagent_api Nano Agent API
 * 
 * @addtogroup nanoagent_api
 * @{
 */

#ifndef nano_agent_service_h
#define nano_agent_service_h

/******************************************************************************
 *                            ClientLocatorMapping
 ******************************************************************************/

typedef struct NANODllExport NANO_XRCE_ClientLocatorMappingI
{
    NANO_XRCE_ClientKey client_key;
    NANO_XRCE_TransportLocator locator;
} NANO_XRCE_ClientLocatorMapping;

#define NANO_XRCE_CLIENTLOCATORMAPPING_INITIALIZER \
{\
    NANO_XRCE_CLIENTKEY_INVALID, /* client_key */\
    NANO_XRCE_TRANSPORTLOCATOR_INITIALIZER /* locator */\
}

/******************************************************************************
 *                              AgentProperties
 ******************************************************************************/

typedef struct NANODllExport NANO_XRCE_AgentPropertiesI
{
    NANO_Time client_session_timeout;
    NANO_XRCE_ClientLocatorMapping *client_mappings;
    NANO_usize client_mappings_len;
    NANO_bool auto_client_mapping;
    NANO_bool confirm_all_requests;
    NANO_bool auto_delete_resources;
    NANO_Time heartbeat_period;
    NANO_usize max_messages_per_acknack;
    NANO_usize max_messages_per_heartbeat;
} NANO_XRCE_AgentProperties;

#define NANO_XRCE_AGENTPROPERTIES_INITIALIZER \
{\
    NANO_AGENT_DEFAULT_CLIENT_SESSION_TIMEOUT, /* client_session_timeout */\
    NULL, /* client_mappings */\
    0, /* client_mappings_len */\
    NANO_AGENT_DEFAULT_AUTO_CLIENT_MAPPING, /* auto_client_mapping */\
    NANO_AGENT_DEFAULT_CONFIRM_ALL_REQUESTS, /* confirm_all_requests */\
    NANO_AGENT_DEFAULT_AUTO_DELETE_RESOURCES, /* auto_delete_resources */\
    NANO_AGENT_DEFAULT_HEARTBEAT_PERIOD, /* heartbeat_period */\
    0, /* max_messages_per_acknack */\
    0, /* max_messages_per_heartbeat */\
}


/******************************************************************************
 *                                  Agent
 ******************************************************************************/

NANODllExport
NANO_XRCE_Agent*
NANO_XRCE_Agent_new(NANO_XRCE_AgentProperties *const properties);

NANODllExport
void
NANO_XRCE_Agent_delete(NANO_XRCE_Agent *const self);

NANODllExport
NANO_RetCode
NANO_XRCE_Agent_register_transport(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_AgentTransportProperties *const properties);

NANODllExport
NANO_RetCode
NANO_XRCE_Agent_enable(NANO_XRCE_Agent *const self);

NANODllExport
NANO_RetCode
NANO_XRCE_Agent_disable(NANO_XRCE_Agent *const self);


/******************************************************************************
 * Agent interface (for D2S2_AgentServer)
 ******************************************************************************/

#include "dds_agent/dds_agent.h"

D2S2_AgentServerInterface*
NANO_XRCE_Agent_as_interface(NANO_XRCE_Agent *const self);

#endif /* nano_agent_service_h */

/** @} */
