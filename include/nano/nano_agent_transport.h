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

#ifndef nano_agent_transport_h
#define nano_agent_transport_h



/* forward declarations */
typedef struct NANODllExport NANO_XRCE_AgentI NANO_XRCE_Agent;
typedef struct NANODllExport NANO_XRCE_ProxyClientI NANO_XRCE_ProxyClient;

/******************************************************************************
 *                            AgentTransport (forward)
 ******************************************************************************/
/**
 * @defgroup nanocore_api_xrce_transport_agent AgentTransport
 * @addtogroup nanocore_api_xrce_transport_agent
 * @{
 */

/**
 * @brief A transport which acts as a server and communicates with multiple
 * clients which connect to it.
 */
typedef struct NANODllExport NANO_XRCE_AgentTransportI
    NANO_XRCE_AgentTransport;

/** @} *//* nanocore_api_xrce_transport_agent */


/******************************************************************************
 *                          AgentTransportProperties
 ******************************************************************************/


typedef struct NANODllExport NANO_XRCE_AgentTransportPropertiesI
{
    NANO_XRCE_TransportLocator bind_address;
    NANO_XRCE_TransportLocator metadata_address;
    NANO_bool enable_discovery;
    NANO_usize mtu_max;
} NANO_XRCE_AgentTransportProperties;

#define NANO_XRCE_AGENTTRANSPORTPROPERTIES_INITIALIZER \
{\
    NANO_XRCE_TRANSPORTLOCATOR_INITIALIZER, /* bind_address */\
    NANO_XRCE_TRANSPORTLOCATOR_INITIALIZER, /* metadata_address */\
    NANO_BOOL_FALSE, /* enable_discovery */\
    NANO_AGENT_DEFAULT_TRANSPORT_MTU_MAX, /* mtu_max */\
}


typedef struct NANODllExport NANO_XRCE_AgentTransportImplPropertiesI
{
    struct REDAExclusiveArea *transport_ea;
    NANO_XRCE_Agent *agent;
} NANO_XRCE_AgentTransportImplProperties;

#define NANO_XRCE_AGENTTRANSPORTIMPLPROPERTIES_INITIALIZER \
{\
    NULL, /* ea */\
    NULL /* agent */\
}

/******************************************************************************
 *                          AgentTransportListener
 ******************************************************************************/

typedef struct NANODllExport NANO_XRCE_AgentTransportListenerI
    NANO_XRCE_AgentTransportListener;

typedef void
    (*NANO_XRCE_AgentTransportListener_OnMessageReceivedFn)(
        NANO_XRCE_AgentTransportListener *const self,
        NANO_XRCE_AgentTransport *const transport,
        const NANO_XRCE_TransportLocator *const src,
        NANO_MessageBuffer *const msg,
        const NANO_usize msg_len,
        NANO_bool *const retained);

struct NANO_XRCE_AgentTransportListenerI
{
    NANO_XRCE_AgentTransportListener_OnMessageReceivedFn on_message_received;

    void *user_data;
};

#define NANO_XRCE_AGENTTRANSPORTLISTENER_INITIALIZER \
{ \
    NULL, /* on_message_received */ \
    NULL /* user_data */\
}

NANO_bool
NANO_XRCE_AgentTransportListener_is_valid(
    const NANO_XRCE_AgentTransportListener *const self);

#define NANO_XRCE_AgentTransportListener_is_valid(s_) \
   ((s_)->on_message_received != NULL)

void 
NANO_XRCE_AgentTransportListener_on_message_received(
    NANO_XRCE_AgentTransportListener *const self,
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_TransportLocator *const src,
    NANO_MessageBuffer *const msg,
    const NANO_usize msg_len,
    NANO_bool *const retained);

#define NANO_XRCE_AgentTransportListener_on_message_received(s_,t_,src_,m_,l_,r_) \
    ((s_)->on_message_received((s_),(t_),(src_),(m_),(l_),(r_)))


/******************************************************************************
 *                          AgentTransportBindEntry
 ******************************************************************************/

typedef struct NANODllExport NANO_XRCE_AgentTransportBindEntryI
{
    struct REDAInlineListNode node;
    NANO_XRCE_AgentTransport *transport;
    NANO_XRCE_TransportLocator locator;
    NANO_XRCE_ClientKey key;
} NANO_XRCE_AgentTransportBindEntry;

