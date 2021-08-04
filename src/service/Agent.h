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

#ifndef Agent_h
#define Agent_h

#include "nano/nano_agent.h"

#include "dds_agent/dds_agent.h"

#include "ProxyClient.h"

struct NANO_XRCE_ProxyClientRequestI
{
    struct REDAInlineListNode node;
    NANO_XRCE_ProxyClient *client;
    NANO_XRCE_MessageHeader msg_hdr;
    NANO_XRCE_SubmessageHeader submsg_hdr;
    NANO_XRCE_RequestId request;
    NANO_XRCE_ObjectId object_id;
    NANO_bool complete;
    NANO_bool reply;
    NANO_RetCode rc;
    NANO_MessageBuffer *reply_mbuf;
};

#define NANO_XRCE_PROXYCLIENTREQUEST_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* client */\
    NANO_XRCE_MESSAGEHEADER_INITIALIZER, /* msg_hdr */\
    NANO_XRCE_SUBMESSAGEHEADER_INITIALIZER, /* msg_hdr */\
    NANO_XRCE_REQUESTID_INVALID, /* request */\
    NANO_XRCE_OBJECTID_INVALID, /* object_id */\
    NANO_BOOL_FALSE, /* complete */\
    NANO_BOOL_FALSE, /* reply */\
    NANO_RETCODE_ERROR, /* rc */\
    NULL, /* reply_mbuf */\
}

typedef struct NANODllExport NANO_XRCE_WriteRequestI
{
    NANO_XRCE_ProxyClientRequest *req;
    NANO_XRCE_WriteDataPayload *payload;
    D2S2_DataRepresentation data_repr;
} NANO_XRCE_WriteRequest;

#define NANO_XRCE_WRITEREQUEST_INITIALIZER \
{\
    NULL, /* base */\
    NULL, /* payload */\
    D2S2_DATAREPRESENTATION_INITIALIZER /* data_repr */\
}

NANO_bool
NANO_XRCE_WriteRequest_needs_reply(NANO_XRCE_WriteRequest *const self);

#define NANO_XRCE_WriteRequest_needs_reply(s_) \
    NANO_XRCE_SubmessageFlags_WRITEDATA_confirm((s_)->base.submsg_hdr.flags)

typedef struct NANODllExport NANO_XRCE_ReaderStateI
{
    NANO_XRCE_ProxyClientRequest *req;
    NANO_XRCE_DataFormat read_fmt;
    D2S2_ReadSpecification read_spec_dds;
    NANO_XRCE_StreamId stream_id;
    NANO_bool forward;
} NANO_XRCE_ReaderState;

#define NANO_XRCE_READERSTATE_INITIALIZER \
{\
    NULL, /* req */\
    NANO_XRCE_FORMAT_INVALID, /* read_fmt */\
    D2S2_READSPECIFICATION_INITIALIZER, /* read_spec_dds */\
    NANO_XRCE_STREAMID_NONE, /* stream_id */\
    NANO_BOOL_FALSE /* forward */\
}


NANO_bool
NANO_XRCE_ReaderState_needs_reply(NANO_XRCE_ReaderState *const self);

#define NANO_XRCE_ReaderState_needs_reply(s_) \
    NANO_XRCE_SubmessageFlags_READDATA_confirm((s_)->base.submsg_hdr.flags)

typedef struct NANODllExport NANO_XRCE_ServiceRequestStateI
{
    NANO_XRCE_ProxyClientRequest *req;
    NANO_XRCE_StreamId stream_id;
    NANO_XRCE_ServiceRequestFlags request_flags;
    NANO_XRCE_ServiceReplyStatus reply_flags;
    D2S2_Buffer request_data;
    D2S2_Buffer reply_data;
} NANO_XRCE_ServiceRequestState;

#define NANO_XRCE_SERVICEREQUESTSTATE_INITIALIZER \
{\
    NULL, /* req */\
    NANO_XRCE_STREAMID_NONE, /* stream_id */\
    NANO_XRCE_SERVICEREQUESTFLAGS_UNKNOWN, /* request_flags */\
    NANO_XRCE_SERVICEREPLYSTATUS_UNKNOWN, /* reply_flags */\
    D2S2_BUFFER_INITIALIZER, /* request_data */\
    D2S2_BUFFER_INITIALIZER  /* reply_data */\
}

#define NANO_XRCE_ServiceRequestState_needs_reply(s_) \
    NANO_XRCE_SubmessageFlags_SERVICEREQUEST_confirm((s_)->base.submsg_hdr.flags)

typedef struct NANODllExport NANO_XRCE_ForwardDataRequestI
{
    struct REDAInlineListNode node;
    const D2S2_ReceivedData *rcvd_data;
    NANO_XRCE_ObjectId reader_id;
    NANO_XRCE_StreamId stream_id;
    NANO_XRCE_SeqNum sn;
} NANO_XRCE_ForwardDataRequest;

