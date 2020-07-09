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

#include "NddsAgent.h"
#include "NddsAgentIntf.h"
#include "clock/clock_monotonic.h"
#include "clock/clock_system.h"


#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

/******************************************************************************
 * Private implementation types
 ******************************************************************************/

struct DeleteNativeResourceParam
{
    D2S2_ResourceKind kind;
    NDDSA_EntityResource native_entity;
};

#define DeleteNativeResourceParam_INITIALIZER \
{\
    D2S2_RESOURCEKIND_UNKNOWN,\
    NDDSA_ENTITYRESOURCE_INITIALIZER \
}


/******************************************************************************
 * Private implementation functions
 ******************************************************************************/

RTI_PRIVATE
RTIBool
NDDSA_Agent_finalize_database(NDDSA_Agent *const self);

RTI_PRIVATE
RTIBool
NDDSA_Agent_finalize_database_sessions(NDDSA_Agent *const self);

RTI_PRIVATE
RTIBool
NDDSA_Agent_finalize_database_resources(NDDSA_Agent *const self);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_finalize_sessionEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec);

RTI_PRIVATE
void 
NDDSA_Agent_on_start_generator(
    struct RTIEventActiveGenerator *generator,
    void * param,
    struct REDAWorker *worker);

RTI_PRIVATE
void 
NDDSA_Agent_on_stop_generator(
    struct RTIEventActiveObject *object,
    void *param, 
    struct REDAWorker *worker);

RTI_PRIVATE
RTIBool
NDDSA_Agent_find_next_disposed_session(
    NDDSA_AgentDb *const db,
    void *const record,
    void *const filter_param);

RTI_PRIVATE
RTIBool
NDDSA_Agent_find_resource_by_native_resource(
    NDDSA_AgentDb *const self,
    NDDSA_ResourceRecord *const resource_rec,
    void *const p,
    RTIBool *const return_record_out);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_find_contained_resource_and_deleteEA(
    NDDSA_Agent *const self,
    const struct DeleteNativeResourceParam *const param);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_native_resource_entityEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceId *const resource_id,
    const D2S2_ResourceKind resource_kind,
    const NDDSA_EntityResource *const native_entity);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_native_resourceEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceId *const resource_id);

RTI_PRIVATE
RTIBool
NDDSA_Agent_read_from_reader_ddsEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *session_rec,
    NDDSA_AttachedResource *const attached,
    struct DDS_DynamicDataSeq *const samples_seq,
    struct DDS_SampleInfoSeq *const infos_seq,
    RTIBool *const has_loan_out,
    DDS_UnsignedLong *const samples_len_out);

RTI_PRIVATE
RTIBool
NDDSA_Agent_return_loan_to_reader_ddsEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *session_rec,
    NDDSA_AttachedResource *const attached,
    struct DDS_DynamicDataSeq *const samples_seq,
    struct DDS_SampleInfoSeq *const infos_seq,
    const RTIBool updated);

RTI_PRIVATE
RTIBool
NDDSA_Agent_read_from_readerEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *session_rec,
    NDDSA_AttachedResource *const attached);

RTI_PRIVATE
RTIBool
NDDSA_Agent_on_data_on_reader(
    const struct RTIEventGeneratorListener *me,
    struct RTINtpTime *newTime, struct RTINtpTime *newSnooze,
    const struct RTINtpTime *now, const struct RTINtpTime *time,
    const struct RTINtpTime *snooze,
    const struct RTIEventGeneratorListenerStorage *listenerStorage,
    struct REDAWorker *worker);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_buffer_to_dyndata(
    NDDSA_Writer *writer_data,
    const D2S2_DataRepresentation *const data,
    DDS_DynamicData *const sample_out);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_enable_resourceEA(
    NDDSA_Agent *const self,
    NDDSA_ResourceRecord *const resource_rec);

RTI_PRIVATE
void
NDDSA_Agent_on_cleanup_sessions(
    void *handler_data,
    DDS_Condition *condition);

RTI_PRIVATE
void
NDDSA_Agent_on_cleanup_resources(
    void *handler_data,
    DDS_Condition *condition);

RTI_PRIVATE
void
NDDSA_Agent_on_cleanup_reads(
    void *handler_data,
    DDS_Condition *condition);

/******************************************************************************
 *                      Public function implementations
 ******************************************************************************/

RTIBool
NDDSA_Agent_post_event(
    NDDSA_Agent *const self,
    struct RTIEventGeneratorListener *const listener,
    struct RTIEventGeneratorListenerStorage *const listener_storage,
    const DDS_UnsignedLong listener_storage_size,
    const struct RTINtpTime *const delay)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_post_event)
    RTIBool retcode = RTI_FALSE;
    struct RTINtpTime ts_now = RTI_NTP_TIME_ZERO,
                    ts_event = RTI_NTP_TIME_ZERO,
                    ts_delay = RTI_NTP_TIME_ZERO;
    
    D2S2Log_fn_entry()

    /* post event so that the guard condition will be triggered
        after delay */
    if (!self->clock->getTime(self->clock, &ts_now))
    {
        goto done;
    }

    ts_delay = *delay;
    RTINtpTime_add(ts_event, ts_now, ts_delay);
    
    if (!((struct RTIEventGenerator*)self->generator)->postEvent(
            ((struct RTIEventGenerator*)self->generator),
            &ts_event,
            &self->event_snooze_ts,
            listener,
            listener_storage,
            listener_storage_size))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_Agent_initialize(
    NDDSA_Agent *const self,
    const NDDSA_AgentProperties *const user_properties)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_initialize)
    RTIBool res = RTI_FALSE;
    const NDDSA_Agent def_self = NDDSA_AGENT_INITIALIZER;
    struct REDAFastBufferPoolProperty pool_props =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    struct DDS_ConditionHandler cond_handler = DDS_ConditionHandler_INITIALIZER;

    D2S2Log_fn_entry()
    
    *self = def_self;
    self->base.intf = &NDDSA_Agent_fv_Intf;

    if (user_properties != NULL)
    {
        self->properties = *user_properties;
    }

    if (!NDDSA_ResourceFactory_initialize(&self->res_factory))
    {
        goto done;
    }

    if (!NDDSA_AgentDb_initialize(&self->db, self))
    {
        goto done;
    }
    
    self->pool_readers = 
        REDAFastBufferPool_newForStructure(NDDSA_Reader, &pool_props);
    if (self->pool_readers == NULL)
    {
        goto done;
    }

    self->pool_writers = 
        REDAFastBufferPool_newForStructure(NDDSA_Writer, &pool_props);
    if (self->pool_writers == NULL)
    {
        goto done;
    }

    self->pool_resources = 
        REDAFastBufferPool_newForStructure(NDDSA_AttachedResource, &pool_props);
    if (self->pool_resources == NULL)
    {
        goto done;
    }

    self->pool_reads = 
        REDAFastBufferPool_newForStructure(NDDSA_Read, &pool_props);
    if (self->pool_reads == NULL)
    {
        goto done;
    }

    self->pool_samples = 
        REDAFastBufferPool_newForStructure(NDDSA_ReceivedData, &pool_props);
    if (self->pool_samples == NULL)
    {
        goto done;
    }

    self->pool_events_session = 
        REDAFastBufferPool_newForStructure(
            NDDSA_ClientSessionEvent, &pool_props);
    if (self->pool_events_session == NULL)
    {
        goto done;
    }

    self->session_event_listener.onEvent = NDDSA_Agent_on_session_event;

    self->clock = RTISystemClock_new();
    if(self->clock == NULL)
    {
        goto done;
    }
  
    self->timer = RTIEventSmartTimer_new();
    if (self->timer == NULL)
    {
        goto done;
    }

    self->sem_stop = 
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_COUNTING, NULL);
    if (self->sem_stop == NULL)
    {
        goto done;
    }

    self->sem_start = 
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_COUNTING, NULL);
    if (self->sem_start == NULL)
    {
        goto done;
    }

    self->cond_cleanup_sessions = DDS_GuardCondition_new();
    if (self->cond_cleanup_sessions == NULL)
    {
        goto done;
    }

    cond_handler.on_condition_triggered = NDDSA_Agent_on_cleanup_sessions;
    cond_handler.handler_data = self;
    if (DDS_RETCODE_OK !=
            DDS_Condition_set_handler(
                DDS_GuardCondition_as_condition(self->cond_cleanup_sessions),
                &cond_handler))
    {
        goto done;
    }

    self->cond_cleanup_reads = DDS_GuardCondition_new();
    if (self->cond_cleanup_reads == NULL)
    {
        goto done;
    }

    cond_handler.on_condition_triggered = NDDSA_Agent_on_cleanup_reads;
    cond_handler.handler_data = self;
    if (DDS_RETCODE_OK !=
            DDS_Condition_set_handler(
                DDS_GuardCondition_as_condition(self->cond_cleanup_reads),
                &cond_handler))
    {
        goto done;
    }

    self->cond_cleanup_resources = DDS_GuardCondition_new();
    if (self->cond_cleanup_resources == NULL)
    {
        goto done;
    }

    cond_handler.on_condition_triggered = NDDSA_Agent_on_cleanup_resources;
    cond_handler.handler_data = self;
    if (DDS_RETCODE_OK !=
            DDS_Condition_set_handler(
                DDS_GuardCondition_as_condition(self->cond_cleanup_resources),
                &cond_handler))
    {
        goto done;
    }

    self->waitset =
        DDS_AsyncWaitSet_new(&DDS_ASYNC_WAITSET_PROPERTY_DEFAULT);
    if (self->waitset == NULL)
    {
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_AsyncWaitSet_attach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_cleanup_sessions)))
    {
        goto done;
    }
    if (DDS_RETCODE_OK !=
            DDS_AsyncWaitSet_attach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_cleanup_resources)))
    {
        goto done;
    }
    if (DDS_RETCODE_OK !=
            DDS_AsyncWaitSet_attach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_cleanup_reads)))
    {
        goto done;
    }

    self->generator_listener.onStarted =
        NDDSA_Agent_on_start_generator;
    self->generator_listener.onStartedParam = self;
    self->generator_listener.parent.onStopped =
        NDDSA_Agent_on_stop_generator;
    self->generator_listener.parent.onStoppedParam = self;

    self->generator =
        RTIEventActiveGenerator_new(
            NULL,
            self->db.worker_factory,
            self->clock,
            self->timer,
            &self->generator_listener,
            NULL /* property */,
            NULL /* thread_factory */,
            NULL /* worker */);
    if (self->generator == NULL)
    {
        goto done;
    }
    /* wait for event generator thread to start */
    RTIOsapiSemaphore_take(self->sem_start, RTI_NTP_TIME_INFINITE);


    if (DDS_RETCODE_OK != DDS_AsyncWaitSet_start(self->waitset))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "DDS_AsyncWaitSet_start");
        goto done;
    }

    res = RTI_TRUE;
done:
    if (!res)
    {
        /* TODO finalize allocated resources */
    }
    D2S2Log_fn_exit()
    return res;
}

RTIBool
NDDSA_Agent_finalize(NDDSA_Agent *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_finalize)
    RTIBool retcode = RTI_FALSE;
    D2S2_AgentServerInterface *intf = NULL;

    D2S2Log_fn_entry()

    if (self->started)
    {
        if (DDS_RETCODE_OK != D2S2_Agent_stop(&self->base))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "D2S2_Agent_stop");
            goto done;
        }
    }

    intf = (D2S2_AgentServerInterface*)
                REDAInlineList_getFirst(&self->interfaces);
    while (intf  != NULL)
    {
        D2S2_AgentServerInterface_on_before_interface_disposed(
            intf, &self->base);
        intf = (D2S2_AgentServerInterface*)
                    REDAInlineListNode_getNext(&intf->node);
    }

    self->disposed = RTI_TRUE;

    if (!NDDSA_Agent_finalize_database(self))
    {
        goto done;
    }

    if (DDS_RETCODE_OK != DDS_AsyncWaitSet_stop(self->waitset))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "DDS_AsyncWaitSet_stop");
        goto done;
    }
    
    RTIEventActiveGenerator_shutdown(self->generator, NULL);
    RTIOsapiSemaphore_take(self->sem_stop, RTI_NTP_TIME_INFINITE);

    if (DDS_RETCODE_OK !=
            DDS_AsyncWaitSet_detach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_cleanup_sessions)))
    {
        goto done;
    }
    if (DDS_RETCODE_OK !=
            DDS_AsyncWaitSet_detach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_cleanup_sessions)))
    {
        goto done;
    }
    if (DDS_RETCODE_OK !=
            DDS_AsyncWaitSet_detach_condition(
                self->waitset,
                DDS_GuardCondition_as_condition(self->cond_cleanup_reads)))
    {
        goto done;
    }


    intf = (D2S2_AgentServerInterface*)
                REDAInlineList_getFirst(&self->interfaces);
    while (intf  != NULL)
    {
        D2S2_AgentServerInterface_on_interface_disposed(
            intf, &self->base);
        REDAInlineList_removeNodeEA(&self->interfaces, &intf->node);

        intf = (D2S2_AgentServerInterface*)
                    REDAInlineList_getFirst(&self->interfaces);
    }

    /* Finalize database */
    if (!NDDSA_AgentDb_finalize(&self->db))
    {
        goto done;
    }

    if (DDS_RETCODE_OK != DDS_AsyncWaitSet_delete(self->waitset))
    {
        goto done;
    }
    self->waitset = NULL;

    RTIEventActiveGenerator_delete(self->generator, NULL);
    self->generator = NULL;

    RTIEventSmartTimer_delete(self->timer);
    self->timer = NULL;

    RTISystemClock_delete(self->clock);
    
    RTIOsapiSemaphore_delete(self->sem_stop);
    self->sem_stop = NULL;

    RTIOsapiSemaphore_delete(self->sem_start);
    self->sem_start = NULL;

    DDS_GuardCondition_delete(self->cond_cleanup_sessions);
    self->cond_cleanup_sessions = NULL;

    DDS_GuardCondition_delete(self->cond_cleanup_resources);
    self->cond_cleanup_resources = NULL;

    DDS_GuardCondition_delete(self->cond_cleanup_reads);
    self->cond_cleanup_reads = NULL;
    
    REDAFastBufferPool_delete(self->pool_readers);
    self->pool_readers = NULL;
    REDAFastBufferPool_delete(self->pool_writers);
    self->pool_writers = NULL;
    REDAFastBufferPool_delete(self->pool_reads);
    self->pool_reads = NULL;
    REDAFastBufferPool_delete(self->pool_samples);
    self->pool_samples = NULL;
    REDAFastBufferPool_delete(self->pool_resources);
    self->pool_resources = NULL;
    REDAFastBufferPool_delete(self->pool_events_session);
    self->pool_events_session = NULL;

    NDDSA_ResourceFactory_finalize(&self->res_factory);

    DDS_DomainParticipantFactory_finalize_instance();

    retcode = RTI_TRUE;
    