#define NANO_XRCE_AGENTTRANSPORTBINDENTRY_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* transport */\
    NANO_XRCE_TRANSPORTLOCATOR_INITIALIZER, /* locator */\
    NANO_XRCE_CLIENTKEY_INVALID /* key */\
}

/******************************************************************************
 *                            AgentTransport
 ******************************************************************************/
/**
 * @defgroup nanocore_api_xrce_transport_agent AgentTransport
 * @addtogroup nanocore_api_xrce_transport_agent
 * @{
 */

/**
 * @brief TODO
 * 
 */
typedef NANO_RetCode
    (*NANO_XRCE_AgentTransport_InitializeFn)(
        NANO_XRCE_AgentTransport *const self,
        const NANO_XRCE_AgentTransportListener *const listener,
        const NANO_XRCE_AgentTransportProperties *const properties,
        const NANO_XRCE_AgentTransportImplProperties *const impl_properties);

/**
 * @brief TODO
 * 
 */
typedef void
    (*NANO_XRCE_AgentTransport_FinalizeFn)(
        NANO_XRCE_AgentTransport *const self);

/**
 * @brief TODO
 * 
 */
typedef NANO_RetCode
    (*NANO_XRCE_AgentTransport_ListenFn)(
        NANO_XRCE_AgentTransport *const self);

/**
 * @brief TODO
 * 
 */
typedef NANO_RetCode
    (*NANO_XRCE_AgentTransport_CloseFn)(
        NANO_XRCE_AgentTransport *const self);

/**
 * @brief TODO
 * 
 */
typedef NANO_RetCode
    (*NANO_XRCE_AgentTransport_SendToFn)(
        NANO_XRCE_AgentTransport *const self,
        NANO_XRCE_AgentTransportBindEntry *const client_entry,
        NANO_MessageBuffer *const msg);

/**
 * @brief TODO
 * 
 */
typedef NANO_RetCode
    (*NANO_XRCE_AgentTransport_SendDirectFn)(
        NANO_XRCE_AgentTransport *const self,
        const NANO_XRCE_TransportLocator *const locator,
        NANO_MessageBuffer *const msg);

typedef void
    (*NANO_XRCE_AgentTransport_ReturnMessageFn)(
        NANO_XRCE_AgentTransport *const self,
        NANO_MessageBuffer *const msg);

struct NANO_XRCE_AgentTransportI
{
    NANO_XRCE_AgentTransport_InitializeFn initialize;
    NANO_XRCE_AgentTransport_FinalizeFn finalize;
    NANO_XRCE_AgentTransport_ListenFn listen;
    NANO_XRCE_AgentTransport_CloseFn close;
    NANO_XRCE_AgentTransport_SendToFn send_to;
    NANO_XRCE_AgentTransport_SendDirectFn send_direct;
    NANO_XRCE_AgentTransport_ReturnMessageFn return_msg;
    NANO_XRCE_AgentTransportListener listener;
    NANO_XRCE_Agent *agent;
};

/**
 * @brief TODO
 * 
 */
#define NANO_XRCE_AGENTTRANSPORT_INITIALIZER \
{\
    NULL, /* initialize */ \
    NULL, /* finalize */ \
    NULL, /* listen */ \
    NULL, /* close */ \
    NULL, /* send_to */ \
    NULL, /* send_direct */ \
    NULL, /* return_msg */ \
    NANO_XRCE_AGENTTRANSPORT_COMMON_INITIALIZER /* other fields */\
}

#define NANO_XRCE_AGENTTRANSPORT_COMMON_INITIALIZER \
    NANO_XRCE_AGENTTRANSPORTLISTENER_INITIALIZER, /* listener */ \
    NULL /* agent */

/**
 * @brief TODO
 * 
 * @param self 
 * @return NANO_bool 
 */
NANO_bool
NANO_XRCE_AgentTransport_is_valid(
    const NANO_XRCE_AgentTransport *const self);

#define NANO_XRCE_AgentTransport_is_valid(s_) \
   ((s_)->listen != NULL && \
    (s_)->close != NULL && \
    (s_)->initialize != NULL && \
    (s_)->finalize != NULL && \
    (s_)->send_to != NULL && \
    (s_)->send_direct != NULL && \
    (s_)->return_msg != NULL)

/**
 * @brief Initialize an agent transport.
 * 
 * @param self The transport to initialize.
 * @param properties Optional properties.
 * @return NANO_RETCODE_OK if the transport was successfully initialized.
 */
