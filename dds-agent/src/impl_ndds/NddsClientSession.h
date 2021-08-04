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

#ifndef NddsClientSession_h
#define NddsClientSession_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsResource.h"
#include "NddsExternalService.h"

typedef struct NDDSA_ClientI NDDSA_Client;
typedef struct NDDSA_ReadI NDDSA_Read;

typedef struct NDDSA_AttachedResourceI
{
    D2S2_AttachedResource base;
    struct REDAInlineListNode node;
    NDDSA_Resource *resource;
    NDDSA_ClientSession *session;
    void *user_data;
    NDDSA_Read *read_req;
    NDDSA_ExternalServiceRequest *svc_req;
    struct DDS_SequenceNumber_t last_sample_forwarded;
    struct DDS_SequenceNumber_t last_sample_returned;
} NDDSA_AttachedResource;

#define NDDSA_ATTACHEDRESOURCE_INITIALIZER \
{\
    D2S2_ATTACHEDRESOURCE_INITIALIZER, /* base */\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* resource */\
    NULL, /* session */\
    NULL, /* user_data */\
    NULL, /* read_req */\
    NULL, /* svc_req */\
    DDS_SEQUENCENUMBER_DEFAULT, /* last_sample_forwarded */\
    DDS_SEQUENCENUMBER_DEFAULT  /* last_sample_returned */\
}

#define NDDSA_AttachedResource_from_node(n_) \
    ((NDDSA_AttachedResource*)\
        (((unsigned char *)(n_)) - NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_AttachedResource,node)))


struct NDDSA_ClientSessionI
{
    D2S2_ClientSession base;
    NDDSA_Agent *agent;
    D2S2_AgentServerInterface *intf;
    struct REDAInlineList resources;
    D2S2_ClientSessionProperties props;
    void *user_data;
    RTIBool deleted;
    RTIBool timedout;
    RTIBool resetting;
    struct REDAInlineList rcvd_data_pool;
    struct RTIEventGeneratorListener timeout_listener;
    struct RTIEventGeneratorListenerStorage timeout_listener_storage;
    struct RTIOsapiSemaphore *sem_timeout;
    struct RTIOsapiSemaphore *sem_cleanup;
};

#define NDDSA_CLIENTSESSION_INITIALIZER \
{\
    D2S2_CLIENTSESSION_INITIALIZER, /* ref */\
    NULL, /* agent */\
    NULL, /* intf */\
    REDA_INLINE_LIST_EMPTY, /* resources */\
    D2S2_CLIENTSESSIONPROPERTIES_INITIALIZER, /* props */\
    NULL,  /* user_data */\
    RTI_FALSE, /* deleted */\
    RTI_FALSE, /* timedout */\
    RTI_FALSE, /* resetting */\
    REDA_INLINE_LIST_EMPTY, /* rcvd_data_pool */\
    { NULL }, /* timeout_listener */\
    { { NULL } }, /* timeout_listener_storage */\
    NULL, /* sem_timeout */\
    NULL, /* sem_cleanup */\
}

#define NDDSA_ClientSession_active(s_) \
    (!((s_)->deleted || (s_)->timedout || (s_)->resetting))

typedef struct NDDSA_ClientSessionRecordI
{
    struct REDAInlineListNode node;
    struct REDAWeakReference ref;
    NDDSA_ClientSession session;
    struct REDAExclusiveArea *ea;
} NDDSA_ClientSessionRecord;

#define NDDSA_CLIENTSESSIONRECORD_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    REDA_WEAK_REFERENCE_INVALID, /* ref */ \
    NDDSA_CLIENTSESSION_INITIALIZER, /* session */\
    NULL /* ea */\
}

NDDSA_ClientSessionRecord*
NDDSA_ClientSessionRecord_from_session(NDDSA_ClientSession *const self);

#define NDDSA_ClientSessionRecord_from_session(s_) \
((NDDSA_ClientSessionRecord*) \
    (((unsigned char*)(s_)) - \
        NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_ClientSessionRecord, session)))



void
NDDSA_ClientSession_find_attached_resource(
    NDDSA_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    NDDSA_AttachedResource **const attached_res_out);

typedef struct NDDSA_NDDSA_AttachedResourceRecordI
{
    struct REDAInlineListNode node;
    NDDSA_AttachedResource resource;
} NDDSA_AttachedResourceRecord;

#define NDDSA_ATTACHEDRESOURCERECORD_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NDDSA_ATTACHEDRESOURCE_INITIALIZER /* resource */\
}