done:
    
    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_Agent_on_session_timeout(
    const struct RTIEventGeneratorListener *me,
    struct RTINtpTime *newTime, struct RTINtpTime *newSnooze,
    const struct RTINtpTime *now, const struct RTINtpTime *time,
    const struct RTINtpTime *snooze,
    const struct RTIEventGeneratorListenerStorage *listenerStorage,
    struct REDAWorker *worker)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_session_timeout)
    NDDSA_ClientSession *session =
        (NDDSA_ClientSession*) listenerStorage->field[0];
    RTIBool disposed = (listenerStorage->field[1] == NULL);
    NDDSA_Agent *self = NULL;
    D2S2_AgentServerInterface *src = NULL;
    D2S2_ClientSessionKey session_key = D2S2_CLIENTSESSIONKEY_INITIALIZER;
    NDDSA_ClientSessionRecord *session_rec = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(now);
    UNUSED_ARG(time);
    UNUSED_ARG(snooze);
    UNUSED_ARG(worker);
    UNUSED_ARG(newSnooze);
    UNUSED_ARG(newTime);
    UNUSED_ARG(me);

    if (disposed)
    {
        // printf("*****************************************\n");
        // printf("     GIVE sem_timeout %p\n", session->sem_timeout);
        // printf("*****************************************\n");
        RTIOsapiSemaphore_give(session->sem_timeout);
        return RTI_FALSE;
    }

    self = session->agent;
    src = session->intf;
    session_key = session->base.key;

    AGENT_LOOKUP_SESSION_OR_DONE(
        self, &session_key, &session_rec, /* on_unknown */);

    session_rec->session.timedout = RTI_TRUE;

    D2S2_AgentServerInterface_on_session_timed_out(
        src,
        &self->base,
        &session_rec->session.base,
        session_rec->session.user_data);
    
    if (DDS_RETCODE_OK !=
            NDDSA_Agent_dispose_sessionEA(self, session_rec))
    {
        goto done;
    }

    if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
    {
        session_rec = NULL;
        goto done;
    }
    session_rec = NULL;
    
done:

    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
        {
            /* TODO log */
        }
    }

    D2S2Log_fn_exit()
    return RTI_FALSE;
}


DDS_ReturnCode_t
NDDSA_Agent_cancel_readI(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_cancel_readI)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_ResourceRecord *resource_rec = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(src);
    
    if (!NDDSA_AgentDb_lookup_resource(
            &self->db, &attached->base.resource, &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "UNKNOWN resource");
        goto done;
    }
    if (resource_rec->resource.native_deleted)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "native resource ALREADY DELETED");
        goto done;
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_cancel_readEA(
                self,
                session_rec,
                attached,
                RTI_FALSE /* locked resources */,
                resource_rec,
                request_param))
    {
        goto done;
    }

    retcode = DDS_RETCODE_OK;
    
done:
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_on_session_activityI(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTINtpTime timeout_ts = RTI_NTP_TIME_ZERO;

    /* If the session's timeout is not infinite nor zero, post an event that 
       will trigger after the specified time to dispose the session */
    if (!DDS_Duration_is_infinite(&session_rec->session.props.timeout) &&
        !DDS_Duration_is_zero(&session_rec->session.props.timeout))
    {
        DDS_Duration_to_ntp_time(
            &session_rec->session.props.timeout, &timeout_ts);

        if (!NDDSA_Agent_post_event(
                self,
                &session_rec->session.timeout_listener,
                &session_rec->session.timeout_listener_storage,
                sizeof(NDDSA_ClientSession*),
                &timeout_ts))
        {
            goto done;
        }
    }
    
    retcode = DDS_RETCODE_OK;
    
done:

    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_dispose_sessionEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    if (session_rec->session.deleted)
    {
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    session_rec->session.deleted = RTI_TRUE;

    if (DDS_RETCODE_OK !=
            DDS_GuardCondition_set_trigger_value(
                self->cond_cleanup_sessions, DDS_BOOLEAN_TRUE))
    {
        goto done;
    }
    
    retcode = DDS_RETCODE_OK;
    
done:

    return retcode;
}

RTIBool
NDDSA_Agent_wait_for_semaphoreEA(
    NDDSA_Agent *const self,
    const RTIBool locked_sessions,
    NDDSA_ClientSessionRecord *const session_rec,
    const RTIBool locked_resources,
    NDDSA_ResourceRecord *const resource_rec,
    struct RTIOsapiSemaphore *sem_event)
{
    RTIBool retcode = RTI_FALSE;
    NDDSA_ClientSessionRecord *s_rec = session_rec;
    RTIBool in_resources = locked_resources,
            in_sessions = locked_sessions;
    NDDSA_ResourceRecord *r_rec = resource_rec;
    D2S2_ResourceId rid = D2S2_RESOURCEID_INITIALIZER;
    D2S2_ClientSessionKey sid = D2S2_CLIENTSESSIONKEY_INITIALIZER;

    sid = session_rec->session.base.key;

    if (resource_rec != NULL)
    {
        if (!D2S2_ResourceId_copy(&rid, &resource_rec->resource.base.id))
        {
            goto done;
        }

        r_rec = NULL;
        if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            goto done;
        }
    }

    if (locked_resources)
    {
        in_resources = RTI_FALSE;
        if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_FALSE /* finish_cursor */))
        {
            goto done;
        }
    }

    s_rec = NULL;
    if (!NDDSA_AgentDb_release_sessionEA(&self->db, &sid))
    {
        goto done;
    }

    if (locked_sessions)
    {
        in_sessions = RTI_FALSE;
        if (!NDDSA_AgentDb_unlock_sessions(&self->db, RTI_FALSE /* finish_cursor */))
        {
            goto done;
        }
    }

    RTIOsapiSemaphore_take(sem_event, RTI_NTP_TIME_INFINITE);
    
    retcode = RTI_TRUE;
    
done:

    if (retcode)
    {
        if (locked_sessions && !in_sessions)
        {
            if (!NDDSA_AgentDb_lock_sessions(
                    &self->db,
                    RTI_FALSE /* start_cursor */,
                    RTI_FALSE /* goto_top */))
            {
                return RTI_FALSE;
            }
        }

        if (s_rec == NULL)
        {
            if (!NDDSA_AgentDb_lookup_sessionEA(
                    &self->db, &sid, &s_rec))
            {
                return RTI_FALSE;
            }
            if (s_rec != session_rec)
            {
                return RTI_FALSE;
            }
        }

        if (locked_resources && !in_resources)
        {
            if (!NDDSA_AgentDb_lock_resources(
                    &self->db,
                    RTI_FALSE /* start_cursor */,
                    RTI_FALSE /* goto_top */))
            {
                return RTI_FALSE;
            }
        }

        if (resource_rec != NULL && r_rec == NULL)
        {
            if (!NDDSA_AgentDb_lookup_resourceEA(
                    &self->db, &rid, &r_rec))
            {
                return RTI_FALSE;
            }
            if (r_rec != resource_rec)
            {
                return RTI_FALSE;
            }
        }
    }
    D2S2_ResourceId_finalize(&rid);
    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_detach_resourceEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    const RTIBool delete_request,
    void *const request_param,
    D2S2_ResourceKind *const resource_kind_out,
    D2S2_ResourceId *const ref_resource_id_out,
    void **const resource_data_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_detach_resourceEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTIBool table_locked = RTI_FALSE,
            delete_resource = RTI_FALSE,
            cancelled_read = RTI_FALSE;
    NDDSA_ResourceRecord *resource_rec = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(request_param);

    if (!NDDSA_AgentDb_lock_resources(
            &self->db,
            RTI_TRUE,
            RTI_FALSE /* goto_top */))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOCK_RESOURCES_FAILED);
        goto done;
    }
    table_locked = RTI_TRUE;

    if (!NDDSA_AgentDb_lookup_resourceEA(
            &self->db, &attached->base.resource, &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec == NULL)
    {
        /* This should never happen */
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "UNKNOWN resource");
        goto done;
    }
    if (resource_rec->resource.attached_count == 0)
    {
        /* This should never happen */
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "unexpected zero attach count");
        goto done;
    }

    if (attached->base.kind == D2S2_RESOURCEKIND_DATAREADER &&
        attached->read_req != NULL)
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_cancel_readEA(
                    self,
                    session_rec,
                    attached,
                    RTI_TRUE /* locked_resource */,
                    resource_rec,
                    NULL))
        {
            goto done;
        }
        cancelled_read = RTI_TRUE;
    }

    resource_rec->resource.attached_count -= 1;

    resource_rec->resource.pending_delete =
            resource_rec->resource.pending_delete || delete_request;

    delete_resource = resource_rec->resource.pending_delete &&
                        resource_rec->resource.attached_count == 0;

    // printf("*** DELETE RESOURCE: id='%s', do_it=%d, count=%d, loaded=%d\n",
    //     resource_rec->resource.base.id.value.ref,
    //     delete_resource,
    //     resource_rec->resource.attached_count,
    //     resource_rec->resource.loaded);

    if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
        resource_rec = NULL;
        goto done;
    }
    resource_rec = NULL;

    if (cancelled_read)
    {
        D2S2_AgentServerInterface_on_read_complete(
            src,
            &self->base,
            &session_rec->session.base,
            session_rec->session.user_data,
            attached->base.id,
            attached->user_data);
    }

    if (delete_resource)
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_delete_native_resourceEA(
                    self, &attached->base.resource))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "FAILED to delete native resource");
            goto done;
        }
        if (!NDDSA_AgentDb_delete_resourceEA(
                &self->db, &attached->base.resource))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_DELETE_RESOURCE_FAILED);
            goto done;
        }
    }

    if (ref_resource_id_out != NULL)
    {
        if (!D2S2_ResourceId_copy(ref_resource_id_out, &attached->base.resource))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "FAILED to copy resource id");
            goto done;
        }
    }
    if (resource_data_out != NULL)
    {
        *resource_data_out = attached->user_data;
    }
    if (resource_kind_out != NULL)
    {
        *resource_kind_out = attached->base.kind;
    }

    D2S2_ResourceId_finalize(&attached->base.resource);
    REDAFastBufferPool_returnBuffer(self->pool_resources, attached);
    
    retcode = DDS_RETCODE_OK;

done:

    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }
    if (table_locked)
    {
        if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE /* finish_cursor */))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_UNLOCK_RESOURCES_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}


DDS_ReturnCode_t
NDDSA_Agent_attach_resource_to_all_sessionsEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceId *const resource_id)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_attach_resource_to_all_sessionsEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_ClientSessionRecord *session_rec = NULL,
                              *prev_session_rec = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;
    RTIBool sessions_locked = RTI_FALSE,
            notify = RTI_FALSE;
    NDDSA_AttachedResource *attached = NULL;
    D2S2_AttachedResourceId attach_id = D2S2_ATTACHEDRESOURCEID_INVALID;

    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_lock_sessions(
            &self->db,
            RTI_TRUE /* start_cursor */,
            RTI_TRUE /* goto_top */))
    {
        goto done;
    }
    sessions_locked = RTI_TRUE;

    do 
    {
        if (!NDDSA_AgentDb_find_next_session(
                &self->db, NULL, NULL, prev_session_rec, &session_rec))
        {
            goto done;
        }
        prev_session_rec = session_rec;
        if (session_rec != NULL)
        {
            if (!NDDSA_AgentDb_lookup_resource(
                    &self->db, resource_id, &resource_rec))
            {
                goto done;
            }

            if (DDS_RETCODE_OK !=
                    D2S2_Agent_generate_attached_resource_id(
                        &self->base,
                        &resource_rec->resource.base.id,
                        resource_rec->resource.base.kind,
                        &attach_id))
            {
                goto done;
            }

            NDDSA_ClientSession_find_attached_resource(
                &session_rec->session, attach_id, &attached);
            
            if (attached == NULL)
            {
                if (!NDDSA_Agent_attach_resource_to_sessionEA(
                        self, session_rec, attach_id, resource_rec, &attached))
                {
                    goto done;
                }
                notify = RTI_TRUE;
            }

            if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
            {
                /* TODO log */
                resource_rec = NULL;
                goto done;
            }
            resource_rec = NULL;

            if (notify)
            {
                void *resource_data = NULL;

                notify = RTI_FALSE;

                if (DDS_RETCODE_OK !=
                        D2S2_AgentServerInterface_on_resource_created(
                            session_rec->session.intf,
                            &self->base,
                            &session_rec->session.base,
                            session_rec->session.user_data,
                            attached->base.kind,
                            &attached->base.resource,
                            attached->base.id,
                            NULL,
                            &resource_data))
                {
                    NDDSA_Agent_detach_resourceEA(
                        self,
                        session_rec->session.intf,
                        session_rec,
                        attached,
                        RTI_FALSE,
                        NULL,
                        NULL,
                        NULL,
                        NULL);
                    goto done;
                }

                attached->user_data = resource_data;
            }
        }
    } while (session_rec != NULL);

    
    retcode = DDS_RETCODE_OK;
    