NANO_RetCode
NANO_XRCE_AgentTransport_initialize(
    NANO_XRCE_AgentTransport *const self,
    const NANO_XRCE_AgentTransportListener *const listener,
    const NANO_XRCE_AgentTransportProperties *const properties,
    const NANO_XRCE_AgentTransportImplProperties *const impl_properties);

#define NANO_XRCE_AgentTransport_initialize(s_,l_,p_,pi_) \
    ((s_)->initialize((s_),(l_),(p_),(pi_)))

void
NANO_XRCE_AgentTransport_finalize(
    NANO_XRCE_AgentTransport *const self);

#define NANO_XRCE_AgentTransport_finalize(s_) \
    ((s_)->finalize((s_)))

NANO_RetCode
NANO_XRCE_AgentTransport_listen(
    NANO_XRCE_AgentTransport *const self);

#define NANO_XRCE_AgentTransport_listen(s_) \
    ((s_)->listen((s_)))

NANO_RetCode
NANO_XRCE_AgentTransport_close(
    NANO_XRCE_AgentTransport *const self);

#define NANO_XRCE_AgentTransport_close(s_) \
    ((s_)->close((s_)))

/**
 * @brief TODO
 * 
 */
NANO_RetCode
NANO_XRCE_AgentTransport_send_to(
    NANO_XRCE_AgentTransport *const self,
    NANO_XRCE_AgentTransportBindEntry *const client_entry,
    NANO_MessageBuffer *const msg);

#define NANO_XRCE_AgentTransport_send_to(s_,e_,m_) \
    ((s_)->send_to((s_),(e_),(m_)))


NANO_RetCode
NANO_XRCE_AgentTransport_send_direct(
    NANO_XRCE_AgentTransport *const self,
    const NANO_XRCE_TransportLocator *const locator,
    NANO_MessageBuffer *const msg);

#define NANO_XRCE_AgentTransport_send_direct(s_,l_,m_) \
    ((s_)->send_direct((s_),(l_),(m_)))

void
NANO_XRCE_AgentTransport_return(
    NANO_XRCE_AgentTransport *const self,
    NANO_MessageBuffer *const msg);

#define NANO_XRCE_AgentTransport_return(s_,m_) \
    ((s_)->return_msg((s_),(m_)))

typedef struct NANODllExport NANO_XRCE_ProxyClientTransportProperties
{
    NANO_XRCE_ClientTransportProperties base;
    NANO_XRCE_ClientKey key;
    NANO_XRCE_SessionId id;
} NANO_XRCE_ProxyClientTransportProperties;

#define NANO_XRCE_PROXYCLIENTTRANSPORTPROPERTIES_INITIALIZER \
{\
    NANO_XRCE_CLIENTTRANSPORTPROPERTIES_INITIALIZER, /* base */\
    NANO_XRCE_CLIENTKEY_INVALID, /* key */\
    NANO_XRCE_SESSIONID_NONE_WITH_CLIENT /* id */\
}

typedef struct NANODllExport NANO_XRCE_ProxyClientTransportI
{
    NANO_XRCE_ClientTransport base;
    NANO_XRCE_ClientKey key;
    NANO_XRCE_SessionId id;
    NANO_XRCE_AgentTransportBindEntry bind_entry;
    NANO_XRCE_Agent *agent;
    NANO_XRCE_ProxyClient *client;
} NANO_XRCE_ProxyClientTransport;


#define NANO_XRCE_PROXYCLIENTTRANSPORT_INITIALIZER \
{\
    {\
        NANO_XRCE_ProxyClientTransport_initialize,\
        NANO_XRCE_ProxyClientTransport_finalize,\
        NANO_XRCE_ProxyClientTransport_process_input,\
        NANO_XRCE_ProxyClientTransport_flush_output,\
        NANO_XRCE_ProxyClientTransport_return_message,\
        NANO_XRCE_ProxyClientTransport_update_locator, \
        NANO_XRCE_CLIENTTRANSPORT_COMMON_INITIALIZER \
    }, /* base */\
    NANO_XRCE_CLIENTKEY_INVALID, /* key */\
    NANO_XRCE_SESSIONID_NONE_WITH_CLIENT, /* id */\
    NANO_XRCE_AGENTTRANSPORTBINDENTRY_INITIALIZER, /* bind_entry */\
    NULL, /* agent */ \
    NULL,  /* client  */ \
}

NANODllExport
NANO_RetCode
NANO_XRCE_ProxyClientTransport_initialize(
    NANO_XRCE_ClientTransport *const self,
    const NANO_XRCE_ClientTransportListener *const listener,
    const NANO_XRCE_ClientTransportProperties *const properties);

