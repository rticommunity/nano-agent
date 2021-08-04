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

#ifndef NddsExternalService_h
#define NddsExternalService_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsInfrastructure.h"

typedef struct NDDSA_ExternalServiceI
{
    D2S2_ResourceId id;
    D2S2_ExternalServicePlugin *plugin;
    char * plugin_name;
    char * url;
    char * descriptor;
    void * data;
    void * user_data;
    struct REDAInlineList resources;
} NDDSA_ExternalService;

#define NDDSA_EXTERNALSERVICE_INITIALIZER \
{\
    D2S2_RESOURCEID_INITIALIZER, /* id */\
    NULL, /* plugin */\
    NULL, /* plugin_name */\
    NULL, /* url */\
    NULL, /* descriptor */\
    NULL, /* data */\
    NULL, /* user_data */\
    REDA_INLINE_LIST_EMPTY /* resources */\
}

typedef struct NDDSA_ExternalServiceResourceI
{
    struct REDAInlineListNode node;
    D2S2_ResourceId id;
    char * path;
    char * descriptor;
    void * data;
    void * user_data;
    NDDSA_ExternalService * service;
    struct REDAInlineList requests;
} NDDSA_ExternalServiceResource;

#define NDDSA_EXTERNALSERVICERESOURCE_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    D2S2_RESOURCEID_INITIALIZER, /* id */\
    NULL, /* path */\
    NULL, /* descriptor */\
    NULL, /* data */\
    NULL, /* user_data */\
    NULL, /* service */\
    REDA_INLINE_LIST_EMPTY, /* resources */\
}

typedef struct NDDSA_ExternalServiceReplyI
{
    D2S2_ReceivedData base;
    struct REDAInlineListNode node;
    NDDSA_ClientSession *session;
    D2S2_DataRepresentation data_repr;
    D2S2_ExternalServiceReply *reply;
} NDDSA_ExternalServiceReply;

#define NDDSA_EXTERNALSERVICEREPLY_INITIALIZER \
{\
    D2S2_RECEIVEDDATA_INITIALIZER, /* base */\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* session */\
    D2S2_DATAREPRESENTATION_INITIALIZER, /* reply */\
    NULL /* reply */\
}

NDDSA_ExternalServiceReply*
NDDSA_ExternalServiceReply_from_node(struct REDAInlineListNode *const self);
#define NDDSA_ExternalServiceReply_from_node(s_) \
((NDDSA_ExternalServiceReply*) \
    (((unsigned char*)(s_)) - \
        NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_ExternalServiceReply, node)))

typedef struct NDDSA_ExternalServiceRequestI
{
    D2S2_ExternalServiceRequestToken token;
    D2S2_ClientSession *session;
    D2S2_AttachedResourceId attached;
    struct REDAInlineList replies;
    struct RTIEventGeneratorListener listener;
    struct RTIEventGeneratorListenerStorage listener_storage;
    struct RTIOsapiSemaphore *sem_cancelled;
} NDDSA_ExternalServiceRequest;

#define NDDSA_EXTERNALSERVICEREQUEST_INITIALIZER \
{\
    D2S2_EXTERNALSERVICEREQUESTTOKEN_UNKNOWN, /* token */\
    NULL, /* session */\
    D2S2_ATTACHEDRESOURCEID_INVALID, /* attached */\
    REDA_INLINE_LIST_EMPTY, /* replies */\
    { NULL }, /* listener */\
    { { NULL } }, /* listener_storage */\
    NULL /* sem_cancelled */\
}

typedef struct NDDSA_ExternalServicePendingRequestI
{
    struct REDAInlineListNode node;
    NDDSA_ExternalServiceRequest *request;
} NDDSA_ExternalServicePendingRequest;

#define NDDSA_EXTERNALSERVICEPENDINGREQUEST_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL /* request */\
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsExternalService_h */