done:

    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            /* TODO log */
            retcode = DDS_RETCODE_ERROR;
        }
    }
    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_sessionEA(
                &self->db,
                &session_rec->session.base.key))
        {
            /* TODO log */
            retcode = DDS_RETCODE_ERROR;
        }
    }
    if (sessions_locked)
    {
        if (!NDDSA_AgentDb_unlock_sessions(
                &self->db,
                RTI_TRUE /* finish_cursor */))
        {
            /* TODO log */
            retcode = DDS_RETCODE_ERROR;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_attach_all_resources_to_sessionEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_attach_all_resources_to_sessionEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_ResourceRecord *resource_rec = NULL;
    NDDSA_AttachedResource *attached = NULL;
    RTIBool table_locked = RTI_FALSE;
    void *resource_data = NULL;
    D2S2_AttachedResourceId attach_id = D2S2_ATTACHEDRESOURCEID_INVALID;

    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_lock_resources(
            &self->db,
            RTI_TRUE /* start_cursor */,
            RTI_TRUE /* goto_top */))
    {
        goto done;
    }
    table_locked = RTI_TRUE;

    do
    {
        if (resource_rec != NULL)
        {
            if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
            {
                resource_rec = NULL;
                goto done;
            }
            resource_rec = NULL;
        }
        
        if (!NDDSA_AgentDb_find_next_resourceEA(
                &self->db, NULL, NULL, &resource_rec))
        {
            goto done;
        }

        if (resource_rec != NULL &&
            !resource_rec->resource.native_deleted &&
            self->properties.auto_attach_resources)
        {
            if (DDS_RETCODE_OK !=
                    D2S2_Agent_generate_attached_resource_id(
                        &self->base,
                        &resource_rec->resource.base.id,
                        resource_rec->resource.base.kind,
                        &attach_id))
            {
                goto done;
            }

            // printf("[AUTO-ID] resource='%s', id=%08X\n",
            //     resource_rec->resource.base.id.value.ref, attach_id);

            NDDSA_ClientSession_find_attached_resource(
                &session_rec->session, attach_id, &attached);
            
            if (attached == NULL)
            {
                if (DDS_RETCODE_OK !=
                        NDDSA_Agent_attach_resource_to_sessionEA(
                            self,
                            session_rec,
                            attach_id,
                            resource_rec,
                            &attached))
                {
                    goto done;
                }

                if (DDS_RETCODE_OK !=
                        NDDSA_Agent_enable_resourceEA(self, resource_rec))
                {
                    D2S2Log_exception(
                        method_name,
                        &RTI_LOG_ANY_FAILURE_s,
                        "NDDSA_Agent_enable_resourceEA");
                    goto done;
                }

                if (DDS_RETCODE_OK !=
                        D2S2_AgentServerInterface_on_resource_created(
                            session_rec->session.intf,
                            &self->base,
                            &session_rec->session.base,
                            session_rec->session.user_data,
                            attached->base.kind,
                            &attached->base.resource,
                            attached->base.id,
                            NULL,
                            &resource_data))
                {
                    NDDSA_Agent_detach_resourceEA(
                        self,
                        session_rec->session.intf,
                        session_rec,
                        attached,
                        RTI_FALSE,
                        NULL,
                        NULL,
                        NULL,
                        NULL);
                    goto done;
                }

                attached->user_data = resource_data;
            }
        }
    } while (resource_rec != NULL);
    
    retcode = DDS_RETCODE_OK;
    
done:

    if (resource_rec != NULL)
    {
        if (NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            retcode = DDS_RETCODE_ERROR;
        }
    }
    if (table_locked)
    {
        if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE))
        {
            /* TODO log */
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_attach_resource_to_sessionEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    const D2S2_AttachedResourceId resource_id,
    NDDSA_ResourceRecord *const resource_rec,
    NDDSA_AttachedResource **const attached_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_attach_resource_to_sessionEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    const NDDSA_AttachedResource def_attached =
                NDDSA_ATTACHEDRESOURCE_INITIALIZER;
    NDDSA_AttachedResource *attached = NULL;
    D2S2_AttachedResourceId attach_id = resource_id;
    const struct DDS_SequenceNumber_t def_sn = DDS_SEQUENCENUMBER_DEFAULT;

    D2S2Log_fn_entry()
    
    if (attach_id == D2S2_ATTACHEDRESOURCEID_INVALID)
    {
        if (DDS_RETCODE_OK !=
                    D2S2_Agent_generate_attached_resource_id(
                        &self->base,
                        &resource_rec->resource.base.id,
                        resource_rec->resource.base.kind,
                        &attach_id))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_AGENT_GENERATE_RESOURCE_ID_FAILED);
            goto done;
        }
    }

    attached = (NDDSA_AttachedResource*)
                    REDAFastBufferPool_getBuffer(self->pool_resources);
    if (attached == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_ss,
            D2S2_LOG_MSG_ALLOC_BUFFER_FAILED,
            "attached resources");
        goto done;
    }
    *attached = def_attached;

    attached->base.kind = resource_rec->resource.base.kind;
    attached->base.id = resource_id;
    attached->base.client = session_rec->session.base.key.client;
    attached->base.session = session_rec->session.base.key.id;
    attached->session = &session_rec->session;
    attached->last_sample_forwarded = def_sn;
    attached->last_sample_returned = def_sn;

    if (!D2S2_ResourceId_copy(
            &attached->base.resource, &resource_rec->resource.base.id))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_ss,
            D2S2_LOG_MSG_AGENT_COPY_FAILED,
            "resource id");
        goto done;
    }

    REDAInlineList_addNodeToBackEA(
        &session_rec->session.resources, &attached->node);

    resource_rec->resource.attached_count += 1;

    // printf("*** ATTACH RESOURCE: ptr=%p, id='%s', count=%d, loaded=%d\n",
    //     resource_rec,
    //     resource_rec->resource.base.id.value.ref,
    //     resource_rec->resource.attached_count,
    //     resource_rec->resource.loaded);
    
    *attached_out = attached;

    retcode = DDS_RETCODE_OK;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_enable_resourceEA(
    NDDSA_Agent *const self,
    NDDSA_ResourceRecord *const resource_rec)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Publisher *publisher = NULL;
    DDS_Subscriber *subscriber = NULL;
    DDS_DataWriter *writer = NULL;
    DDS_DataReader *reader = NULL;
    DDS_Topic *topic = NULL;

    UNUSED_ARG(self);

    switch (resource_rec->resource.base.kind)
    {
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(
                    DDS_DomainParticipant_as_entity(
                        resource_rec->resource.native.value.entity.participant)))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        publisher = resource_rec->resource.native.value.entity.publisher;

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(
                    DDS_DomainParticipant_as_entity(
                        DDS_Publisher_get_participant(publisher))))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(DDS_Publisher_as_entity(publisher)))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        subscriber = resource_rec->resource.native.value.entity.subscriber;

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(
                    DDS_DomainParticipant_as_entity(
                        DDS_Subscriber_get_participant(subscriber))))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(DDS_Subscriber_as_entity(subscriber)))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        writer = resource_rec->resource.native.value.entity.writer;
        publisher = DDS_DataWriter_get_publisher(writer);

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(
                    DDS_DomainParticipant_as_entity(
                        DDS_Publisher_get_participant(publisher))))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(DDS_Publisher_as_entity(publisher)))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(DDS_DataWriter_as_entity(writer)))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        reader = resource_rec->resource.native.value.entity.reader;
        subscriber = DDS_DataReader_get_subscriber(reader);

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(
                    DDS_DomainParticipant_as_entity(
                        DDS_Subscriber_get_participant(subscriber))))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(DDS_Subscriber_as_entity(subscriber)))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(DDS_DataReader_as_entity(reader)))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        topic = resource_rec->resource.native.value.entity.topic;
        
        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(
                    DDS_DomainParticipant_as_entity(
                        DDS_TopicDescription_get_participant(
                            DDS_Topic_as_topicdescription(topic)))))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                DDS_Entity_enable(DDS_Topic_as_entity(topic)))
        {
            goto done;
        }
        break;
    }
    default:
        break;
    }
    
    retcode = DDS_RETCODE_OK;
    
done:

    return retcode;
}

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
    NDDSA_AttachedResource **const attached_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_create_resourceEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_AttachedResource *attached = NULL,
                           *attached_parent = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;
    RTIBool action_delete_existing = RTI_FALSE,
            compatible_repr = RTI_FALSE,
            table_locked = RTI_FALSE,
            cancelled_read = RTI_FALSE;
    D2S2_ResourceId *parent_res_id = NULL;
    D2S2_ResourceRepresentation existing_resource_repr =
        D2S2_RESOURCEREPRESENTATION_INITIALIZER;
    
    D2S2Log_fn_entry()

    UNUSED_ARG(request_param);

    if (resource_repr->fmt == D2S2_RESOURCEREPRESENTATIONFORMAT_UNKNOWN ||
        resource_repr->fmt == D2S2_RESOURCEREPRESENTATIONFORMAT_BIN ||
        resource_repr->fmt == D2S2_RESOURCEREPRESENTATIONFORMAT_JSON)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_REPRESENTATION_FORMAT);
        goto done;
    }

    /* Check that the parent is valid based on the resource kind */
    switch (kind)
    {
    case D2S2_RESOURCEKIND_APPLICATION:
    case D2S2_RESOURCEKIND_QOSPROFILE:
    case D2S2_RESOURCEKIND_DOMAIN:
    case D2S2_RESOURCEKIND_TYPE:
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        /* these types of resources don't have a parent */
        if (parent_id != D2S2_ATTACHEDRESOURCEID_INVALID)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_AGENT_UNEXPECTED_PARENT_RESOURCE);
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    case D2S2_RESOURCEKIND_PUBLISHER:
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    case D2S2_RESOURCEKIND_DATAWRITER:
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        /* NON-STANDARD: Allow clients to create an entity by specifying 
           a full reference and an invalid parent_id */
#if 0
        /* these types of resources must have a parent */
        if (parent_id == D2S2_ATTACHEDRESOURCEID_INVALID)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_AGENT_MISSING_PARENT_RESOURCE);
            goto done;
        }
#endif
        break;
    }
    default:
    {
        D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND);
        goto done;
    }
    }

    /* If a parent was specified, look it up already, since it must exist
       or the call will fail */
    if (parent_id != D2S2_ATTACHEDRESOURCEID_INVALID)
    {
        NDDSA_ClientSession_find_attached_resource(
            &session_rec->session, parent_id, &attached_parent);
        if (attached_parent == NULL)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_ss,
                D2S2_LOG_MSG_AGENT_ATTACHED_RESOURCE_NOT_FOUND,
                "parent");
            goto done;
        }

        /* Check that the parent is valid based on the resource kind */
        switch (kind)
        {
        case D2S2_RESOURCEKIND_TOPIC:
        {
            if (attached_parent->base.kind !=
                    D2S2_RESOURCEKIND_DOMAINPARTICIPANT)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
                    "parent");
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_PUBLISHER:
        {
            if (attached_parent->base.kind !=
                    D2S2_RESOURCEKIND_DOMAINPARTICIPANT)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
                    "parent");
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_SUBSCRIBER:
        {
            if (attached_parent->base.kind !=
                    D2S2_RESOURCEKIND_DOMAINPARTICIPANT)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
                    "parent");
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_DATAWRITER:
        {
            if (attached_parent->base.kind != D2S2_RESOURCEKIND_PUBLISHER)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
                    "parent");
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_DATAREADER:
        {
            if (attached_parent->base.kind != D2S2_RESOURCEKIND_SUBSCRIBER)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
                    "parent");
                goto done;
            }
            break;
        }
        default:
        {
            /* should never get here */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_s,
                "unexpected resource kind");
            goto done;
        }
        }

        parent_res_id = &attached_parent->base.resource;
    }

    if (resource_id != D2S2_ATTACHEDRESOURCEID_INVALID)
    {
        /* Check if a resource is already attached to the session with the
           specifed id */
        NDDSA_ClientSession_find_attached_resource(
            &session_rec->session, resource_id, &attached);

        if (attached != NULL)
        {
            /* another resource already exists with the specified id.
            What we do next depends on the properties specified by the caller.
            It might be an error, or we might just reuse the entity "as is",
            or we might delete it and recreate it */
            D2S2Log_warn(
                method_name,
                &RTI_LOG_ANY_s,
                D2S2_LOG_MSG_AGENT_RESOURCE_ALREADY_ATTACHED);
            
            /* A resource may only be replaced by another one of the same kind */
            if (attached->base.kind != kind)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND);
                goto done;
            }

            if (!(properties->reuse || properties->replace))
            {
                /* no "reuse" and no "replace" means we cannot delete/recreate
                * the existing resource, nor reuse it. */
               D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_AGENT_REUSE_REPLACE_DISABLED);
                goto done;
            }
        }
    }
    if (!NDDSA_AgentDb_lock_resources(
            &self->db,
            RTI_TRUE,
            RTI_FALSE /* goto_top */))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOCK_RESOURCES_FAILED);
        goto done;
    }
    table_locked = RTI_TRUE;

    if (attached != NULL)
    {
        if (!NDDSA_AgentDb_lookup_resourceEA(
                &self->db, &attached->base.resource, &resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
            goto done;
        }
        if (resource_rec == NULL)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_ss,
                D2S2_LOG_MSG_AGENT_RESOURCE_NOT_FOUND,
                "pre-attached resource");
            goto done;
        }

        if (properties->reuse)
        {
            /* "reuse" means we should check if the current representation is
                * compatible with the existing resource. If it isn't, then we
                * must check "replace":
                *   - if "replace" is set, we will delete the existing resource
                *     and recreate it.
                *   - if "replace" is not set, then we cannot create the resource.
                */

            if (!NDDSA_Resource_convert_representation(
                    &resource_rec->resource,
                    &resource_rec->resource.native.create_repr,
                    resource_repr->fmt,
                    &existing_resource_repr))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_AGENT_CONVERT_RESOURCE_REPRESENTATION_FAILED);
                goto done;
            }
            if (existing_resource_repr.fmt != resource_repr->fmt)
            {
                /* (in theory, once all things are in place) this should
                    * never happen */
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_REPRESENTATION_FORMAT,
                    "pre-attached resource");
                goto done;
            }

            switch (resource_repr->fmt)
            {
            case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
            {
                compatible_repr = 
                    (strcmp(
                        resource_repr->value.xml,
                        existing_resource_repr.value.xml) == 0);
                break;
            }
            case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
            {
                compatible_repr = 
                    (strcmp(
                        resource_repr->value.ref,
                        existing_resource_repr.value.ref) == 0);
                break;
            }
            default:
            {
                /* TODO implement comparison of other formats */
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_REPRESENTATION_FORMAT,
                    "unsupported comparison");
                goto done;
            }
            }

            if (!compatible_repr)
            {
                if (!properties->replace)
                {
                    D2S2Log_exception(
                        method_name,
                        &RTI_LOG_ANY_FAILURE_s,
                        D2S2_LOG_MSG_AGENT_INCOMPATIBLE_RESOURCE_REPRESENTATION);
                    goto done;
                }

                action_delete_existing = RTI_TRUE;
            }
        }
        else
        {
            /* "replace" must be TRUE or it would have been caught by 
                * previous block. We will delete the native entity and
                * recreate it. */
            action_delete_existing = RTI_TRUE;
        }


        if (action_delete_existing)
        {
#if 1
            /* Currently we don't support deleting and recreating an entity
            because we have no good way of going from an "entity" to a
            "resource id", which we would need in order to recreate all
            contained entities after the parent entity is recreated.
            
            Ideally, we would delete all contained entities and mark
            any associated resource record as "native delted". Once the
            new entity is created, then we could iterate over its
            contained entities, convert them to "resource ids", see if
            a record exists for that id, and update its native entity
            and clear the "native deleted* state.
            
            After that's done, we could release the resource table, and
            iterate over each resource and detach and delete any resource
            that is still marked as "native deleted".
            
            But since this cannot be implemented, we just return error. */

            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "deletion of existing resources NOT IMPLEMENTED");
            goto done;
#else
            /* We return the resource record and trigger deletion of the
                associated native entity. The resource table will be locked,
                and we will look up any resource that might existing for one
                of the entities contained by the one being deleted (which will
                also be deleted in turn). We will mark those resource as
                "native_deleted" so that any operation on them will fail */
            
            if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
            {
                resource_rec = NULL;
                goto done;
            }
            resource_rec = NULL;

            if (DDS_RETCODE_OK !=
                    NDDSA_Agent_delete_native_resourceEA(
                        self, &attached->base.resource))
            {
                goto done;
            }
