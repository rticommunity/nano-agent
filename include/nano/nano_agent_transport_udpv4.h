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

#ifndef nano_agent_transport_udpv4_h
#define nano_agent_transport_udpv4_h

#include "nano/nano_core_xrce_transport_udpv4.h"

/******************************************************************************
 *                          Agent transport
 ******************************************************************************/

typedef struct NANODllExport NANO_XRCE_Udpv4AgentTransportPropertiesI
{
    NANO_XRCE_AgentTransportProperties base;
    NANO_OSAPI_Udpv4SocketProperties socket;
    NANO_OSAPI_Udpv4SocketProperties metadata_socket;
} NANO_XRCE_Udpv4AgentTransportProperties;

#define NANO_XRCE_UDPV4AGENTTRANSPORTPROPERTIES_INITIALIZER \
{\
    NANO_XRCE_AGENTTRANSPORTPROPERTIES_INITIALIZER, /* base */\
    NANO_OSAPI_UDPV4SOCKETPROPERTIES_INITIALIZER, /* socket */\
    NANO_OSAPI_UDPV4SOCKETPROPERTIES_INITIALIZER  /* metadata_socket */\
}

typedef struct NANODllExport NANO_XRCE_Udpv4AgentTransportI
{
    NANO_XRCE_AgentTransport base;
    struct RTIOsapiThread *recv_thread;
    struct RTIOsapiThread *metadata_thread;
    struct RTIOsapiSemaphore *sem_thread_exit;
    NANO_OSAPI_Udpv4Socket client_socket;
    NANO_OSAPI_Udpv4Socket metadata_socket;
    NANO_MessageBufferPool recv_pool;
    NANO_XRCE_TransportLocatorMedium wakeup_address;
    NANO_XRCE_TransportLocatorMedium bind_address;
    NANO_XRCE_TransportLocatorMedium metadata_address;
    NANO_bool active;
} NANO_XRCE_Udpv4AgentTransport;

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_initialize(
    NANO_XRCE_AgentTransport *const self,
    const NANO_XRCE_AgentTransportListener *const listener,
    const NANO_XRCE_AgentTransportProperties *const properties,
    const NANO_XRCE_AgentTransportImplProperties *const impl_properties);

void
NANO_XRCE_Udpv4AgentTransport_finalize(
    NANO_XRCE_AgentTransport *const self);

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_listen(
    NANO_XRCE_AgentTransport *const self);

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_close(
    NANO_XRCE_AgentTransport *const self);

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_send_to(
    NANO_XRCE_AgentTransport *const self,
    NANO_XRCE_AgentTransportBindEntry *const client_entry,
    NANO_MessageBuffer *const msg);

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_send_direct(
    NANO_XRCE_AgentTransport *const self,
    const NANO_XRCE_TransportLocator *const locator,
    NANO_MessageBuffer *const msg);

void
NANO_XRCE_Udpv4AgentTransport_return(
    NANO_XRCE_AgentTransport *const self,
    NANO_MessageBuffer *const msg);


#define NANO_XRCE_UDPV4AGENTTRANSPORT_INITIALIZER \
{\
    {\
        NANO_XRCE_Udpv4AgentTransport_initialize, \
        NANO_XRCE_Udpv4AgentTransport_finalize, \
        NANO_XRCE_Udpv4AgentTransport_listen, \
        NANO_XRCE_Udpv4AgentTransport_close, \
        NANO_XRCE_Udpv4AgentTransport_send_to, \
        NANO_XRCE_Udpv4AgentTransport_send_direct, \
        NANO_XRCE_Udpv4AgentTransport_return, \
        NANO_XRCE_AGENTTRANSPORT_COMMON_INITIALIZER /* common fields */\
    },\
    NULL, /* recv_thread */\
    NULL, /* metadata_thread */\
    NULL, /* sem_thread_exit */\
    NANO_OSAPI_UDPV4SOCKET_INITIALIZER, /* client_socket */\
    NANO_OSAPI_UDPV4SOCKET_INITIALIZER, /* metadata_socket */\
    NANO_MESSAGEBUFFERPOOL_INITIALIZER, /* recv_pool */\
    NANO_XRCE_TRANSPORTLOCATORMEDIUM_INITIALIZER, /* wakeup_address */\
    NANO_XRCE_TRANSPORTLOCATORMEDIUM_INITIALIZER, /* bind_address */\
    NANO_XRCE_UDPV4CLIENTTRANSPORT_METADATA_LOCATOR_INITIALIZER, /* metadata_address */\
    NANO_BOOL_TRUE /* active */\
}



#endif /* nano_agent_transport_udpv4_h */