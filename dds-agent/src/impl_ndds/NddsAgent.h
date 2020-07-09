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

#ifndef NddsAgent_h
#define NddsAgent_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsInfrastructure.h"
#include "NddsResource.h"
#include "NddsAgentDb.h"
#include "NddsResourceFactory.h"
#include "NddsReader.h"
#include "NddsWriter.h"


#define RTI_EVENTACTIVEOBJECTLISTENER_INITIALIZER \
{\
    NULL, /* onStopped */\
    NULL  /* onStoppedParam */\
}

#define RTI_EVENTACTIVEGENERATORLISTENER_INITIALIZER \
{\
    RTI_EVENTACTIVEOBJECTLISTENER_INITIALIZER, /* parent */\
    NULL, /* onStarted */\
    NULL  /* onStartedParam */\
}

struct NDDSA_AgentI
{
    D2S2_Agent base;
    NDDSA_AgentDb db;
    NDDSA_ResourceFactory res_factory;
    struct REDAFastBufferPool *pool_readers;
    struct REDAFastBufferPool *pool_writers;
    struct REDAFastBufferPool *pool_resources;
    struct REDAFastBufferPool *pool_reads;
    struct REDAFastBufferPool *pool_samples;
    struct REDAFastBufferPool *pool_events_session;
    struct RTIClock *clock;
    struct REDAInlineList interfaces;
    struct RTIEventTimer *timer;
    struct RTIEventActiveGenerator *generator;
    struct RTIEventActiveGeneratorListener generator_listener;
    struct RTINtpTime event_snooze_ts;
    struct RTIOsapiSemaphore *sem_start;
    struct RTIOsapiSemaphore *sem_stop;
    DDS_GuardCondition *cond_cleanup_sessions;
    DDS_GuardCondition *cond_cleanup_resources;
    DDS_GuardCondition *cond_cleanup_reads;
    DDS_AsyncWaitSet *waitset;
    RTIBool started;
    RTIBool disposed;
    NDDSA_AgentProperties properties;
    struct RTIEventGeneratorListener session_event_listener;
};


#define NDDSA_AGENT_INITIALIZER \
{\
    D2S2_AGENT_INITIALIZER, /* base */\
    NDDSA_AGENTDB_INITIALIZER, /* db */\
    NDDSA_RESOURCEFACTORY_INITIALIZER, /* res_factory */ \
    NULL, /* pool_readers */\
    NULL, /* pool_writers */\
    NULL, /* pool_resources */\
    NULL, /* pool_reads */\
    NULL, /* pool_samples */\
    NULL, /* pool_events_session */\
    NULL, /* clock */\
    REDA_INLINE_LIST_EMPTY, /* interfaces */\
    NULL, /* timer */\
    NULL,  /* generator */\
    RTI_EVENTACTIVEGENERATORLISTENER_INITIALIZER, /* generator_listener */\
    RTI_NTP_TIME_ZERO, /* event_snooze_ts */\
    NULL, /* sem_start */\
    NULL, /* sem_stop */\
    NULL, /* cond_cleanup_sessions */\
    NULL, /* cond_cleanup_resources */\
    NULL, /* cond_cleanup_reads */\
    NULL, /* waitset */\
    RTI_FALSE, /* disposed */\
    RTI_FALSE, /* started */\
    NDDSA_AGENTPROPERTIES_INITIALIZER, /* properties */\
    { NULL } /* session_event_listener */\
}

typedef struct NDDSA_ClientSessionEventI
{
    D2S2_ClientSessionEvent base;
    D2S2_Agent_OnSessionEventCallback on_event;
    struct RTINtpTime delay_ts;
    struct RTIEventGeneratorListenerStorage listener_storage;
    struct RTIOsapiSemaphore *sem_cancelled;
} NDDSA_ClientSessionEvent;

RTIBool
NDDSA_Agent_on_session_event(
    const struct RTIEventGeneratorListener *me,
    struct RTINtpTime *newTime, struct RTINtpTime *newSnooze,
    const struct RTINtpTime *now, const struct RTINtpTime *time,
    const struct RTINtpTime *snooze,
    const struct RTIEventGeneratorListenerStorage *listenerStorage,
    struct REDAWorker *worker);

RTIBool
NDDSA_Agent_post_event(
    NDDSA_Agent *const self,
    struct RTIEventGeneratorListener *const listener,
    struct RTIEventGeneratorListenerStorage *const listener_storage,
    const DDS_UnsignedLong listener_storage_size,
    const struct RTINtpTime *const delay);

RTIBool
NDDSA_Agent_initialize(
    NDDSA_Agent *const self,
    const NDDSA_AgentProperties *const user_properties);

RTIBool
NDDSA_Agent_finalize(NDDSA_Agent *const self);

RTIBool
NDDSA_Agent_on_session_timeout(
    const struct RTIEventGeneratorListener *me,
    struct RTINtpTime *newTime, struct RTINtpTime *newSnooze,
    const struct RTINtpTime *now, const struct RTINtpTime *time,
    const struct RTINtpTime *snooze,
    const struct RTIEventGeneratorListenerStorage *listenerStorage,
    struct REDAWorker *worker);

