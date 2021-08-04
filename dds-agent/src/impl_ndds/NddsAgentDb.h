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

#ifndef NddsAgentDb_h
#define NddsAgentDb_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsResource.h"
#include "NddsClientSession.h"


typedef struct NDDSA_AgentDbI
{
    NDDSA_Agent *agent;
    struct REDAWorkerFactory *worker_factory;
    struct REDAExclusiveArea *ea_db;
    struct REDADatabase *db;
    struct REDAWeakReference table_resources;
    struct REDAWeakReference table_sessions;
    struct REDACursorPerWorker *cursor_p_worker_resources;
    struct REDACursorPerWorker *cursor_p_worker_sessions;
    struct REDAExclusiveArea *ea_table_resources;
    struct REDAExclusiveArea *ea_table_sessions;
    DDS_DomainParticipantFactory *factory;
} NDDSA_AgentDb;


#define NDDSA_AGENTDB_INITIALIZER \
{\
    NULL, /* agent */\
    NULL, /* worker_factory */\
    NULL, /* ea_db */\
    NULL, /* db */\
    REDA_WEAK_REFERENCE_INVALID, /* table_resources */\
    REDA_WEAK_REFERENCE_INVALID, /* table_sessions */\
    NULL, /* cursor_p_worker_resources */\
    NULL, /* cursor_p_worker_sessions */\
    NULL, /* ea_table_resources */\
    NULL, /* ea_table_sessions */\
    NULL  /* factory */\
}
    
RTIBool
NDDSA_AgentDb_initialize(
    NDDSA_AgentDb *const self,
    NDDSA_Agent *const agent);

RTIBool
NDDSA_AgentDb_finalize(
    NDDSA_AgentDb *const self);

RTIBool
NDDSA_AgentDb_lookup_resource(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const id,
    NDDSA_ResourceRecord **const record_out);

RTIBool
NDDSA_AgentDb_release_resource(
    NDDSA_AgentDb *const self,
    NDDSA_ResourceRecord *const record);

RTIBool
NDDSA_AgentDb_lock_resources(
    NDDSA_AgentDb *const self,
    const RTIBool start_cursor,
    const RTIBool goto_top);

RTIBool
NDDSA_AgentDb_unlock_resources(
    NDDSA_AgentDb *const self,
    const RTIBool finish_cursor);

RTIBool
NDDSA_AgentDb_iterate_resourcesEA(NDDSA_AgentDb *const self);

RTIBool
NDDSA_AgentDb_finish_iterate_resourcesEA(NDDSA_AgentDb *const self);

RTIBool
NDDSA_AgentDb_lookup_resourceEA(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const resource_id,
    NDDSA_ResourceRecord **const record_out);

RTIBool
NDDSA_AgentDb_release_resourceEA(
    NDDSA_AgentDb *const self,
    NDDSA_ResourceRecord *const record);


typedef RTIBool
    (*NDDSA_AgentDb_FilterResourceRecordFn)(
        NDDSA_AgentDb *const self,
        NDDSA_ResourceRecord *const record,
        void *const param,
        RTIBool *const return_record_out);

RTIBool
NDDSA_AgentDb_find_next_resourceEA(
    NDDSA_AgentDb *const self,
    NDDSA_AgentDb_FilterResourceRecordFn filter,
    void *const param,
    NDDSA_ResourceRecord **const record_out);

RTIBool
NDDSA_AgentDb_insert_resourceEA(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const id,
    NDDSA_ResourceRecord **const record_out,
    RTIBool *const already_exists_out);

RTIBool
NDDSA_AgentDb_delete_resourceEA(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const record_id);


RTIBool
NDDSA_AgentDb_lock_sessions(
    NDDSA_AgentDb *const self,
    const RTIBool start_cursor,
    const RTIBool goto_top);

RTIBool
NDDSA_AgentDb_unlock_sessions(
    NDDSA_AgentDb *const self,
    const RTIBool finish_cursor);

RTIBool
NDDSA_AgentDb_lookup_sessionEA(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key,
    NDDSA_ClientSessionRecord **const record_out);


RTIBool
NDDSA_AgentDb_release_sessionEA(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key);

RTIBool
NDDSA_AgentDb_delete_sessionEA(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key);

RTIBool
NDDSA_AgentDb_assert_session(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key,
    NDDSA_ClientSessionRecord **const record_out,
    RTIBool *const already_exists);

RTIBool
NDDSA_AgentDb_insert_session(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key,
    RTIBool *const new_record_out,
    NDDSA_ClientSessionRecord **const record_out);

RTIBool
NDDSA_AgentDb_lookup_session(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key,
    NDDSA_ClientSessionRecord **const record_out);

RTIBool
NDDSA_AgentDb_release_session(
    NDDSA_AgentDb *const self,
    NDDSA_ClientSessionRecord *const record);

RTIBool
NDDSA_AgentDb_delete_session(
    NDDSA_AgentDb *const self,
    NDDSA_ClientSessionRecord *const record);

typedef RTIBool
    (*NDDSA_AgentDb_OnIterateSessionFn)(
        void *listener,
        NDDSA_AgentDb *const self,
        NDDSA_ClientSessionRecord *const record);


typedef RTIBool
    (*NDDSA_AgentDb_FilterRecordFn)(
        NDDSA_AgentDb *const db,
        void *const record,
        void *const filter_param);

RTIBool
NDDSA_AgentDb_iterate_sessions(NDDSA_AgentDb *const self);

RTIBool
NDDSA_AgentDb_finish_iterate_sessions(NDDSA_AgentDb *const self);


RTIBool
NDDSA_AgentDb_find_next_session(
    NDDSA_AgentDb *const self,
    NDDSA_AgentDb_FilterRecordFn filter_record_fn,
    void *const filter_param,
    NDDSA_ClientSessionRecord *const prev_session,
    NDDSA_ClientSessionRecord **const record_out);

RTIBool
NDDSA_AgentDb_find_next_session_and_delete_previous(
    NDDSA_AgentDb *const self,
    void *const filter_param,
    NDDSA_ClientSessionRecord *const prev_session,
    NDDSA_ClientSessionRecord **const record_out);

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsAgentDb_h */