NANODllExport
void
NANO_XRCE_ProxyClientTransport_finalize(
    NANO_XRCE_ClientTransport *const self);

NANODllExport
NANO_RetCode
NANO_XRCE_ProxyClientTransport_process_input(
    NANO_XRCE_ClientTransport *const self,
    NANO_u32 max_messages,
    NANO_Timeout timeout_ms);

NANODllExport
void
NANO_XRCE_ProxyClientTransport_return_message(
    NANO_XRCE_ClientTransport *const self,
    NANO_MessageBuffer *const msg);

NANODllExport
void
NANO_XRCE_ProxyClientTransport_flush_output(
    NANO_XRCE_ClientTransport *const self);

NANODllExport
NANO_RetCode
NANO_XRCE_ProxyClientTransport_update_locator(
    NANO_XRCE_ClientTransport *const self,
    const NANO_XRCE_ClientTransportLocatorType locator_type,
    const NANO_XRCE_TransportLocator *const locator);

void
NANO_XRCE_ProxyClientTransport_on_message_received(
    NANO_XRCE_ProxyClientTransport *const self,
    NANO_MessageBuffer *const msg,
    NANO_usize msg_len,
    NANO_bool *const retained);

NANO_XRCE_ClientKey*
NANO_XRCE_ProxyClientTransport_key(
    NANO_XRCE_ProxyClientTransport *const self);

#define NANO_XRCE_ProxyClientTransport_key(s_) \
    (&(s_)->key)

NANO_RetCode
NANO_XRCE_ProxyClientTransport_set_bind_entry(
    NANO_XRCE_ProxyClientTransport *const self,
    NANO_XRCE_AgentTransport *const src_transport,
    const NANO_XRCE_TransportLocator *const src_locator);


/******************************************************************************
 *                          MessageBufferPool
 ******************************************************************************/
typedef struct NANODllExport NANO_MessageBufferPoolI
{
    struct REDAFastBufferPool *pool;
} NANO_MessageBufferPool;

#define NANO_MESSAGEBUFFERPOOL_INITIALIZER \
{\
    NULL /* pool */\
}

NANODllExport
NANO_RetCode
NANO_MessageBufferPool_initialize(
    NANO_MessageBufferPool *const self,
    const NANO_usize data_size);

NANODllExport
void
NANO_MessageBufferPool_finalize(NANO_MessageBufferPool *const self);

NANODllExport
NANO_usize
NANO_MessageBufferPool_data_size(NANO_MessageBufferPool *const self);

#define NANO_MessageBufferPool_data_size(s_) \
    NANO_MESSAGEBUFFER_MAX_INLINE_PAYLOAD(\
        (NANO_usize)REDAFastBufferPool_getBufferSize(self->pool))

NANODllExport
NANO_RetCode
NANO_MessageBufferPool_allocate(
    NANO_MessageBufferPool *const self,
    NANO_MessageBuffer **const msg_out);


NANODllExport
void
NANO_MessageBufferPool_release(
    NANO_MessageBufferPool *const self,
    NANO_MessageBuffer *const msg);

/** @} *//* nanocore_api_xrce_transport_agent */


/******************************************************************************
 *                          Transport Implementations
 ******************************************************************************/

#if NANO_FEAT_TRANSPORT_PLUGIN_UDPV4
#include "nano/nano_agent_transport_udpv4.h"
#endif /* NANO_FEAT_TRANSPORT_PLUGIN_UDPV4 */

#if NANO_FEAT_TRANSPORT_PLUGIN_UDPV6
#include "nano/nano_agent_transport_udpv6.h"
#endif /* NANO_FEAT_TRANSPORT_PLUGIN_UDPV6 */

#if NANO_FEAT_TRANSPORT_PLUGIN_TCPV4
#include "nano/nano_agent_transport_tcpv4.h"
#endif /* NANO_FEAT_TRANSPORT_PLUGIN_TCPV4 */

#if NANO_FEAT_TRANSPORT_PLUGIN_TCPV6
#include "nano/nano_agent_transport_tcpv6.h"
#endif /* NANO_FEAT_TRANSPORT_PLUGIN_TCPV6 */

#if NANO_FEAT_TRANSPORT_PLUGIN_SERIAL
#include "nano/nano_agent_transport_serial.h"
#endif /* NANO_FEAT_TRANSPORT_PLUGIN_SERIAL */


#endif /* nano_agent_transport_h */