DDS_ReturnCode_t
NDDSA_Agent_cancel_readI(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    void *const request_param);

DDS_ReturnCode_t
NDDSA_Agent_on_session_activityI(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec);

DDS_ReturnCode_t
NDDSA_Agent_dispose_sessionEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec);

RTIBool
NDDSA_Agent_wait_for_semaphoreEA(
    NDDSA_Agent *const self,
    const RTIBool locked_sessions,
    NDDSA_ClientSessionRecord *const session_rec,
    const RTIBool locked_resources,
    NDDSA_ResourceRecord *const resource_rec,
    struct RTIOsapiSemaphore *sem_event);

DDS_ReturnCode_t
NDDSA_Agent_detach_resourceEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    const RTIBool explicit_request,
    void *const request_param,
    D2S2_ResourceKind *const resource_kind_out,
    D2S2_ResourceId *const ref_resource_id_out,
    void **const resource_data_out);

DDS_ReturnCode_t
NDDSA_Agent_create_resourceEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    const D2S2_AttachedResourceId resource_id,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceRepresentation *const resource_repr,
    const D2S2_AttachedResourceId parent_id,
    const D2S2_ResourceProperties *const properties,
    void *const request_param,
    NDDSA_AttachedResource **const attached_out);

DDS_ReturnCode_t
NDDSA_Agent_readEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    const D2S2_ReadSpecification *const read_spec,
    void *const request_param,
    void **const resource_data_out);

DDS_ReturnCode_t
NDDSA_Agent_cancel_readEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    const RTIBool locked_resources,
    NDDSA_ResourceRecord *const resource_rec,
    void *const request_param);

DDS_ReturnCode_t
NDDSA_Agent_writeEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *attached,
    const D2S2_DataRepresentation *const data,
    void *const request_param);

DDS_ReturnCode_t
NDDSA_Agent_create_implicit_resources(
    NDDSA_Agent *const self,
    const D2S2_ResourceProperties *const properties);

DDS_ReturnCode_t
NDDSA_Agent_insert_resource_recordEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceRepresentation *const resource_repr,
    const D2S2_ResourceId *const parent_res_id,
    const D2S2_ResourceProperties *const properties,
    NDDSA_ResourceRecord **const resource_rec_out);

DDS_ReturnCode_t
NDDSA_Agent_attach_all_resources_to_sessionEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec);

DDS_ReturnCode_t
NDDSA_Agent_attach_resource_to_sessionEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    const D2S2_AttachedResourceId resource_id,
    NDDSA_ResourceRecord *const resource_rec,
    NDDSA_AttachedResource **const attached_out);

DDS_ReturnCode_t
NDDSA_Agent_attach_resource_to_all_sessionsEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceId *const resource_id);


DDS_ReturnCode_t
NDDSA_Agent_generate_attached_resource_id(
    D2S2_Agent *const self,
    const D2S2_ResourceId *resource_id,
    const D2S2_ResourceKind resource_kind,
    D2S2_AttachedResourceId *const id_out);

/******************************************************************************
 * Helper Macros to lookup and lock a Session
 ******************************************************************************/

#define AGENT_LOOKUP_SESSION_OR_DONE(s_,k_,sro_,on_unknown_) \
{\
    if (!NDDSA_AgentDb_lookup_session(\
            &(s_)->db, (k_), (sro_)))\
    {\
        D2S2Log_exception(\
            method_name,\
            &RTI_LOG_ANY_FAILURE_s,\
            D2S2_LOG_MSG_DB_LOOKUP_SESSION_FAILED);\
        goto done;\
    }\
    if (*(sro_) == NULL)\
    {\
        D2S2Log_local(\
            method_name,\
            &RTI_LOG_ANY_s,\
            "UNKNOWN session");\
        on_unknown_ \
        goto done;\
    }\
    if (!NDDSA_ClientSession_active(&(*(sro_))->session))\
    {\
        D2S2Log_warn(\
            method_name,\
            &RTI_LOG_ANY_s,\
            "INACTIVE session");\
        goto done;\
    }\
}

#define AGENT_LOOKUP_SESSION_EA_OR_DONE(s_,k_,sro_) \
{\
    if (!NDDSA_AgentDb_lookup_sessionEA(\
            &(s_)->db, (k_), (sro_)))\
    {\
        D2S2Log_exception(\
            method_name,\
            &RTI_LOG_ANY_FAILURE_s,\
            D2S2_LOG_MSG_DB_LOOKUP_SESSION_FAILED);\
        goto done;\
    }\
    if (*(sro_) == NULL)\
    {\
        D2S2Log_local(\
            method_name,\
            &RTI_LOG_ANY_s,\
            "UNKNOWN session");\
        goto done;\
    }\
    if (!NDDSA_ClientSession_active(&(*(sro_))->session))\
    {\
        D2S2Log_warn(\
            method_name,\
            &RTI_LOG_ANY_s,\
            "INACTIVE session");\
        goto done;\
    }\
}


#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsAgent_h */