#define NANO_XRCE_FORWARDDATAREQUEST_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* rcvd_data */\
    NANO_XRCE_OBJECTID_INVALID, /* reader_id */\
    NANO_XRCE_STREAMID_NONE, /* stream_id */\
    NANO_XRCE_SEQNUM_INITIALIZER, /* sn */\
}

typedef struct NANODllExport NANO_XRCE_ExternalServiceResourceStateI
{
    NANO_XRCE_ProxyClientRequest *req;
    NANO_bool forward;
} NANO_XRCE_ExternalServiceResourceState;


#define NANO_XRCE_EXERNALSERVICERESOURCESTATE_INITIALIZER \
{\
    NULL, /* req */\
    NANO_BOOL_FALSE /* forward */\
}

typedef struct NANODllExport NANO_XRCE_AgentTransportRecordI
{
    struct REDAInlineListNode node;
    NANO_XRCE_AgentTransport *transport;
    struct REDAExclusiveArea *ea;
} NANO_XRCE_AgentTransportRecord;

#define NANO_XRCE_AGENTTRANSPORTRECORD_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* transport */\
    NULL /* ea */\
}

typedef struct NANODllExport NANO_XRCE_AgentMessageReceivedI \
{
    struct REDAInlineListNode node;
    NANO_XRCE_AgentTransport *transport;
    NANO_XRCE_ProxyClient *client;
    NANO_MessageBuffer *msg;
} NANO_XRCE_AgentMessageReceived;

#define NANO_XRCE_AGENTMESSAGERECEIVED_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* transport */\
    NULL, /* client */\
    NULL  /* msg */\
}

typedef NANO_u16 NANO_XRCE_AgentFlags;

#define NANO_XRCE_AGENTFLAGS_DEFAULT            (0)
#define NANO_XRCE_AGENTFLAGS_ENABLED            (0x01 << 0)
#define NANO_XRCE_AGENTFLAGS_EXIT               (0x01 << 1)

struct NANO_XRCE_AgentI
{
    D2S2_AgentServerInterface base;
    
    struct REDAInlineList transports;

    struct REDAFastBufferPool *clients_pool;
    struct REDAFastBufferPool *transports_pool;
    struct REDAFastBufferPool *reads_pool;
    struct REDAFastBufferPool *service_requests_pool;
    struct REDAFastBufferPool *pool_storage_stream;
    struct REDAFastBufferPool *pool_storage_session;
    
    NANO_XRCE_AgentFlags flags;
    
    NANO_XRCE_AgentProperties props;
    struct RTIOsapiSemaphore *mappings_mutex;
};

extern const D2S2_AgentServerInterfaceApi NANO_XRCE_gv_AgentInterfaceApi;

#define NANO_XRCE_AGENT_INITIALIZER \
{\
    D2S2_AGENTSERVERINTERFACE_INITIALIZER, /* base */\
    REDA_INLINE_LIST_EMPTY, /* transports */\
    NULL, /* clients_pool */\
    NULL, /* transports_pool */\
    NULL, /* reads_pool */\
    NULL, /* service_requests_pool */\
    NULL, /* pool_storage_stream */\
    NULL, /* pool_storage_session */\
    NANO_XRCE_AGENTFLAGS_DEFAULT, /* flags */\
    NANO_XRCE_AGENTPROPERTIES_INITIALIZER, /* props */\
    NULL /* mappings_mutex */\
}

NANODllExport
NANO_RetCode
NANO_XRCE_Agent_initialize(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_AgentProperties *const properties);

NANODllExport
void
NANO_XRCE_Agent_finalize(NANO_XRCE_Agent *const self);

NANO_bool
NANO_XRCE_Agent_flags(
    NANO_XRCE_Agent *const self,
    const NANO_XRCE_AgentFlags flags);

#define NANO_XRCE_Agent_flags(s_,f_) \
    (((s_)->flags & (NANO_XRCE_AgentFlags)(f_))?\
        NANO_BOOL_TRUE : NANO_BOOL_FALSE)

void
NANO_XRCE_Agent_flags_set(
    NANO_XRCE_Agent *const self,
    const NANO_XRCE_AgentFlags flags);

#define NANO_XRCE_Agent_flags_set(s_,f_) \
{\
    (s_)->flags |= (NANO_XRCE_AgentFlags)(f_);\
}

void
NANO_XRCE_Agent_flags_unset(
    NANO_XRCE_Agent *const self,
    const NANO_XRCE_AgentFlags flags);

#define NANO_XRCE_Agent_flags_unset(s_,f_) \
{\
    (s_)->flags &= ~((NANO_XRCE_AgentFlags)(f_));\
}

NANO_RetCode
NANO_XRCE_Agent_reply_to_operation(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_RetCode op_rc);

NANO_RetCode
NANO_XRCE_Agent_on_submsg_create(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_CreatePayload *const submsg);

NANO_RetCode
NANO_XRCE_Agent_on_submsg_delete(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_DeletePayload *const submsg);

NANO_RetCode
NANO_XRCE_Agent_on_submsg_write(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_WriteDataPayload *const submsg);