#endif
        }
        else
        {
            /* cancel any existing read request (ideally this would be done by
            the resource deletion code path) */
            if (attached->read_req != NULL)
            {
                if (DDS_RETCODE_OK !=
                        NDDSA_Agent_cancel_readEA(
                            self,
                            session_rec,
                            attached,
                            RTI_TRUE /* locked_resource */,
                            resource_rec,
                            NULL))
                {
                    goto done;
                }
                cancelled_read = RTI_TRUE;
            }
        }
    }

    if (resource_rec == NULL)
    {
        /* The resource either doesn't exist or it was already deleted */
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_insert_resource_recordEA(
                    self,
                    kind,
                    resource_repr,
                    parent_res_id,
                    properties,
                    &resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_insert_resource_recordEA");
            goto done;
        }
        if (resource_rec == NULL)
        {
            /* The resource exists but was already deleted, and it must be
               disposed before it can be created again.
               It's unlikely that we will ever get here, because there will
               likely be problem with ResouceFactory_create_native since it
               will likely fail to add the XML entry */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_s,
                "Resource pending deletion");
            goto done;
        }
    }

    if (attached == NULL)
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_attach_resource_to_sessionEA(
                    self, session_rec, resource_id, resource_rec, &attached))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_attach_resource_to_sessionEA");
            goto done;
        }
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_enable_resourceEA(self, resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "NDDSA_Agent_enable_resourceEA");
        goto done;
    }

    if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
        resource_rec = NULL;
        goto done;
    }
    resource_rec = NULL;

    table_locked = RTI_FALSE;
    if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE /* finish_cursor */))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_UNLOCK_RESOURCES_FAILED);
        goto done;
    }

    if (cancelled_read)
    {
        D2S2_AgentServerInterface_on_read_complete(
            src,
            &self->base,
            &session_rec->session.base,
            session_rec->session.user_data,
            attached->base.id,
            attached->user_data); 
    }

    *attached_out = attached;
    
    retcode = DDS_RETCODE_OK;
    
done:
    D2S2_ResourceRepresentation_finalize(&existing_resource_repr);

    if (resource_rec != NULL)
    {
        if (table_locked)
        {
            if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
            {
                /* TODO log */
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
                retcode = DDS_RETCODE_ERROR;
            }
        }
        else
        {
            if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
            {
                /* TODO log */
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
                retcode = DDS_RETCODE_ERROR;
            }
        }
    }
    if (table_locked)
    {
        if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE /* finish_cursor */))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_UNLOCK_RESOURCES_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_readEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    const D2S2_ReadSpecification *const read_spec,
    void *const request_param,
    void **const resource_data_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_readEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_ResourceRecord *resource_rec = NULL;
    const NDDSA_Read def_read = NDDSA_READ_INITIALIZER;
    NDDSA_Reader *reader_data = NULL;
    const struct RTINtpTime ts_zero = RTI_NTP_TIME_ZERO;
        
    D2S2Log_fn_entry()

    UNUSED_ARG(src);
    
    if (!NDDSA_AgentDb_lookup_resource(
            &self->db, &attached->base.resource, &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "UNKNOWN resource");
        goto done;
    }
    if (resource_rec->resource.native_deleted)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "native resource ALREADY DELETED");
        goto done;
    }

    reader_data = (NDDSA_Reader*)resource_rec->resource.user_data;

    if (attached->read_req != NULL)
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_cancel_readEA(
                    self,
                    session_rec,
                    attached,
                    RTI_FALSE /* locked_resources */,
                    resource_rec,
                    request_param))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "FAILED to cancel read requests");
            goto done;
        }
    }

    attached->read_req = 
        (NDDSA_Read*) REDAFastBufferPool_getBuffer(self->pool_reads);
    if (attached->read_req == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_ss,
            D2S2_LOG_MSG_ALLOC_BUFFER_FAILED,
            "read_request state");
        goto done;
    }
    *attached->read_req = def_read;

    attached->read_req->agent = self;
    attached->read_req->session_key = session_rec->session.base.key;
    attached->read_req->reader_id = attached->base.id;
    attached->read_req->event_listener.onEvent =
        NDDSA_Agent_on_data_on_reader;
    attached->read_req->event_listener_storage.field[0] = attached->read_req;
    attached->read_req->event_listener_storage.field[1] = attached->read_req;
    attached->read_req->sem_event =
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    if (attached->read_req->sem_event == NULL)
    {
        goto done;
    }

    switch (self->properties.read_start_point)
    {
    case NDDSA_READSTARTPOINT_FIRST_CACHED:
    {
        /* Nothing to do, read_req->last_sample == SEQNUM_DEFAULT */
        break;
    }
    case NDDSA_READSTARTPOINT_LAST_RETURNED:
    {
        attached->read_req->last_sample = attached->last_sample_returned;
        break;
    }
    case NDDSA_READSTARTPOINT_LAST_FORWARDED:
    {
        attached->read_req->last_sample = attached->last_sample_forwarded;
        break;
    }
    default:
        break;
    }

    
    if (read_spec != NULL)
    {
        if (read_spec->min_pace_period > 0)
        {
            DDS_UnsignedLong pace_s = 0,
                            pace_ms = read_spec->min_pace_period;
            
            pace_s = read_spec->min_pace_period / 1000;
            pace_ms = read_spec->min_pace_period - (1000 * pace_s);

            RTINtpTime_packFromMillisec(attached->read_req->sample_pace, pace_s, pace_ms);
        }
        if (read_spec->max_elapsed_time > 0)
        {
            DDS_UnsignedLong s = 0,
                            ms = read_spec->max_elapsed_time;
            
            s = read_spec->max_elapsed_time / 1000;
            ms = read_spec->max_elapsed_time - (1000 * s);

            RTINtpTime_packFromMillisec(attached->read_req->max_elapsed_ts, s, ms);
        }
        if (read_spec->content_filter_expr != NULL)
        {
            attached->read_req->content_filter_expr =
                DDS_String_dup(read_spec->content_filter_expr);
            if (attached->read_req->content_filter_expr == NULL)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    "FAILED to duplicate string");
                goto done;
            }
        }
        attached->read_req->samples_max = read_spec->max_samples;
    }
    
    if (!self->clock->getTime(self->clock, &attached->read_req->start_ts))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "FAILED to get timestamp from clock");
        goto done;
    }

    if (!NDDSA_Reader_new_request(reader_data, attached->read_req))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "FAILED to create new reader request");
        goto done;
    }

    /* Trigger the read request to process samples that might already be
       in the data reader's cache */
    if (!NDDSA_Agent_post_event(
            self,
            &attached->read_req->event_listener,
            &attached->read_req->event_listener_storage,
            sizeof(NDDSA_Read*),
            &ts_zero))
    {
        goto done;
    }

    if (resource_data_out != NULL)
    {
        *resource_data_out = attached->user_data;
    }

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        if (attached != NULL && attached->read_req != NULL)
        {
            /* TODO finalize read */
            REDAFastBufferPool_returnBuffer(
                self->pool_reads, attached->read_req);
        }
    }
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}


DDS_ReturnCode_t
NDDSA_Agent_cancel_readEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    const RTIBool locked_resources,
    NDDSA_ResourceRecord *const resource_rec,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_cancel_readEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Reader *reader_data = NULL;
    const struct RTINtpTime ts_zero = RTI_NTP_TIME_ZERO;
    
    D2S2Log_fn_entry()

    UNUSED_ARG(request_param);

    reader_data = (NDDSA_Reader*)resource_rec->resource.user_data;

    if (attached->read_req == NULL)
    {
        retcode = DDS_RETCODE_OK;
        goto done;
    }
    else
    {
        /* we can't delete the read if the caller has not return some received
           samples (they should have returned the loan(s) before recreating
           the subscription) */
        
        if (NDDSA_Read_has_pending_samples(attached->read_req))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_s,
                "cannot cancel read with cached samples");
            goto done;
        }

        if (!NDDSA_Reader_request_complete(reader_data, attached->read_req))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_s,
                "FAILED to delete request on DDS reader");
            goto done;
        }

        D2S2Log_warn(
            method_name,
            &RTI_LOG_ANY_s,
            "CANCELLING read event");
        
        attached->read_req->event_listener_storage.field[1] = NULL;

        if (!NDDSA_Agent_post_event(
                self,
                &attached->read_req->event_listener,
                &attached->read_req->event_listener_storage,
                sizeof(NDDSA_Read*),
                &ts_zero))
        {
            goto done;
        }

        if (!NDDSA_Agent_wait_for_semaphoreEA(
                self,
                RTI_FALSE,
                session_rec,
                locked_resources,
                resource_rec,
                attached->read_req->sem_event))
        {
            goto done;
        }

        if (attached->read_req->content_filter_expr != NULL)
        {
            DDS_String_free(attached->read_req->content_filter_expr);
        }

        RTIOsapiSemaphore_delete(attached->read_req->sem_event);

        REDAFastBufferPool_returnBuffer(
                self->pool_reads, attached->read_req);
        
        attached->read_req = NULL;
    }

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        
    }
    D2S2Log_fn_exit()
    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_writeEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *attached,
    const D2S2_DataRepresentation *const data,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_writeEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_ResourceRecord *resource_rec = NULL;
    NDDSA_Writer *writer_data = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(request_param);
    UNUSED_ARG(session_rec);
    UNUSED_ARG(src);

    if (data->fmt != D2S2_DATAREPRESENTATIONFORMAT_XCDR)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "UNSUPPORTED data format");
        goto done;
    }
    
    if (!NDDSA_AgentDb_lookup_resource(
            &self->db, &attached->base.resource, &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "UNKNOWN resource");
        goto done;
    }
    if (resource_rec->resource.native_deleted)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "native resource ALREADY DELETED");
        goto done;
    }

    writer_data = (NDDSA_Writer*)resource_rec->resource.user_data;

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_buffer_to_dyndata(
                writer_data, data, writer_data->dyn_sample))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "FAILED to convert buffer into DynamicData sample");
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_DynamicDataWriter_write(
                writer_data->dyn_writer,
                writer_data->dyn_sample,
                &DDS_HANDLE_NIL))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "FAILED to write data with DDS DataWriter");
        goto done;
    }

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        
    }
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }

    D2S2Log_fn_exit()
    return retcode;
}


DDS_ReturnCode_t
NDDSA_Agent_create_implicit_resources(
    NDDSA_Agent *const self,
    const D2S2_ResourceProperties *const user_properties)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_create_implicit_resources)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTIBool table_locked = RTI_FALSE;
    D2S2_ResourceProperties properties = D2S2_RESOURCEPROPERTIES_INITIALIZER;
    NDDSA_XmlResourceVisitor visitor = NDDSA_XMLRESOURCEVISITOR_INITIALIZER;
    struct NDDSA_CreatedResourceLogSeq created_resources = DDS_SEQUENCE_INITIALIZER;
    NDDSA_CreatedResourceLog *resource_log = NULL;
    DDS_UnsignedLong seq_len = 0,
                     i = 0;
    D2S2_AttachedResourceId attach_id = D2S2_ATTACHEDRESOURCEID_INVALID;
    
    D2S2Log_fn_entry()

    if (user_properties != NULL)
    {
        properties = *user_properties;
    }

    NDDSA_XmlResourceVisitor_initialize(
        &visitor, &properties, &created_resources);

    if (!NDDSA_AgentDb_lock_resources(
            &self->db,
            RTI_TRUE,
            RTI_FALSE /* goto_top */))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOCK_RESOURCES_FAILED);
        goto done;
    }
    table_locked = RTI_TRUE;

    if (!NDDSA_Agent_visit_xml(self, &visitor.base))
    {
        goto done;
    }

    seq_len = NDDSA_CreatedResourceLogSeq_get_length(&created_resources);
    
    for (i = 0; i < seq_len; i++)
    {
        resource_log =
            NDDSA_CreatedResourceLogSeq_get_reference(&created_resources, i);
        
        if (DDS_RETCODE_OK !=
                    D2S2_Agent_generate_attached_resource_id(
                        &self->base,
                        &resource_log->id,
                        resource_log->kind,
                        &attach_id))
        {
            goto done;
        }
        D2S2Log_freeForm(RTI_LOG_BIT_EXCEPTION)(
            "DEFAULT ID\t0x%04X\t'%s'\n", 
            attach_id, resource_log->id.value.ref);
    }
    
    
    retcode = DDS_RETCODE_OK;
    
done:

    if (table_locked)
    {
        if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_UNLOCK_RESOURCES_FAILED);
            retcode = DDS_RETCODE_OK;
        }
    }
    NDDSA_CreatedResourceLogSeq_finalize(&created_resources);
    D2S2Log_fn_exit()
    return retcode;
}


/******************************************************************************
 *                      Private function implementations
 ******************************************************************************/

RTI_PRIVATE
RTIBool
NDDSA_Agent_finalize_database_sessions(NDDSA_Agent *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_finalize_database_sessions)
    RTIBool retcode = RTI_FALSE,
            sessions_locked = RTI_FALSE;
    NDDSA_ClientSessionRecord *session_rec = NULL,
                              *prev_session_rec = NULL;
    D2S2_AgentServerInterface *src = NULL;
    D2S2_ClientSessionKey session_key = D2S2_CLIENTSESSIONKEY_INITIALIZER;
    void *session_data = NULL;

    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_lock_sessions(
            &self->db,
            RTI_TRUE /* start_cursor */,
            RTI_TRUE /* goto_top */))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOCK_SESSIONS_FAILED);
        goto done;
    }
    sessions_locked = RTI_TRUE;

    if (!NDDSA_AgentDb_find_next_session_and_delete_previous(
            &self->db, NULL, NULL, NULL, &session_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "NDDSA_AgentDb_find_next_session_and_delete_previous");
        goto done;
    }

    while (session_rec != NULL)
    {
        prev_session_rec = session_rec;

        session_data = session_rec->session.user_data;
        src = session_rec->session.intf;
        session_key = session_rec->session.base.key;

        D2S2_AgentServerInterface_on_session_timed_out(
            src,
            &self->base,
            &session_rec->session.base,
            session_rec->session.user_data);

        if (DDS_RETCODE_OK != 
                NDDSA_Agent_dispose_sessionEA(self, session_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_dispose_sessionEA");
            goto done;
        }

        if (!NDDSA_Agent_wait_for_semaphoreEA(
                self,
                RTI_TRUE,
                session_rec,
                RTI_FALSE,
                NULL,
                session_rec->session.sem_cleanup))
        {
            goto done;
        }
        RTIOsapiSemaphore_delete(session_rec->session.sem_cleanup);
        session_rec->session.sem_cleanup = NULL;

        session_rec = NULL;
        if (!NDDSA_AgentDb_find_next_session_and_delete_previous(
                &self->db, NULL, NULL, prev_session_rec, &session_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_AgentDb_find_next_session_and_delete_previous");
            goto done;
        }

        D2S2_AgentServerInterface_on_session_closed(
            src,
            &self->base,
            &session_key,
            session_data);
    }
    
    retcode = RTI_TRUE;
    
done:

    if (sessions_locked)
    {
        if (!NDDSA_AgentDb_unlock_sessions(&self->db, RTI_TRUE))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_UNLOCK_SESSIONS_FAILED);
            retcode = RTI_FALSE;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_finalize_database_resources(NDDSA_Agent *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_finalize_database_resources)
    RTIBool retcode = RTI_FALSE;
    NDDSA_ResourceRecord *resource_rec = NULL;
    RTIBool table_locked = RTI_FALSE;
    D2S2_ResourceId resource_id = D2S2_RESOURCEID_INITIALIZER;

    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_lock_resources(
            &self->db,
            RTI_TRUE /* start_cursor */,
            RTI_TRUE /* goto_top */))
    {
        goto done;
    }
    table_locked = RTI_TRUE;

    if (!NDDSA_AgentDb_find_next_resourceEA(
            &self->db, NULL, NULL, &resource_rec))
    {
        goto done;
    }

    while (resource_rec != NULL)
    {
        /* All sessions should have been already deleted by the time this 
           function is called, so all resources should have an attached_count
           of 0 */
        if (resource_rec->resource.attached_count > 0)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_s,
                "unexpected non-zero attach count");
            goto done;
        }

        if (!D2S2_ResourceId_copy(
                &resource_id, &resource_rec->resource.base.id))
        {
            goto done;
        }

        if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            resource_rec = NULL;
            goto done;
        }
        resource_rec = NULL;

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_delete_native_resourceEA(self, &resource_id))
        {
            goto done;
        }

        if (!NDDSA_AgentDb_delete_resourceEA(&self->db, &resource_id))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_DELETE_RESOURCE_FAILED);
            goto done;
        }

        /* Always go back to top of table, since we are deleting records */
        if (!NDDSA_AgentDb_iterate_resourcesEA(&self->db))
        {
            goto done;
        }

        if (!NDDSA_AgentDb_find_next_resourceEA(
                &self->db, NULL, NULL, &resource_rec))
        {
            goto done;
        }
    }
    
    retcode = RTI_TRUE;
    
