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

#ifndef ProxyClient_h
#define ProxyClient_h

#include "nano/nano_agent.h"
#include "dds_agent/dds_agent.h"

#include "StreamStorage.h"

typedef struct NANODllExport NANO_XRCE_ProxyClientRequestI
    NANO_XRCE_ProxyClientRequest;


struct NANO_XRCE_ProxyClientI
{
    struct REDAInlineListNode node;
    NANO_XRCE_Session session;
    NANO_XRCE_ProxyClientTransport transport;
    D2S2_ClientSession *dds_session;
    NANO_XRCE_SessionStorageRecord *storage;
    struct REDAInlineList stream_storage;
    NANO_XRCE_Agent *agent;
    struct REDAInlineList forwards;
    struct REDAInlineList forward_replies;
    NANO_bool disposed;
    NANO_XRCE_ProxyClientRequest *dispose_request;
    D2S2_ClientSessionEvent *event_hb;
    NANO_bool cache_mapping;
    struct REDAFastBufferPool *requests_pool;
    struct REDAFastBufferPool *forwards_pool;
    struct REDAFastBufferPool *writes_pool;
};

#define NANO_XRCE_PROXYCLIENT_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NANO_XRCE_SESSION_INITIALIZER, /* session */\
    NANO_XRCE_PROXYCLIENTTRANSPORT_INITIALIZER, /* transport */\
    NULL, /* dds_session */\
    NULL, /* storage */\
    REDA_INLINE_LIST_EMPTY, /* stream_storage */\
    NULL, /* agent */\
    REDA_INLINE_LIST_EMPTY, /* forwards */\
    REDA_INLINE_LIST_EMPTY, /* forward_replies */\
    NANO_BOOL_FALSE, /* disposed */\
    NULL, /* dispose_request */\
    NULL, /* event_hb */\
    NANO_BOOL_FALSE, /* cache_mapping */\
    NULL, /* requests_pool */\
    NULL, /* forwards_pool */\
    NULL  /* writes_pool */\
}

NANO_RetCode
NANO_XRCE_ProxyClient_initialize(
    NANO_XRCE_ProxyClient *const self,
    const NANO_XRCE_ClientRepresentation *const client_repr,
    NANO_XRCE_Agent *const agent);

void
NANO_XRCE_ProxyClient_finalize(NANO_XRCE_ProxyClient *const self);

NANO_RetCode
NANO_XRCE_ProxyClient_allocate_stream_storage(
    NANO_XRCE_ProxyClient *const self,
    NANO_XRCE_StreamStorageRecord **const record_out);

void
NANO_XRCE_ProxyClient_release_stream_storage(
    NANO_XRCE_ProxyClient *const self,
    NANO_XRCE_StreamStorageRecord *const record);

NANO_RetCode
NANO_XRCE_ProxyClient_dismiss_forward_request(
    NANO_XRCE_ProxyClient *self,
    const NANO_XRCE_StreamId stream_id,
    const NANO_XRCE_SeqNum *const data_sn);

#endif /* ProxyClient_h */