NANO_RetCode
NANO_XRCE_Agent_on_submsg_read(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_ReadDataPayload *const submsg);

NANO_RetCode
NANO_XRCE_Agent_on_submsg_getinfo(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_GetInfoPayload *const submsg);

NANO_RetCode
NANO_XRCE_Agent_on_submsg_servicereq(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_ServiceRequestPayload *const submsg);

NANO_RetCode
NANO_XRCE_Agent_delete_client_session(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client);

NANO_RetCode
NANO_XRCE_Agent_dispose_client_session(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_ProxyClientRequest *const request);

NANO_RetCode
NANO_XRCE_Agent_reply_to_connection(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client);

NANO_XRCE_ProxyClientRequest*
NANO_XRCE_Agent_new_client_request(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    const NANO_XRCE_MessageHeader *const msg_hdr,
    const NANO_XRCE_SubmessageHeader *const submsg_hdr,
    const NANO_XRCE_RequestId *const req_id,
    const NANO_XRCE_ObjectId *const obj_id,
    const NANO_bool reply);

void
NANO_XRCE_Agent_return_client_request(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request);

DDS_ReturnCode_t
NANO_XRCE_Agent_on_data_received(
    NANO_XRCE_Agent *const self,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const reader_data,
    const D2S2_ReceivedData *const data,
    DDS_Boolean *const retained_out,
    DDS_Boolean *const try_again_out);

// void
// NANO_XRCE_Agent_on_periodic_heartbeat(
//     D2S2_Agent *const agent,
//     D2S2_AgentServerInterface *const src,
//     D2S2_ClientSession *const session,
//     void *const session_data,
//     void *const event,
//     void *const listener);

NANO_XRCE_Stream*
NANO_XRCE_Agent_lookup_stream(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    const NANO_XRCE_StreamId stream_id);

void
NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_Stream *const req_stream,
    const D2S2_AttachedResourceId reader_id,
    const DDS_Boolean confirmed);

NANO_RetCode
NANO_XRCE_Agent_find_client_locator_mapping(
    NANO_XRCE_Agent *const self,
    const NANO_XRCE_TransportLocator *const locator,
    NANO_XRCE_ClientLocatorMapping **const mapping_out);

NANO_RetCode
NANO_XRCE_Agent_allocate_reply_message(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_Stream *reply_stream,
    const NANO_XRCE_StreamId reply_stream_id,
    const NANO_usize payload_size,
    const NANO_u8 *const user_payload,
    NANO_MessageBuffer **const payload_out,
    NANO_XRCE_Stream **const reply_stream_out);

void
NANO_XRCE_Agent_release_reply_message(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    const NANO_XRCE_StreamId reply_stream_id,
    NANO_MessageBuffer *const payload);

#define NANO_XRCE_RepresentationFormat_to_dds(s_) \
    (((s_) == NANO_XRCE_REPRESENTATION_AS_XML_STRING)?\
        D2S2_RESOURCEREPRESENTATIONFORMAT_XML : \
    ((s_) == NANO_XRCE_REPRESENTATION_BY_REFERENCE)?\
        D2S2_RESOURCEREPRESENTATIONFORMAT_REF : \
    ((s_) == NANO_XRCE_REPRESENTATION_IN_BINARY)?\
        D2S2_RESOURCEREPRESENTATIONFORMAT_BIN : \
        D2S2_RESOURCEREPRESENTATIONFORMAT_UNKNOWN)

#define NANO_XRCE_ObjectKind_to_dds(s_) \
    ((s_) == NANO_XRCE_OBJK_PARTICIPANT)?\
        D2S2_RESOURCEKIND_DOMAINPARTICIPANT : \
    ((s_) == NANO_XRCE_OBJK_TOPIC)?\
        D2S2_RESOURCEKIND_TOPIC : \
    ((s_) == NANO_XRCE_OBJK_PUBLISHER)?\
        D2S2_RESOURCEKIND_PUBLISHER : \
    ((s_) == NANO_XRCE_OBJK_SUBSCRIBER)?\
        D2S2_RESOURCEKIND_SUBSCRIBER : \
    ((s_) == NANO_XRCE_OBJK_DATAWRITER)?\
        D2S2_RESOURCEKIND_DATAWRITER : \
    ((s_) == NANO_XRCE_OBJK_DATAREADER)?\
        D2S2_RESOURCEKIND_DATAREADER : \
    ((s_) == NANO_XRCE_OBJK_TYPE)?\
        D2S2_RESOURCEKIND_TYPE : \
    ((s_) == NANO_XRCE_OBJK_QOSPROFILE)?\
        D2S2_RESOURCEKIND_QOSPROFILE : \
    ((s_) == NANO_XRCE_OBJK_DOMAIN)?\
        D2S2_RESOURCEKIND_DOMAIN : \
    ((s_) == NANO_XRCE_OBJK_APPLICATION)?\
        D2S2_RESOURCEKIND_APPLICATION : \
        D2S2_RESOURCEKIND_UNKNOWN

#endif /* Agent_h */