done:

    if (resource_rec != NULL)
    {
        if (NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            retcode = DDS_RETCODE_ERROR;
        }
    }
    if (table_locked)
    {
        if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE))
        {
            /* TODO log */
            retcode = DDS_RETCODE_ERROR;
        }
    }
    D2S2_ResourceId_finalize(&resource_id);
    D2S2Log_fn_exit()
    return retcode;
}


RTI_PRIVATE
RTIBool
NDDSA_Agent_finalize_database(NDDSA_Agent *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_finalize_database)
    RTIBool retcode = RTI_FALSE;
    
    D2S2Log_fn_entry()

    if (!NDDSA_Agent_finalize_database_sessions(self))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "NDDSA_Agent_finalize_database_sessions");
        goto done;
    }

    if (!NDDSA_Agent_finalize_database_resources(self))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "NDDSA_Agent_finalize_database_resources");
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_finalize_sessionEA(
    NDDSA_Agent *const self,
    D2S2_AgentServerInterface *const src,
    NDDSA_ClientSessionRecord *const session_rec)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_finalize_sessionEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_AttachedResource *attached = NULL;
    struct REDAInlineListNode *node = NULL;
    void *resource_data = NULL;
    D2S2_ResourceId ref_resource_id = D2S2_RESOURCEID_INITIALIZER;
    D2S2_ResourceKind resource_kind = D2S2_RESOURCEKIND_UNKNOWN;
    NDDSA_ResourceRecord *resource_rec = NULL;
    D2S2_AttachedResourceId resource_id = D2S2_RESOURCEKIND_UNKNOWN;
    const struct RTINtpTime ts_zero = RTI_NTP_TIME_ZERO;

    D2S2Log_fn_entry()

    if (!session_rec->session.deleted)
    {
        /* session should have been marked as "deleted" by
           NDDSA_Agent_dispose_session */
        goto done;
    }

    /* Delete all currently active subscriptions */
    node = REDAInlineList_getFirst(&session_rec->session.resources);

    while (node != NULL)
    {

        attached = (NDDSA_AttachedResource*)
                        NDDSA_AttachedResource_from_node(node);
        
        node = REDAInlineListNode_getNext(&attached->node);

        if (attached->base.kind != D2S2_RESOURCEKIND_DATAREADER ||
            attached->read_req == NULL)
        {
            continue;
        }
        
        if (!NDDSA_AgentDb_lookup_resource(
                &self->db, &attached->base.resource, &resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
            goto done;
        }
        if (resource_rec == NULL)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "UNKNOWN resource");
            continue;
        }
        if (resource_rec->resource.native_deleted)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "native resource ALREADY DELETED");
            if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
                resource_rec = NULL;
                goto done;
            }
            resource_rec = NULL;
            continue;
        }

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_cancel_readEA(
                    self,
                    session_rec,
                    attached,
                    RTI_FALSE /* locked_resources */,
                    resource_rec,
                    NULL))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "FAILED to cancel read on close session");
            goto done;
        }

        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            resource_rec = NULL;
            goto done;
        }
        resource_rec = NULL;

        D2S2_AgentServerInterface_on_read_complete(
            src,
            &self->base,
            &session_rec->session.base,
            session_rec->session.user_data,
            attached->base.id,
            attached->user_data);

    }

    /* Delete all attached resource */
    node = REDAInlineList_getFirst(&session_rec->session.resources);

    while (node != NULL)
    {
        attached = (NDDSA_AttachedResource*)
                        NDDSA_AttachedResource_from_node(node);
        
        node = REDAInlineListNode_getNext(&attached->node);

        resource_id = attached->base.id;

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_detach_resourceEA(
                    self,
                    src,
                    session_rec,
                    attached,
                    RTI_FALSE,
                    NULL,
                    &resource_kind,
                    &ref_resource_id,
                    &resource_data))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "FAILED to delete attached resource");
            goto done;
        }

        D2S2_AgentServerInterface_on_resource_deleted(
            src,
            &self->base,
            &session_rec->session.base,
            session_rec->session.user_data,
            resource_kind,
            &ref_resource_id,
            resource_id,
            resource_data,
            NULL);
    
        D2S2_ResourceId_finalize(&ref_resource_id);
    }

    while (REDAInlineList_getFirst(
                &session_rec->session.rcvd_data_pool) != NULL)
    {
        NDDSA_ReceivedData *rcvd_data = 
            NDDSA_ReceivedData_from_node(
                REDAInlineList_getFirst(
                    &session_rec->session.rcvd_data_pool));
        REDAInlineList_removeNodeEA(
            &session_rec->session.rcvd_data_pool, &rcvd_data->node);
        RTIOsapiHeap_freeBufferAligned(
            rcvd_data->data.value.xcdr.buffer.data);
        REDAFastBufferPool_returnBuffer(self->pool_samples, rcvd_data);

    }

    if (!DDS_Duration_is_infinite(&session_rec->session.props.timeout) &&
        !DDS_Duration_is_zero(&session_rec->session.props.timeout) &&
        !session_rec->session.timedout)
    {
        session_rec->session.timeout_listener_storage.field[1] = NULL;

        if (!NDDSA_Agent_post_event(
                self,
                &session_rec->session.timeout_listener,
                &session_rec->session.timeout_listener_storage,
                sizeof(NDDSA_ClientSession*),
                &ts_zero))
        {
            goto done;
        }

        if (!NDDSA_Agent_wait_for_semaphoreEA(
                self,
                RTI_TRUE,
                session_rec,
                RTI_FALSE,
                NULL,
                session_rec->session.sem_timeout))
        {
            goto done;
        }

        // RTIOsapiSemaphore_take(
        //     session_rec->session.sem_timeout, RTI_NTP_TIME_INFINITE);
    }

    RTIOsapiSemaphore_delete(session_rec->session.sem_timeout);

    retcode = DDS_RETCODE_OK;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
void 
NDDSA_Agent_on_start_generator(
    struct RTIEventActiveGenerator *generator,
    void * param,
    struct REDAWorker *worker)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_start_generator)
    NDDSA_Agent *self = (NDDSA_Agent*)param;

    D2S2Log_fn_entry()

    UNUSED_ARG(generator);
    UNUSED_ARG(worker);

    RTIOsapiSemaphore_give(self->sem_start);

    D2S2Log_fn_exit()
}

RTI_PRIVATE
void 
NDDSA_Agent_on_stop_generator(
    struct RTIEventActiveObject *object,
    void *param, 
    struct REDAWorker *worker)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_stop_generator)
    NDDSA_Agent *self = (NDDSA_Agent*)param;

    D2S2Log_fn_entry()

    UNUSED_ARG(object);
    UNUSED_ARG(worker);

    RTIOsapiSemaphore_give(self->sem_stop);

    D2S2Log_fn_exit()
}


RTI_PRIVATE
RTIBool
NDDSA_Agent_find_next_disposed_session(
    NDDSA_AgentDb *const db,
    void *const record,
    void *const filter_param)
{
    NDDSA_Agent *const self = db->agent;
    NDDSA_ClientSessionRecord *const session_rec = 
        (NDDSA_ClientSessionRecord*)record;
    
    UNUSED_ARG(self);
    UNUSED_ARG(filter_param);
    
    return (session_rec->session.deleted);
}


RTI_PRIVATE
RTIBool
NDDSA_Agent_find_next_session_w_completed_read(
    NDDSA_AgentDb *const db,
    void *const record,
    void *const filter_param)
{
    NDDSA_Agent *const self = db->agent;
    NDDSA_ClientSessionRecord *const session_rec = 
        (NDDSA_ClientSessionRecord*)record;
    struct REDAInlineListNode *node = NULL;
    NDDSA_AttachedResource *attached = NULL;

    UNUSED_ARG(self);
    UNUSED_ARG(filter_param);
    
    node = REDAInlineList_getFirst(&session_rec->session.resources);
    while (node != NULL)
    {
        attached = NDDSA_AttachedResource_from_node(node);
        node = REDAInlineListNode_getNext(node);

        if (attached->read_req != NULL &&
            NDDSA_Read_check_status(
                attached->read_req, NDDSA_READSTATUS_COMPLETE))
        {
            return RTI_TRUE;
        }
    }
    
    return RTI_FALSE;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_find_resource_by_native_resource(
    NDDSA_AgentDb *const self,
    NDDSA_ResourceRecord *const resource_rec,
    void *const p,
    RTIBool *const return_record_out)
{
    RTIBool retcode = RTI_FALSE,
            compatible = RTI_FALSE;
    struct DeleteNativeResourceParam *const param =
        (struct DeleteNativeResourceParam*)p;

    UNUSED_ARG(self);

    *return_record_out = RTI_FALSE;

    if (param->kind != resource_rec->resource.base.kind)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    switch (param->kind)
    {
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        compatible =
            (resource_rec->resource.native.value.entity.reader == 
                param->native_entity.reader);
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        compatible =
            (resource_rec->resource.native.value.entity.writer ==
                param->native_entity.writer);
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        compatible =
            (resource_rec->resource.native.value.entity.subscriber == 
                param->native_entity.subscriber);
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        compatible =
            (resource_rec->resource.native.value.entity.publisher ==
                param->native_entity.publisher);
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        compatible =
            (resource_rec->resource.native.value.entity.topic ==
                param->native_entity.topic);
        break;
    }

    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        compatible =
            (resource_rec->resource.native.value.entity.participant ==
                param->native_entity.participant);
        break;
    }
    default:
        break;
    }

    *return_record_out = compatible;
    
    retcode = RTI_TRUE;
    
done:

    return retcode;
}


RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_find_contained_resource_and_deleteEA(
    NDDSA_Agent *const self,
    const struct DeleteNativeResourceParam *const param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_find_contained_resource_and_deleteEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    D2S2_ResourceId contained_entity_id = D2S2_RESOURCEID_INITIALIZER;
    NDDSA_ResourceRecord *resource_rec = NULL;
    
    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_iterate_resourcesEA(&self->db))\
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_ITERATE_RESOURCES_FAILED);
        goto done;
    }

    if (!NDDSA_AgentDb_find_next_resourceEA(
            &self->db,
            NDDSA_Agent_find_resource_by_native_resource,
            (void*)param,
            &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_FIND_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec != NULL)
    {
        if (!D2S2_ResourceId_copy(
                &contained_entity_id, &resource_rec->resource.base.id))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_ss,
                D2S2_LOG_MSG_AGENT_COPY_FAILED,
                "resource id");
            goto done;
        }
        if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            resource_rec = NULL;
            goto done;
        }
        resource_rec = NULL;
        
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_delete_native_resourceEA(
                    self, &contained_entity_id))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "delete contained native entity");
            goto done;
        }
    }
    else
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_delete_native_resource_entityEA(
                    self, NULL, param->kind, &param->native_entity))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_delete_native_resource_entityEA");
            goto done;
        }
    }
    
    retcode = DDS_RETCODE_OK;
    