typedef struct NDDSA_ReceivedDataI
{
    D2S2_ReceivedData base;
    struct REDAInlineListNode node;
    D2S2_DataRepresentation data;
    NDDSA_Read *read;
    NDDSA_ClientSession *session;
    struct DDS_SequenceNumber_t dds_sn;
} NDDSA_ReceivedData;

NDDSA_ReceivedData*
NDDSA_ReceivedData_from_node(struct REDAInlineListNode *const self);

#define NDDSA_ReceivedData_from_node(s_) \
((NDDSA_ReceivedData*) \
    (((unsigned char*)(s_)) - \
        NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_ReceivedData, node)))

typedef enum NDDSA_ReadStatusI
{
    NDDSA_READSTATUS_DATA_AVAILABLE     = 0x01 << 0,
    NDDSA_READSTATUS_COMPLETE           = 0x01 << 1
} NDDSA_ReadStatus;

#define NDDSA_READSTATUS_DEFAULT        ((NDDSA_ReadStatus)0)

struct NDDSA_ReadI
{
    struct REDAInlineListNode node;
    NDDSA_Agent *agent;
    D2S2_ClientSessionKey session_key;
    D2S2_AttachedResourceId reader_id;
    DDS_UnsignedLong samples_rcvd;
    DDS_UnsignedLong samples_max;
    DDS_UnsignedLong max_bytes;
    struct REDAInlineList samples;
    struct DDS_SequenceNumber_t last_sample;
    struct RTINtpTime start_ts;
    struct RTINtpTime last_sample_ts;
    struct RTINtpTime sample_pace;
    struct RTINtpTime max_elapsed_ts;
    char *content_filter_expr;
    RTIBool delayed;
    NDDSA_ReadStatus status;
    struct RTIEventGeneratorListener event_listener;
    struct RTIEventGeneratorListenerStorage event_listener_storage;
    struct RTIOsapiSemaphore *sem_event;
};

#define NDDSA_READ_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* agent */\
    D2S2_CLIENTSESSIONKEY_INITIALIZER, /* session_key */\
    D2S2_ATTACHEDRESOURCEID_INVALID, /* reader_id */\
    0, /* samples_rcvd */\
    0, /* samples_max */\
    0, /* max_bytes */\
    REDA_INLINE_LIST_EMPTY, /* samples */\
    DDS_SEQUENCENUMBER_DEFAULT, /* last_sample */\
    RTI_NTP_TIME_ZERO, /* start_ts */\
    RTI_NTP_TIME_ZERO, /* last_sample_ts */\
    RTI_NTP_TIME_ZERO, /* sample_pace */\
    RTI_NTP_TIME_ZERO, /* max_elapsed_ts */\
    NULL, /* content_filter_expr */\
    RTI_FALSE, /* delayed */\
    NDDSA_READSTATUS_DEFAULT, /* status */\
    { NULL }, /* event_listener */\
    { { NULL } }, /* event_listener_storage */\
    NULL /* sem_event */\
}

void
NDDSA_Read_set_status(NDDSA_Read *const self, const NDDSA_ReadStatus flags);

#define NDDSA_Read_set_status(s_,f_) \
{\
    (s_)->status |= (f_);\
}

void
NDDSA_Read_clear_status(NDDSA_Read *const self, const NDDSA_ReadStatus flags);

#define NDDSA_Read_clear_status(s_,f_) \
{\
    (s_)->status &= ~(f_);\
}

RTIBool
NDDSA_Read_check_status(NDDSA_Read *const self, const NDDSA_ReadStatus flags);

#define NDDSA_Read_check_status(s_,f_) \
    (((s_)->status & ((const NDDSA_ReadStatus)(f_)))?RTI_TRUE:RTI_FALSE)

NDDSA_Read*
NDDSA_Read_from_node(struct REDAInlineListNode *const self);

#define NDDSA_Read_from_node(s_) \
((NDDSA_Read*) \
    (((unsigned char*)(s_)) - \
        NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_Read, node)))

void
NDDSA_Read_is_complete(
    NDDSA_Read *const self,
    struct RTINtpTime *const ts_now,
    RTIBool *const finite_request,
    RTIBool *const complete);

RTIBool
NDDSA_Read_has_pending_samples(NDDSA_Read *const self);

#define NDDSA_Read_has_pending_samples(s_) \
    ((REDAInlineList_getFirst(&(s_)->samples) != NULL)?RTI_TRUE:RTI_FALSE)

RTIBool
NDDSA_Read_process_sampleEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    struct DDS_SampleInfo *const sample_info,
    DDS_DynamicData *const sample,
    struct RTINtpTime *const ts_now,
    RTIBool *const old_sample,
    RTIBool *const try_again,
    struct RTINtpTime *const min_delay);

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsClientSession_h */