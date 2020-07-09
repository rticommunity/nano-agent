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

#ifndef nano_agent_transport_serial_h
#define nano_agent_transport_serial_h


typedef struct NANODllExport NANO_XRCE_SerialAgentTransportPropertiesI
{
    NANO_XRCE_AgentTransportProperties base;
    NANO_OSAPI_SerialConnectionProperties connection;
    NANO_Timeout recv_timeout;
} NANO_XRCE_SerialAgentTransportProperties;

#define NANO_XRCE_SERIALAGENTTRANSPORTPROPERTIES_INITIALIZER \
{\
    NANO_XRCE_AGENTTRANSPORTPROPERTIES_INITIALIZER, /* base */\
    NANO_OSAPI_SERIALCONNECTIONPROPERTIES_INITIALIZER, /* connection */\
    NANO_AGENT_DEFAULT_SERIAL_RECV_TIMEOUT /* recv_timeout */\
}

typedef struct NANODllExport NANO_XRCE_SerialAgentTransportI
{
    NANO_XRCE_AgentTransport base;
    struct REDAInlineList recv_threads;
    struct RTIOsapiSemaphore *mutex;
    struct RTIOsapiSemaphore *sem_thread_exit;
    struct REDAFastBufferPool *recv_threads_pool;
    NANO_MessageBufferPool msg_pool;
    NANO_XRCE_SerialAgentTransportProperties props;
    NANO_bool active;
} NANO_XRCE_SerialAgentTransport;

NANO_RetCode
NANO_XRCE_SerialAgentTransport_initialize(
    NANO_XRCE_AgentTransport *const self,
    const NANO_XRCE_AgentTransportListener *const listener,
    const NANO_XRCE_AgentTransportProperties *const properties,
    const NANO_XRCE_AgentTransportImplProperties *const impl_properties);

void
NANO_XRCE_SerialAgentTransport_finalize(
    NANO_XRCE_AgentTransport *const self);

NANO_RetCode
NANO_XRCE_SerialAgentTransport_listen(
    NANO_XRCE_AgentTransport *const self);

NANO_RetCode
NANO_XRCE_SerialAgentTransport_close(
    NANO_XRCE_AgentTransport *const self);

NANO_RetCode
NANO_XRCE_SerialAgentTransport_send_to(
    NANO_XRCE_AgentTransport *const self,
    NANO_XRCE_AgentTransportBindEntry *const client_entry,
    NANO_MessageBuffer *const msg);

NANO_RetCode
NANO_XRCE_SerialAgentTransport_send_direct(
    NANO_XRCE_AgentTransport *const self,
    const NANO_XRCE_TransportLocator *const locator,
    NANO_MessageBuffer *const msg);

void
NANO_XRCE_SerialAgentTransport_return(
    NANO_XRCE_AgentTransport *const self,
    NANO_MessageBuffer *const msg);


#define NANO_XRCE_SERIALAGENTTRANSPORT_INITIALIZER \
{\
    {\
        NANO_XRCE_SerialAgentTransport_initialize, \
        NANO_XRCE_SerialAgentTransport_finalize, \
        NANO_XRCE_SerialAgentTransport_listen, \
        NANO_XRCE_SerialAgentTransport_close, \
        NANO_XRCE_SerialAgentTransport_send_to, \
        NANO_XRCE_SerialAgentTransport_send_direct, \
        NANO_XRCE_SerialAgentTransport_return, \
        NANO_XRCE_AGENTTRANSPORT_COMMON_INITIALIZER /* common fields */\
    },\
    REDA_INLINE_LIST_EMPTY, /* recv_threads */\
    NULL, /* mutex */\
    NULL, /* sem_thread_exit */\
    NULL, /* recv_threads_pool */\
    NANO_MESSAGEBUFFERPOOL_INITIALIZER, /* msg_pool */\
    NANO_XRCE_SERIALAGENTTRANSPORTPROPERTIES_INITIALIZER, /* props */\
    NANO_BOOL_TRUE /* active */\
}



#endif /* nano_agent_transport_serial_h */