done:
    
    D2S2_ResourceId_finalize(&contained_entity_id);
    
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_native_resource_entityEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceId *const resource_id,
    const D2S2_ResourceKind resource_kind,
    const NDDSA_EntityResource *const native_entity)
{
#define MAX_CONTAINED_ENTITIES      1024

    D2S2Log_METHOD_NAME(NDDSA_Agent_delete_native_resource_entityEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0,
                     i = 0;
    struct DDS_SubscriberSeq subs = DDS_SEQUENCE_INITIALIZER;
    struct DDS_PublisherSeq pubs = DDS_SEQUENCE_INITIALIZER;
    struct DDS_DataWriterSeq writers = DDS_SEQUENCE_INITIALIZER;
    struct DDS_DataReaderSeq readers = DDS_SEQUENCE_INITIALIZER;
    DDS_TopicDescription *topic_desc = NULL;
    struct DeleteNativeResourceParam param = DeleteNativeResourceParam_INITIALIZER;
    NDDSA_EntityResource parent_entity = NDDSA_ENTITYRESOURCE_INITIALIZER;

    D2S2Log_fn_entry()

    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "DELETING a DomainParticipant's contained entities: ",
            ((resource_id != NULL)?
                resource_id->value.ref : "not a DDS Resource"));
        
        /* Iterate over the DomainParticipant's subscribers and delete any
           resource that have might be created for them */
        if (!DDS_SubscriberSeq_ensure_length(&subs, 0, MAX_CONTAINED_ENTITIES))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "ensure SubscriberSeq's length");
            goto done;
        }

        if (DDS_RETCODE_OK != 
                DDS_DomainParticipant_get_subscribers(
                        native_entity->participant, &subs))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "get DomainParticipant's subscribers");
            goto done;
        }

        seq_len = DDS_SubscriberSeq_get_length(&subs);
        for (i = 0; i < seq_len; i++)
        {
            param.kind = D2S2_RESOURCEKIND_SUBSCRIBER;
            param.native_entity.subscriber =
                *DDS_SubscriberSeq_get_reference(&subs, i);
                
            if (DDS_RETCODE_OK !=
                    NDDSA_Agent_find_contained_resource_and_deleteEA(
                        self, &param))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    "NDDSA_Agent_find_contained_resource_and_deleteEA");
                goto done;
            }
        }

        /* Iterate over the DomainParticipant's publishers and delete any
           resource that have might be created for them */
        if (!DDS_PublisherSeq_ensure_length(&pubs, 0, MAX_CONTAINED_ENTITIES))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "ensure PublisherSeq's length");
            goto done;
        }

        if (DDS_RETCODE_OK != 
                DDS_DomainParticipant_get_publishers(
                        native_entity->participant, &pubs))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "get DomainParticipant's publishers");
            goto done;
        }

        seq_len = DDS_PublisherSeq_get_length(&pubs);
        for (i = 0; i < seq_len; i++)
        {
            param.kind = D2S2_RESOURCEKIND_PUBLISHER;
            param.native_entity.publisher =
                *DDS_PublisherSeq_get_reference(&pubs, i);;
            
            if (DDS_RETCODE_OK !=
                    NDDSA_Agent_find_contained_resource_and_deleteEA(
                        self, &param))
                        // self, session_rec, &param))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    "NDDSA_Agent_find_contained_resource_and_deleteEA");
                goto done;
            }
        }

        if (DDS_RETCODE_OK !=
                DDS_DomainParticipantFactory_delete_participant(
                    self->db.factory, native_entity->participant))
        {
            goto done;
        }
        
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "DELETING a Publishers's contained entities",
            ((resource_id != NULL)?
                resource_id->value.ref : "not a DDS Resource"));
        
        if (!DDS_DataWriterSeq_ensure_length(
                &writers, 0, MAX_CONTAINED_ENTITIES))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "ensure DataWriterSeq's length");
            goto done;
        }

        if (DDS_RETCODE_OK != 
                DDS_Publisher_get_all_datawriters(
                    native_entity->publisher, &writers))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "get Publisher's writers");
            goto done;
        }

        seq_len = DDS_DataWriterSeq_get_length(&writers);

        for (i = 0; i < seq_len; i++)
        {
            param.kind = D2S2_RESOURCEKIND_DATAWRITER;
            param.native_entity.writer =
                *DDS_DataWriterSeq_get_reference(&writers, i);
            
            if (DDS_RETCODE_OK !=
                    NDDSA_Agent_find_contained_resource_and_deleteEA(
                        self, &param))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    "NDDSA_Agent_find_contained_resource_and_deleteEA");
                goto done;
            }
        }

        parent_entity.participant =
            DDS_Publisher_get_participant(native_entity->publisher);

        if (DDS_RETCODE_OK !=
                DDS_DomainParticipant_delete_publisher(
                    parent_entity.participant, native_entity->publisher))
        {
            goto done;
        }
        
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "DELETING a Subscribers's contained entities",
            ((resource_id != NULL)?
                resource_id->value.ref : "not a DDS Resource"));
        
        if (!DDS_DataReaderSeq_ensure_length(
                &readers, 0, MAX_CONTAINED_ENTITIES))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "ensure DataReaderSeq's length");
            goto done;
        }

        if (DDS_RETCODE_OK != 
                DDS_Subscriber_get_all_datareaders(
                    native_entity->subscriber, &readers))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "get Subscriber's readers");
            goto done;
        }

        seq_len = DDS_DataReaderSeq_get_length(&readers);

        for (i = 0; i < seq_len; i++)
        {
            param.kind = D2S2_RESOURCEKIND_DATAREADER;
            param.native_entity.reader =
                *DDS_DataReaderSeq_get_reference(&readers, i);
            
            if (DDS_RETCODE_OK !=
                    NDDSA_Agent_find_contained_resource_and_deleteEA(
                        self, &param))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    "NDDSA_Agent_find_contained_resource_and_deleteEA");
                goto done;
            }
        }

        parent_entity.participant =
            DDS_Subscriber_get_participant(native_entity->subscriber);

        if (DDS_RETCODE_OK !=
                DDS_DomainParticipant_delete_subscriber(
                    parent_entity.participant, native_entity->subscriber))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "DELETING a DataReader's topic",
            ((resource_id != NULL)?
                resource_id->value.ref : "not a DDS Resource"));
        
        topic_desc = DDS_DataReader_get_topicdescription(native_entity->reader);
        if (topic_desc == NULL)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "get DataReader topic");
            goto done;
        }
        param.kind = D2S2_RESOURCEKIND_TOPIC;
        param.native_entity.topic = DDS_Topic_narrow(topic_desc);

        /* for DataReaders we must deleted them before we
            recur onto the topic */
        parent_entity.subscriber =
            DDS_DataReader_get_subscriber(native_entity->reader);
        
        if (DDS_RETCODE_OK !=
                DDS_Subscriber_delete_datareader(
                    parent_entity.subscriber, native_entity->reader))
        {
            goto done;
        }

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_delete_native_resource_entityEA(
                    self,
                    NULL,
                    param.kind,
                    &param.native_entity))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_delete_native_resource_entityEA");
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "DELETING a DataWriter's topic",
            ((resource_id != NULL)?
                resource_id->value.ref : "not a DDS Resource"));
        
        param.kind = D2S2_RESOURCEKIND_TOPIC;
        param.native_entity.topic =
            DDS_DataWriter_get_topic(native_entity->writer);
        
        if (param.native_entity.topic == NULL)
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "get DataWriter topic");
            goto done;
        }

        /* for DataWriters we must deleted them before we
            recur onto the topic */
        parent_entity.publisher =
            DDS_DataWriter_get_publisher(native_entity->writer);

        if (DDS_RETCODE_OK !=
                DDS_Publisher_delete_datawriter(
                    parent_entity.publisher, native_entity->writer))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "DDS_Publisher_delete_datawriter");
            goto done;
        }

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_delete_native_resource_entityEA(
                    self,
                    NULL,
                    param.kind,
                    &param.native_entity))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_delete_native_resource_entityEA");
            goto done;
        }

        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        /* for DataReaders we must deleted them before we
            recur onto the topic */
        parent_entity.participant =
            DDS_TopicDescription_get_participant(
                DDS_Topic_as_topicdescription(native_entity->topic));

        if (DDS_RETCODE_OK !=
                DDS_DomainParticipant_delete_topic(
                    parent_entity.participant, native_entity->topic))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "DDS_DomainParticipant_delete_topic");
        }
        break;
    }
    default:
        break;
    }

    if (resource_id != NULL)
    {
        NDDSA_ResourceFactory_unload_resource_xml(
                &self->res_factory, resource_kind, resource_id);
    }
    
    retcode = DDS_RETCODE_OK;
    
done:
    DDS_SubscriberSeq_finalize(&subs);
    DDS_PublisherSeq_finalize(&pubs);
    DDS_DataWriterSeq_finalize(&writers);
    DDS_DataReaderSeq_finalize(&readers);

    D2S2Log_fn_exit()
    return retcode;
}


RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_native_resourceEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceId *const resource_id)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_delete_native_resourceEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTIBool native_entity_exists = RTI_FALSE;
    NDDSA_ResourceRecord *resource_rec = NULL;
    NDDSA_EntityResource native_entity = NDDSA_ENTITYRESOURCE_INITIALIZER;
    D2S2_ResourceKind resource_kind = D2S2_RESOURCEKIND_UNKNOWN;
    
    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_lookup_resourceEA(
            &self->db, resource_id, &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec == NULL)
    {
        /* Resource might have already been deleted */
        retcode = DDS_RETCODE_OK;
        goto done;
    }
    if (resource_rec->resource.native_deleted)
    {
        /* resource record was already disposed */
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    resource_rec->resource.native_deleted = RTI_TRUE;

    resource_kind = resource_rec->resource.base.kind;

    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    case D2S2_RESOURCEKIND_TOPIC:
    case D2S2_RESOURCEKIND_PUBLISHER:
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    case D2S2_RESOURCEKIND_DATAREADER:
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        break;
    }
    case D2S2_RESOURCEKIND_DOMAIN:
    case D2S2_RESOURCEKIND_QOSPROFILE:
    case D2S2_RESOURCEKIND_TYPE:
    case D2S2_RESOURCEKIND_APPLICATION:
    {

        /* There's nothing to do for this type of resources but unloading
           an XML element */
        NDDSA_ResourceFactory_unload_resource_xml(
            &self->res_factory,
            resource_kind,
            &resource_rec->resource.base.id);
        
        NDDSA_Resource_finalize(&resource_rec->resource);

        retcode = DDS_RETCODE_OK;
        goto done;
    }
    default:
    {
        goto done;
    }
    }

    if (!NDDSA_ResourceFactory_lookup_entity_resource(
                &self->res_factory,
                resource_rec->resource.base.kind,
                &resource_rec->resource.base.id,
                &native_entity_exists,
                &native_entity))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_ss,
            "look up DDS entity resource: ",
            resource_rec->resource.base.id.value.ref);
        goto done;
    }

    if (!native_entity_exists)
    {
        /* Native entity doesn't exist anymore, so nothing to do,
           but not an error */
        D2S2Log_warn(
            method_name,
            &RTI_LOG_ANY_ss,
            "native resource not found: ",
            resource_rec->resource.base.id.value.ref);
    }

    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        /* We must delete all query conditions that might have been created
           by an associated read */
        NDDSA_Reader *const reader_data =
            (NDDSA_Reader*)resource_rec->resource.user_data;
        
        NDDSA_ReaderRequest *read_req = NULL;
        
        read_req = (NDDSA_ReaderRequest*)
                        REDAInlineList_getFirst(&reader_data->requests);
        while (read_req != NULL)
        {
            if (!NDDSA_Reader_request_complete(reader_data, read_req->read))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    "delete reader request");
                goto done;
            }
            read_req = (NDDSA_ReaderRequest*)
                        REDAInlineList_getFirst(&reader_data->requests);
        }
        if (!NDDSA_Reader_finalize(reader_data))
        {
            goto done;
        }
        REDAFastBufferPool_returnBuffer(self->pool_readers, reader_data);
        
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        NDDSA_Writer *writer_data =
            (NDDSA_Writer*)resource_rec->resource.user_data;
        NDDSA_Writer_finalize(writer_data);
        REDAFastBufferPool_returnBuffer(self->pool_writers, writer_data);
        break;
    }
    default:
        break;
    }
    resource_rec->resource.user_data = NULL;

    NDDSA_Resource_finalize(&resource_rec->resource);

    /* Release resource record since we might need to look up others */
    if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
        resource_rec = NULL;
        goto done;
    }
    resource_rec = NULL;

    if (native_entity_exists)
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_delete_native_resource_entityEA(
                    self,
                    resource_id,
                    resource_kind,
                    &native_entity))
        {
            goto done;
        }
    }
    
    retcode = DDS_RETCODE_OK;
    
done:

    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resourceEA(&self->db, resource_rec))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }

    D2S2Log_fn_exit()
    return retcode;
}



RTI_PRIVATE
RTIBool
NDDSA_Agent_read_from_reader_ddsEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *session_rec,
    NDDSA_AttachedResource *const attached,
    struct DDS_DynamicDataSeq *const samples_seq,
    struct DDS_SampleInfoSeq *const infos_seq,
    RTIBool *const has_loan_out,
    DDS_UnsignedLong *const samples_len_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_read_from_reader_ddsEA)
    RTIBool retcode = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    NDDSA_ResourceRecord *resource_rec = NULL;
    NDDSA_Reader *reader_data = NULL;
    NDDSA_ReaderRequest *req = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(session_rec);

    *has_loan_out = RTI_FALSE;
    *samples_len_out = 0;

    /* lock session and read from DDS Data Reader */
    if (!NDDSA_AgentDb_lookup_resource(
            &self->db, &attached->base.resource, &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "UNKNOWN resource");
        goto done;
    }
    if (resource_rec->resource.native_deleted)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "native resource ALREADY DELETED");
        goto done;
    }

    reader_data = (NDDSA_Reader*)resource_rec->resource.user_data;

    NDDSA_Reader_find_request_by_read(
        reader_data, attached->read_req, &req);

    if (req == NULL)
    {
        goto done;
    }
    
    if (req->query_condition != NULL)
    {
        rc = DDS_DynamicDataReader_read_w_condition(
                reader_data->dyn_reader,
                samples_seq,
                infos_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_QueryCondition_as_readcondition(
                    req->query_condition));
    }
    else
    {
        rc = DDS_DynamicDataReader_read(
                reader_data->dyn_reader,
                samples_seq,
                infos_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ANY_INSTANCE_STATE);
    }
    if (rc != DDS_RETCODE_OK)
    {
        if (rc != DDS_RETCODE_NO_DATA)
        {
            goto done;
        }
    }
    else
    {
        *has_loan_out = RTI_TRUE;
        *samples_len_out = DDS_DynamicDataSeq_get_length(samples_seq);
    }

    retcode = RTI_TRUE;
    
done:
    
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = RTI_FALSE;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_return_loan_to_reader_ddsEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *session_rec,
    NDDSA_AttachedResource *const attached,
    struct DDS_DynamicDataSeq *const samples_seq,
    struct DDS_SampleInfoSeq *const infos_seq,
    const RTIBool updated)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_return_loan_to_reader_ddsEA)
    RTIBool retcode = RTI_FALSE;
    NDDSA_ResourceRecord *resource_rec = NULL;
    NDDSA_Reader *reader_data = NULL;
    NDDSA_ReaderRequest *req = NULL;

    D2S2Log_fn_entry()

    /* lock session and read from DDS Data Reader */
    if (!NDDSA_AgentDb_lookup_resource(
            &self->db, &attached->base.resource, &resource_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
        goto done;
    }
    if (resource_rec == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "UNKNOWN resource");
        goto done;
    }
    if (resource_rec->resource.native_deleted)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "native resource ALREADY DELETED");
        goto done;
    }

    reader_data = (NDDSA_Reader*)resource_rec->resource.user_data;

    NDDSA_Reader_find_request_by_read(
        reader_data, attached->read_req, &req);

    if (req == NULL)
    {
        goto done;
    }

    if (DDS_RETCODE_OK != 
            DDS_DynamicDataReader_return_loan(
                reader_data->dyn_reader, samples_seq, infos_seq))
    {
        goto done;
    }
    
    if (updated)
    {
        if (!NDDSA_Reader_requests_updated(
                reader_data, session_rec, attached, resource_rec))
        {
            /* TODO log */
            goto done;
        }
    }

    retcode = RTI_TRUE;
    
done:
    
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = RTI_FALSE;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_read_from_readerEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *session_rec,
    NDDSA_AttachedResource *const attached)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_read_from_readerEA)
    RTIBool retcode = RTI_FALSE,
            try_again = RTI_FALSE,
            ignored = RTI_FALSE,
            updated = RTI_FALSE,
            finite_request = RTI_FALSE,
            read_complete = RTI_FALSE,
            has_loan = RTI_FALSE;
    struct DDS_DynamicDataSeq samples_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq infos_seq = DDS_SEQUENCE_INITIALIZER;
    struct RTINtpTime min_delay = RTI_NTP_TIME_ZERO,
                      ts_now = RTI_NTP_TIME_ZERO;
    DDS_UnsignedLong samples_len = 0,
                     i = 0;

    D2S2Log_fn_entry()

    /* PRECONDITION
        (!NDDSA_Read_check_status(
            attached->read_req, NDDSA_READSTATUS_DATA_AVAILABLE)) */

    /* Take a timestamp of the current time which we'll use to see if we
       need to delay reading further */
    if (!self->clock->getTime(self->clock, &ts_now))
    {
        goto done;
    }

    NDDSA_Read_is_complete(
        attached->read_req, &ts_now, &finite_request, &read_complete);
    
    if (finite_request && read_complete)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    if (!NDDSA_Agent_read_from_reader_ddsEA(
            self,
            session_rec,
            attached,
            &samples_seq,
            &infos_seq,
            &has_loan,
            &samples_len))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "read from DDS reader");
        goto done;
    }

    for (i = 0; i < samples_len && (!finite_request || !read_complete); i++)
    {
        DDS_DynamicData *sample =
            DDS_DynamicDataSeq_get_reference(&samples_seq, i);
        struct DDS_SampleInfo *info =
            DDS_SampleInfoSeq_get_reference(&infos_seq, i);
        
        if (info->valid_data)
        {
            if (!NDDSA_Read_process_sampleEA(
                    self,
                    session_rec,
                    attached,
                    info,
                    sample,
                    &ts_now,
                    &ignored,
                    &try_again,
                    &min_delay))
            {
                goto done;
            }
            if (ignored)
            {
                continue;
            }
            if (try_again)
            {
                retcode = RTI_TRUE;
                goto done;
            }
            updated = RTI_TRUE;
        }
    }

    if (finite_request)
    {
        if (!self->clock->getTime(self->clock, &ts_now))
        {
            goto done;

        }
        NDDSA_Read_is_complete(
            attached->read_req, &ts_now, &finite_request, &read_complete);
    }

    retcode = RTI_TRUE;
    
done:
    if (!try_again)
    {
        NDDSA_Read_clear_status(
            attached->read_req, NDDSA_READSTATUS_DATA_AVAILABLE);
    }
    else
    {
        attached->read_req->delayed = RTI_TRUE;

        if (!NDDSA_Agent_post_event(
                self,
                &attached->read_req->event_listener,
                &attached->read_req->event_listener_storage,
                sizeof(NDDSA_Read*),
                &min_delay))
        {
            retcode = RTI_FALSE;
        }
    }
    if (has_loan)
    {
        if (!NDDSA_Agent_return_loan_to_reader_ddsEA(
                self,
                session_rec,
                attached,
                &samples_seq,
                &infos_seq,
                updated))
        {
            /* TODO log */
            retcode = RTI_FALSE;
        }
    }
    if (finite_request && read_complete)
    {
        NDDSA_Read_set_status(
            attached->read_req, NDDSA_READSTATUS_COMPLETE);
    }
    
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_on_data_on_reader(
    const struct RTIEventGeneratorListener *me,
    struct RTINtpTime *newTime, struct RTINtpTime *newSnooze,
    const struct RTINtpTime *now, const struct RTINtpTime *time,
    const struct RTINtpTime *snooze,
    const struct RTIEventGeneratorListenerStorage *listenerStorage,
    struct REDAWorker *worker)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_data_on_reader)
    NDDSA_Read *const read = (NDDSA_Read*)listenerStorage->field[0];
    RTIBool cancelled = (listenerStorage->field[1] == NULL),
            complete = RTI_FALSE;
    NDDSA_Agent *self = NULL;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    NDDSA_AttachedResource *attached = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(now);
    UNUSED_ARG(time);
    UNUSED_ARG(snooze);
    UNUSED_ARG(worker);
    UNUSED_ARG(newSnooze);
    UNUSED_ARG(newTime);
    UNUSED_ARG(me);

    if (cancelled)
    {
        RTIOsapiSemaphore_give(read->sem_event);
        return RTI_FALSE;
    }
    self = read->agent;

    AGENT_LOOKUP_SESSION_OR_DONE(
        self, &read->session_key, &session_rec, /* on_unknown */);
    
    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, read->reader_id, &attached);
    
    if (attached == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "reader not attached to session");
        goto done;
    }

    if (attached->base.kind != D2S2_RESOURCEKIND_DATAREADER)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_ss,
            D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
            "reader");
        goto done;
    }

    if (attached->read_req == NULL)
    {
        /* read cancelled */
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "data notified without a read request");
        goto done;
    }

    if (attached->read_req == NULL ||
        NDDSA_Read_check_status(
            attached->read_req, NDDSA_READSTATUS_COMPLETE))
    {
        /* read cancelled */
        complete = RTI_TRUE;
        D2S2Log_warn(
            method_name,
            &RTI_LOG_ANY_s,
            "read request already completed");
        goto done;
    }

    if (!read->delayed)
    {
        D2S2Log_periodic(
            method_name,
            &RTI_LOG_ANY_s,
            "MARK subscription as DATA AVAILABLE");
        NDDSA_Read_set_status(
            attached->read_req, NDDSA_READSTATUS_DATA_AVAILABLE);
    }
    read->delayed = RTI_FALSE;

    if (!NDDSA_Agent_read_from_readerEA(self, session_rec, attached))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "read data for subscription");
        goto done;
    }

    complete = NDDSA_Read_check_status(
                    attached->read_req, NDDSA_READSTATUS_COMPLETE);
    
done:
    if (complete)
    {
        if (DDS_RETCODE_OK !=
                DDS_GuardCondition_set_trigger_value(
                    self->cond_cleanup_reads, DDS_BOOLEAN_TRUE))
        {
            goto done;
        }
    }

    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_SESSION_FAILED);
        }
    }
    D2S2Log_fn_exit()
    
    return RTI_FALSE;
}


RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_buffer_to_dyndata(
    NDDSA_Writer *writer_data,
    const D2S2_DataRepresentation *const data,
    DDS_DynamicData *const sample_out)
{
    /* TODO remove this ugliness */
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong data_len = 0;

    UNUSED_ARG(writer_data);

    data_len =
        data->value.xcdr.buffer.data_len + sizeof(DDS_Long);
    
    if (data_len > writer_data->data_buffer_max)
    {

        RTIOsapiHeap_reallocateBufferAligned(
            &writer_data->data_buffer,
            data_len,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

        if (writer_data->data_buffer == NULL)
        {
            writer_data->data_buffer_max = 0;
            writer_data->data_buffer_len = 0;
            goto done;
        }

        writer_data->data_buffer_max = data_len;
    }

    writer_data->data_buffer_len = data_len;


    memcpy(
        writer_data->data_buffer + sizeof(DDS_Long),
        data->value.xcdr.buffer.data,
        data->value.xcdr.buffer.data_len);

    RTIOsapiMemory_zero(writer_data->data_buffer, sizeof(DDS_Long));

    switch (data->value.xcdr.encoding)
    {
    case D2S2_XCDRENCODINGKIND_1:
    case D2S2_XCDRENCODINGKIND_2:
    {
        writer_data->data_buffer[0] = 0x00;

        break;
    }
    case D2S2_XCDRENCODINGKIND_1_PL:
    case D2S2_XCDRENCODINGKIND_2_PL:
    {
        writer_data->data_buffer[0] = 0x01;
        break;
    }
    default:
    {
        goto done;
    }
    }

    if (data->value.xcdr.little_endian)
    {
        switch (data->value.xcdr.encoding)
        {
        case D2S2_XCDRENCODINGKIND_1:
        case D2S2_XCDRENCODINGKIND_1_PL:
        {
            writer_data->data_buffer[1] = 0x01;
            break;
        }
        case D2S2_XCDRENCODINGKIND_2:
        case D2S2_XCDRENCODINGKIND_2_PL:
        {
            writer_data->data_buffer[1] = 0x11;
            break;
        }
        default:
        {
            goto done;
        }
        }
    }
    else
    {
        switch (data->value.xcdr.encoding)
        {
        case D2S2_XCDRENCODINGKIND_1:
        case D2S2_XCDRENCODINGKIND_1_PL:
        {
            writer_data->data_buffer[1] = 0x00;
            break;
        }
        case D2S2_XCDRENCODINGKIND_2:
        case D2S2_XCDRENCODINGKIND_2_PL:
        {
            writer_data->data_buffer[1] = 0x10;
            break;
        }
        default:
        {
            goto done;
        }
        }
    }

    // DDS_DynamicData_enable_legacy_impl();


    // if (DDS_RETCODE_OK !=
    //         DDS_DynamicData_set_and_lock_buffer(
    //                 sample_out,
    //                 (DDS_Octet*)
    //                 data->value.xcdr.buffer.data,
    //                 data->value.xcdr.buffer.data_len))
    // {
    //     goto done;
    // }
#if 1
    if (DDS_RETCODE_OK != 
            DDS_DynamicData_from_cdr_buffer(
                sample_out,
                writer_data->data_buffer,
                writer_data->data_buffer_len))
    {
        goto done;
    }
#else
    if (DDS_RETCODE_OK !=
            DDS_DynamicData_set_and_lock_buffer(
                    sample_out,
                    (DDS_Octet*)
                    writer_data->data_buffer,
                    writer_data->data_buffer_len))
    {
        goto done;
    }
#endif
#if 0
    printf("****************************************\n");
    printf("CONVERTED SAMPLE:\n");    
    printf("recvd(%u)(enc=%X, le=%d): { ",
        data->value.xcdr.buffer.data_len,
        data->value.xcdr.encoding,
        data->value.xcdr.little_endian);
    for (size_t i = 0; i < data->value.xcdr.buffer.data_len; i++)
    {
        printf("%02X ",data->value.xcdr.buffer.data[i]);
    }
    printf("}\n");
    // printf("buffer(%u): { ",writer_data->data_buffer_len);
    // for (size_t i = 0; i < writer_data->data_buffer_len; i++)
    // {
    //     printf("%02X ", (unsigned char)writer_data->data_buffer[i]);
    // }
    // printf("}\n");
    printf("sample: ");
    DDS_DynamicData_print(sample_out, stdout, 0);
    printf("\n");
    printf("****************************************\n");
#endif

    retcode = DDS_RETCODE_OK;
    
done:

    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_insert_resource_recordEA(
    NDDSA_Agent *const self,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceRepresentation *const resource_repr,
    const D2S2_ResourceId *const parent_res_id,
    const D2S2_ResourceProperties *const properties,
    NDDSA_ResourceRecord **const resource_rec_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_insert_resource_recordEA)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct NDDSA_CreatedResourceLogSeq created = DDS_SEQUENCE_INITIALIZER;
    DDS_UnsignedLong created_count = 0;
    NDDSA_CreatedResourceLog *created_id = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;
    NDDSA_GenericResource generic = NDDSA_GENERICRESOURCE_INITIALIZER;
    NDDSA_EntityResource native_entity = NDDSA_ENTITYRESOURCE_INITIALIZER;
    NDDSA_ResourceNative native_resource = NDDSA_RESOURCENATIVE_INITIALIZER;
    NDDSA_Reader *reader_data = NULL;
    NDDSA_Writer *writer_data = NULL;
    struct DDS_DataReaderQos dr_qos = DDS_DataReaderQos_INITIALIZER;
    RTIBool native_entity_exists = RTI_FALSE,
            native_initd = RTI_FALSE,
            resource_rec_initd = RTI_FALSE,
            already_exists = RTI_FALSE;

    D2S2Log_fn_entry()

    *resource_rec_out = NULL;

    /* Try to create the native resource */
    if (!NDDSA_ResourceFactory_create_resource_native(
            &self->res_factory,
            resource_kind,
            resource_repr,
            parent_res_id,
            properties,
            &created))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_AGENT_CREATE_NATIVE_RESOURCE_FAILED);
        goto done;
    }

    created_count = NDDSA_CreatedResourceLogSeq_get_length(&created);
    if (created_count == 0)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "unexpected no entities created");
        goto done;
    }
    created_id = NDDSA_CreatedResourceLogSeq_get_reference(&created, 0);
    if (created_id->kind != resource_kind)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "unexpected kind of created resource");
        goto done;
    }

    if (!NDDSA_AgentDb_insert_resourceEA(
            &self->db, &created_id->id, &resource_rec, &already_exists))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_INSERT_RESOURCE_FAILED);
        goto done;
    }

    if (already_exists &&
        resource_repr->fmt != D2S2_RESOURCEREPRESENTATIONFORMAT_REF)
    {
        /* "By reference" is the only representation that allows us to 
           reuse a database record */
        goto done;
    }

    if (!already_exists)
    {
        /* create "native" resource to store in record */
        switch (resource_kind)
        {
        case D2S2_RESOURCEKIND_APPLICATION:
        case D2S2_RESOURCEKIND_DOMAIN:
        case D2S2_RESOURCEKIND_QOSPROFILE:
        case D2S2_RESOURCEKIND_TYPE:
        {
            /* These resources don't have an actual DDS entity associated to them,
            so we represent them by caching the provided representation */

            /* shallow copy since NDDSA_ResourceNative will copy to own memory */
            generic.id = created_id->id;

            if (!NDDSA_ResourceNative_initialize_generic(
                    &native_resource, &generic, resource_repr))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_s,
                    "unexpected failure to initialize generic resource");
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
        case D2S2_RESOURCEKIND_TOPIC:
        case D2S2_RESOURCEKIND_PUBLISHER:
        case D2S2_RESOURCEKIND_SUBSCRIBER:
        case D2S2_RESOURCEKIND_DATAWRITER:
        case D2S2_RESOURCEKIND_DATAREADER:
        {
            /* Lookup the associated DDS entity */
            if (!NDDSA_ResourceFactory_lookup_entity_resource(
                    &self->res_factory,
                    resource_kind,
                    &created_id->id,
                    &native_entity_exists,
                    &native_entity))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_AGENT_LOOKUP_NATIVE_RESOURCE_FAILED);
                goto done;
            }
            
            if (!native_entity_exists)
            {
                /* should never get here, since we just created the entity */
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_s,
                    "unexpected native entity not found");
                goto done;
            }

            if (!NDDSA_ResourceNative_initialize_entity(
                    &native_resource, &native_entity, resource_repr))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_s,
                    "unexpected failure to initialize native resource");
                goto done;
            }

            break;
        }
        default:
        {
            /* should never get here */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_s,
                "unexpected type of resource");
            goto done;
        }
        }
        native_initd = RTI_TRUE;

        if (!NDDSA_Resource_initialize(
                &resource_rec->resource,
                &created_id->id,
                resource_kind,
                &native_resource))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_INIT_FAILURE_s,
                D2S2_LOG_MSG_AGENT_INITIALIZE_RESOURCE_FAILED);
            goto done;
        }
        resource_rec_initd = RTI_TRUE;

        if (resource_kind == D2S2_RESOURCEKIND_DATAREADER)
        {
            reader_data =
                (NDDSA_Reader*) REDAFastBufferPool_getBuffer(self->pool_readers);
            if (reader_data == NULL)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_ALLOC_BUFFER_FAILED,
                    "readers state");
                goto done;
            }
            if (!NDDSA_Reader_initialize(reader_data, &resource_rec->resource))
            {
                /* TODO finalize reader_data */
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_INIT_FAILURE_s,
                    D2S2_LOG_MSG_AGENT_INITIALIZE_READER_STATE_FAILED);
                REDAFastBufferPool_returnBuffer(self->pool_readers, reader_data);
                goto done;
            }

            reader_data->agent = self;
            
            resource_rec->resource.user_data = reader_data;

            if (DDS_RETCODE_OK != 
                    DDS_DataReader_get_qos(
                        DDS_DynamicDataReader_as_datareader(
                            reader_data->dyn_reader),
                        &dr_qos))
            {
                /* TODO log */
                /* TODO finalize reader_data */
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_s,
                    "unexpected failure to get DataReader QoS");
                REDAFastBufferPool_returnBuffer(
                    self->pool_readers, reader_data);
                goto done;
            }

            reader_data->dismiss_samples =
                (dr_qos.history.kind == DDS_KEEP_ALL_HISTORY_QOS);

        }
        else if (resource_kind == D2S2_RESOURCEKIND_DATAWRITER)
        {
            writer_data =
                (NDDSA_Writer*) REDAFastBufferPool_getBuffer(self->pool_writers);
            if (writer_data == NULL)
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_ss,
                    D2S2_LOG_MSG_ALLOC_BUFFER_FAILED,
                    "writers state");
                goto done;
            }
            if (!NDDSA_Writer_initialize(writer_data, &resource_rec->resource))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_INIT_FAILURE_s,
                    D2S2_LOG_MSG_AGENT_INITIALIZE_WRITER_STATE_FAILED);
                REDAFastBufferPool_returnBuffer(self->pool_writers, writer_data);
                goto done;
            }
            writer_data->agent = self;
            writer_data->user_data = NULL;
            resource_rec->resource.user_data = writer_data;
        }

#if 0
        if (resource_repr->fmt == D2S2_RESOURCEREPRESENTATIONFORMAT_REF)
        {
            /* If resource was created by reference, mark it as "loaded"
              (like the ones created from XML) so that the native resource
              won't be deleted when the resource is deleted */
            resource_rec->resource.loaded = RTI_TRUE;
        }
#endif
    }


    if (DDS_RETCODE_OK !=
            DDS_GuardCondition_set_trigger_value(
                self->cond_cleanup_resources, DDS_BOOLEAN_TRUE))
    {
        goto done;
    }

    if (!(resource_rec->resource.pending_delete || 
            resource_rec->resource.native_deleted))
    {
        *resource_rec_out = resource_rec;
    }

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        if (resource_rec != NULL)
        {
            if (!native_initd)
            {
                /* We just inserted the record in the db, so we should
                    just remove it */
                D2S2Log_freeForm(RTI_LOG_BIT_WARN)(
                    "NOT IMPLEMENTED: remove resource record on FAILURE\n");
            }
            else if (!resource_rec_initd)
            {
                /* failed at NDDSA_Resource_initialize */
                D2S2Log_freeForm(RTI_LOG_BIT_WARN)(
                    "NOT IMPLEMENTED: finalize and delete resource record on FAILURE\n");
            }
            else
            {
                /* TODO
                    we should detach the resource from all sessions, 
                    delete all associated resource, and delete the resource
                    itself */
                D2S2Log_freeForm(RTI_LOG_BIT_WARN)(
                    "NOT IMPLEMENTED: detach, finalize and delete resource record on FAILURE\n");
            }
        }
    }
    NDDSA_CreatedResourceLogSeq_finalize(&created);
    DDS_DataReaderQos_finalize(&dr_qos);
    D2S2Log_fn_exit();
    return retcode;
}


RTI_PRIVATE
void
NDDSA_Agent_on_cleanup_sessions(
    void *handler_data,
    DDS_Condition *condition)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_cleanup_sessions)
    NDDSA_Agent *const self = (NDDSA_Agent*)handler_data;
    RTIBool locked_sessions = RTI_FALSE;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    D2S2_AgentServerInterface *src = NULL;
    D2S2_ClientSessionKey session_key = D2S2_CLIENTSESSIONKEY_INITIALIZER;
    void *session_data = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(condition);

    D2S2Log_local(
        method_name,
        &RTI_LOG_ANY_s,
        "CLEANING UP disposed sessions...");

    if (!NDDSA_AgentDb_lock_sessions(
            &self->db,
            RTI_TRUE /* start_cursor */,
            RTI_TRUE /* goto_top */))
    {
        goto done;
    }
    locked_sessions = RTI_TRUE;

    if (DDS_RETCODE_OK !=
            DDS_GuardCondition_set_trigger_value(
                self->cond_cleanup_sessions, DDS_BOOLEAN_FALSE))
    {
        goto done;
    }

    if (!NDDSA_AgentDb_find_next_session(
            &self->db, 
            NDDSA_Agent_find_next_disposed_session,
            NULL,
            NULL,
            &session_rec))
    {
        goto done;
    }

    while (session_rec != NULL)
    {
        src = session_rec->session.intf;
        session_key = session_rec->session.base.key;
        session_data = session_rec->session.user_data;

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_finalize_sessionEA(
                    self, src, session_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_finalize_sessionEA");
            goto done;
        }

        if (session_rec->session.timedout)
        {
            RTIOsapiSemaphore_delete(session_rec->session.sem_cleanup);
            session_rec->session.sem_cleanup = NULL;

            session_rec = NULL;
            if (!NDDSA_AgentDb_release_sessionEA(&self->db, &session_key))
            {
                goto done;
            }

            if (!NDDSA_AgentDb_delete_sessionEA(&self->db, &session_key))
            {
                goto done;
            }

            D2S2_AgentServerInterface_on_session_closed(
                src,
                &self->base,
                &session_key,
                session_data);
        }
        else
        {
            // printf("*****************************************\n");
            // printf("     GIVE sem_cleanup %p\n", session_rec->session.sem_cleanup);
            // printf("*****************************************\n");
            RTIOsapiSemaphore_give(session_rec->session.sem_cleanup);
            if (!NDDSA_AgentDb_release_sessionEA(&self->db, &session_key))
            {
                session_rec = NULL;
                goto done;
            }
            session_rec = NULL;
        }
        
        if (!NDDSA_AgentDb_find_next_session(
                &self->db, 
                NDDSA_Agent_find_next_disposed_session,
                NULL,
                NULL,
                &session_rec))
        {
            goto done;
        }
    }

    D2S2Log_local(
        method_name,
        &RTI_LOG_ANY_s,
        "CLEAN UP of disposed sessions DONE");


done:
    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_sessionEA(
                &self->db, &session_rec->session.base.key))
        {
            /* TODO log */
        }
    }
    if (locked_sessions)
    {
        if (!NDDSA_AgentDb_unlock_sessions(&self->db, RTI_TRUE))
        {
            /* TODO log */
        }
    }
    
    D2S2Log_fn_exit()
    return;
}


RTI_PRIVATE
void
NDDSA_Agent_on_cleanup_reads(
    void *handler_data,
    DDS_Condition *condition)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_cleanup_reads)
    NDDSA_Agent *const self = (NDDSA_Agent*)handler_data;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;
    struct REDAInlineListNode *node = NULL;
    NDDSA_AttachedResource *attached = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(condition);

    D2S2Log_local(
        method_name,
        &RTI_LOG_ANY_s,
        "CLEANING UP completed reads...");

    if (DDS_RETCODE_OK !=
            DDS_GuardCondition_set_trigger_value(
                self->cond_cleanup_reads, DDS_BOOLEAN_FALSE))
    {
        goto done;
    }

    if (!NDDSA_AgentDb_iterate_sessions(&self->db))
    {
        goto done;
    }

    if (!NDDSA_AgentDb_find_next_session(
            &self->db, 
            NDDSA_Agent_find_next_session_w_completed_read,
            NULL,
            NULL,
            &session_rec))
    {
        goto done;
    }

    while (session_rec != NULL)
    {
        node = REDAInlineList_getFirst(&session_rec->session.resources);
        while (node != NULL)
        {
            attached = NDDSA_AttachedResource_from_node(node);
            node = REDAInlineListNode_getNext(node);

            if (attached->read_req != NULL &&
                NDDSA_Read_check_status(
                    attached->read_req, NDDSA_READSTATUS_COMPLETE))
            {
                
                if (NDDSA_Read_has_pending_samples(attached->read_req))
                {
                    /* This call will end up calling cancel_read/on_read_complete */
                    if (DDS_RETCODE_OK !=
                            D2S2_AgentServerInterface_on_release_read_samples(
                                session_rec->session.intf,
                                &self->base,
                                &session_rec->session.base,
                                session_rec->session.user_data,
                                attached->base.id,
                                attached->user_data))
                    {
                        D2S2Log_exception(
                            method_name,
                            &RTI_LOG_ANY_FAILURE_s,
                            "release cached samples");
                    }
                    continue;
                }

                if (!NDDSA_AgentDb_lookup_resource(
                        &self->db, &attached->base.resource, &resource_rec))
                {
                    D2S2Log_exception(
                        method_name,
                        &RTI_LOG_ANY_FAILURE_s,
                        D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED);
                    goto done;
                }

                if (resource_rec == NULL)
                {
                    D2S2Log_exception(
                        method_name,
                        &RTI_LOG_ANY_FAILURE_s,
                        "UNKNOWN resource");
                    goto done;
                }
                if (DDS_RETCODE_OK !=
                        NDDSA_Agent_cancel_readEA(
                            self,
                            session_rec,
                            attached,
                            RTI_FALSE /* locked_resources */,
                            resource_rec,
                            NULL))
                {
                    D2S2Log_exception(
                        method_name,
                        &RTI_LOG_ANY_FAILURE_s,
                        "FAILED to cancel read on close session");
                    goto done;
                }

                if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
                {
                    D2S2Log_exception(
                        method_name,
                        &RTI_LOG_ANY_FAILURE_s,
                        D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
                    resource_rec = NULL;
                    goto done;
                }
                resource_rec = NULL;
            }
        }
        
        if (!NDDSA_AgentDb_find_next_session(
                &self->db, 
                NDDSA_Agent_find_next_session_w_completed_read,
                NULL,
                session_rec,
                &session_rec))
        {
            goto done;
        }
    }

    if (!NDDSA_AgentDb_finish_iterate_sessions(&self->db))
    {
        goto done;
    }

    D2S2Log_local(
        method_name,
        &RTI_LOG_ANY_s,
        "CLEAN UP of disposed reads DONE");


done:
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            /* TODO log */
        }
    }
    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
        {
            /* TODO log */
        }
    }
    
    D2S2Log_fn_exit()
    return;
}

RTI_PRIVATE
void
NDDSA_Agent_on_cleanup_resources(
    void *handler_data,
    DDS_Condition *condition)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_cleanup_resources)
    NDDSA_Agent *const self = (NDDSA_Agent*)handler_data;
    RTIBool locked_sessions = RTI_FALSE,
            delete_resource = RTI_FALSE;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    NDDSA_AttachedResource *attached = NULL;
    struct REDAInlineListNode *node = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;
    D2S2_ResourceId resource_id = D2S2_RESOURCEID_INITIALIZER;
    D2S2_ResourceKind resource_kind = D2S2_RESOURCEKIND_UNKNOWN;
    void *resource_data = NULL;
    D2S2_AttachedResourceId attach_id = D2S2_ATTACHEDRESOURCEID_INVALID;

    D2S2Log_fn_entry()

    UNUSED_ARG(condition);

    D2S2Log_local(
        method_name,
        &RTI_LOG_ANY_s,
        "CLEANING UP disposed resources...");

    if (DDS_RETCODE_OK !=
            DDS_GuardCondition_set_trigger_value(
                self->cond_cleanup_resources, DDS_BOOLEAN_FALSE))
    {
        goto done;
    }

    if (!NDDSA_AgentDb_lock_sessions(
            &self->db,
            RTI_TRUE /* start_cursor */,
            RTI_TRUE /* goto_top */))
    {
        goto done;
    }
    locked_sessions = RTI_TRUE;

    if (!NDDSA_AgentDb_find_next_session(
            &self->db, 
            NULL,
            NULL,
            NULL,
            &session_rec))
    {
        goto done;
    }

    while (session_rec != NULL)
    {
        node = REDAInlineList_getFirst(&session_rec->session.resources);
        while (node != NULL)
        {
            attached = NDDSA_AttachedResource_from_node(node);
            node = REDAInlineListNode_getNext(&attached->node);

            if (!NDDSA_AgentDb_lookup_resource(
                    &self->db, &attached->base.resource, &resource_rec))
            {
                goto done;
            }
            if (resource_rec == NULL)
            {
                goto done;
            }

            delete_resource = resource_rec->resource.native_deleted ||
                (resource_rec->resource.pending_delete &&
                    resource_rec->resource.attached_count == 0);

            if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
            {
                resource_rec = NULL;
                goto done;
            }
            resource_rec = NULL;

            if (delete_resource)
            {
                attach_id = attached->base.id;

                if (DDS_RETCODE_OK !=
                        NDDSA_Agent_detach_resourceEA(
                            self,
                            session_rec->session.intf,
                            session_rec,
                            attached,
                            RTI_TRUE,
                            NULL,
                            &resource_kind,
                            &resource_id,
                            &resource_data))
                {
                    goto done;
                }

                D2S2_AgentServerInterface_on_resource_deleted(
                    session_rec->session.intf,
                    &self->base,
                    &session_rec->session.base,
                    session_rec->session.user_data,
                    resource_kind,
                    &resource_id,
                    attach_id,
                    resource_data,
                    NULL);
            }
        }

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_attach_all_resources_to_sessionEA(
                    self, session_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_attach_all_resources_to_sessionEA");
            goto done;
        }

        if (!NDDSA_AgentDb_find_next_session(
                &self->db, 
                NULL,
                NULL,
                session_rec,
                &session_rec))
        {
            session_rec = NULL;
            goto done;
        }
    }

    D2S2Log_local(
        method_name,
        &RTI_LOG_ANY_s,
        "CLEAN UP of disposed resources DONE");
done:
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resource(&self->db, resource_rec))
        {
            /* TODO log */
        }
        
    }
    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_sessionEA(
                &self->db, &session_rec->session.base.key))
        {
            /* TODO log */
        }
    }
    if (locked_sessions)
    {
        if (!NDDSA_AgentDb_unlock_sessions(&self->db, RTI_TRUE))
        {
            /* TODO log */
        }
    }
    D2S2Log_fn_exit()
    return;
}



#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */