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
#include "clock/clock_monotonic.h"
#include "clock/clock_system.h"


#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

/******************************************************************************
 * Implementation of D2S2_Agent interface
 ******************************************************************************/

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_open_session(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key,
    const D2S2_ClientSessionProperties *const properties,
    void *const session_data);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_close_session(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_create_resource(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceRepresentation *const resource_repr,
    const D2S2_AttachedResourceId parent_id,
    const D2S2_ResourceProperties *const properties,
    void *const request_param);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_resource(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    void *const request_param);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_lookup_resource(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    DDS_Boolean *const resource_exists_out,
    void **const resource_data_out);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_read(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId reader_id,
    const D2S2_ReadSpecification *const read_spec,
    void *const request_param);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_cancel_read(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId reader_id,
    void *const request_param);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_return_loan(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    D2S2_ReceivedData *const data);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_write(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId writer_id,
    const D2S2_DataRepresentation *const data,
    void *const request_param);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_load_resources_from_xml(
    D2S2_Agent *const agent,
    const char *const xml_url,
    const D2S2_ResourceProperties *const properties);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_register_interface(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const intf);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_register_external_service_plugin(
    D2S2_Agent *const self,
    D2S2_ExternalServicePlugin *const service_plugin);

RTI_PRIVATE
void
NDDSA_Agent_delete(D2S2_Agent *const agent);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_start(D2S2_Agent *const agent);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_stop(D2S2_Agent *const agent);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_create_session_event(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    void *const event_listener,
    D2S2_Agent_OnSessionEventCallback on_event,
    D2S2_ClientSessionEvent **const event_out);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_fire_session_event(
    D2S2_Agent *const self,
    D2S2_ClientSessionEvent *const event,
    const struct RTINtpTime *const delay);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_session_event(
    D2S2_Agent *const self,
    D2S2_ClientSessionEvent *const event);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_make_external_service_request(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId svc_res_id,
    const DDS_UnsignedLong svc_flags,
    const D2S2_Buffer * const svc_query,
    const D2S2_Buffer * const svc_data,
    const D2S2_Buffer * const svc_metadata,
    const DDS_Boolean no_reply,
    void *const request_param);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_return_external_service_reply(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    D2S2_ReceivedData *const data);

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_external_service_reply_available(
    D2S2_Agent *const self,
    const D2S2_ResourceId *const svc_res_id,
    const D2S2_ExternalServiceRequestToken req_token);

/******************************************************************************
 *                  Agent Interface Implementation
 ******************************************************************************/

D2S2_Agent*
NDDSA_Agent_create(
    const NDDSA_AgentProperties *const user_properties)
{
    RTIBool rc = RTI_FALSE;
    NDDSA_Agent *res = NULL;
    
    RTIOsapiHeap_allocateStructure(&res, NDDSA_Agent);
    if (res == NULL)
    {
        goto done;
    }

    if (!NDDSA_Agent_initialize(res, user_properties))
    {
        goto done;
    }

    rc = RTI_TRUE;

done:
    if (!rc && res != NULL)
    {
        RTIOsapiHeap_free(res);
        res = NULL;
    }
    return &res->base;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_register_interface(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const intf)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    
    if (intf->agent != NULL)
    {
        /* interface is already registered on another agent */
        goto done;
    }

    intf->agent = &self->base;

    D2S2_AgentServerInterface_on_interface_registered(intf, &self->base);

    REDAInlineList_addNodeToBackEA(&self->interfaces, &intf->node);
    
    retcode = DDS_RETCODE_OK;
    
done:
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_register_external_service_plugin(
    D2S2_Agent *const agent,
    D2S2_ExternalServicePlugin *const service_plugin)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    
    REDAInlineList_addNodeToBackEA(&self->service_plugins, &service_plugin->node);
    
    retcode = DDS_RETCODE_OK;
    
done:
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_open_session(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key,
    const D2S2_ClientSessionProperties *const user_properties,
    void *const session_data)
{
    const char *const method_name = "NDDSA_Agent_open_session";
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    RTIBool new_session = RTI_FALSE;
    D2S2_ClientSessionProperties properties = 
        D2S2_CLIENTSESSIONPROPERTIES_INITIALIZER;
    void *new_session_data = NULL;
    NDDSA_AttachedResource *attached = NULL;
    struct REDAInlineListNode *node = NULL;

    D2S2Log_fn_entry();

    if (user_properties != NULL)
    {
        properties = *user_properties;
    }

    if (!NDDSA_AgentDb_insert_session(
            &self->db, session_key, &new_session, &session_rec))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_INSERT_SESSION_FAILED);
        goto done;
    }
    if (!new_session)
    {
        if (!NDDSA_ClientSession_active(&session_rec->session))
        {
            goto done;
        }
        session_rec->session.resetting = RTI_TRUE;

        if (session_rec->session.intf != src)
        {
            goto done;
        }

        /* Mark session as "resetting" so we can ignore timeouts 
          and simultaneous data, since we need to release the EA to
          cancel the reads. */
        session_rec->session.resetting = RTI_TRUE;

        if (DDS_RETCODE_OK !=
                D2S2_AgentServerInterface_on_session_reset(
                        src,
                        &self->base,
                        &session_rec->session.base,
                        session_rec->session.user_data,
                        session_data,
                        &new_session_data))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_INTF_ON_SESSION_RESET_FAILED);
            goto done;
        }
        
        if (new_session_data != NULL)
        {
            session_rec->session.user_data = new_session_data;
        }

        /* Cancel all existing reads */
        node = REDAInlineList_getFirst(&session_rec->session.resources);
        while (node != NULL)
        {
            attached = (NDDSA_AttachedResource*)
                            NDDSA_AttachedResource_from_node(node);
            node = REDAInlineListNode_getNext(&attached->node);

            if (attached->read_req == NULL)
            {
                continue;
            }

            if (DDS_RETCODE_OK !=
                    NDDSA_Agent_cancel_readI(
                        self,
                        src,
                        session_rec,
                        attached,
                        NULL))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    "FAILED to cancel existing read");
                goto done;
            }
        }
        session_rec->session.resetting = RTI_FALSE;
    }
    else
    {
        session_rec->session.base.key = *session_key;
        session_rec->session.agent = self;
        session_rec->session.intf = src;
        session_rec->session.deleted = RTI_FALSE;
        session_rec->session.props = properties;
        session_rec->session.user_data = session_data;
        session_rec->session.timeout_listener.onEvent = 
            NDDSA_Agent_on_session_timeout;
        session_rec->session.timeout_listener_storage.field[0] = 
            &session_rec->session;
        session_rec->session.timeout_listener_storage.field[1] = 
            &session_rec->session;
        /* TODO check that these semaphores are actually cleaned up */
        session_rec->session.sem_timeout =
            RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
        if (session_rec->session.sem_timeout == NULL)
        {
            goto done;
        }
        session_rec->session.sem_cleanup = 
            RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
        if (session_rec->session.sem_cleanup == NULL)
        {
            goto done;
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


        if (DDS_RETCODE_OK !=
                D2S2_AgentServerInterface_on_session_opened(
                        src,
                        &self->base,
                        &session_rec->session.base,
                        session_rec->session.user_data))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_INTF_ON_SESSION_OPENED_FAILED);
            goto done;
        }
    }

    // if (DDS_RETCODE_OK !=
    //         NDDSA_Agent_attach_all_resources_to_sessionEA(
    //             self, session_rec))
    // {
    //     D2S2Log_exception(
    //         method_name,
    //         &RTI_LOG_ANY_FAILURE_s,
    //         "NDDSA_Agent_attach_all_resources_to_sessionEA");
    //     goto done;
    // }

    retcode = DDS_RETCODE_OK;
    
done:

    if (DDS_RETCODE_OK != retcode)
    {
        if (new_session && session_rec != NULL)
        {
            if (!NDDSA_AgentDb_delete_session(&self->db, session_rec))
            {
                D2S2Log_exception(
                        method_name,
                        &RTI_LOG_ANY_FAILURE_s,
                        D2S2_LOG_MSG_DB_DELETE_SESSION_FAILED);
                retcode = DDS_RETCODE_ERROR;
                session_rec = NULL;
            }
            session_rec = NULL;
        }
    }
    else
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_on_session_activityI(self, session_rec))
        {
            D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_FAILURE_s,
                    D2S2_LOG_MSG_AGENT_MARK_SESSION_ACTIVE_FAILED);
            retcode = DDS_RETCODE_ERROR;
        }
    }

    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
        {
            /* this should never fail */
            retcode = DDS_RETCODE_ERROR;
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_SESSION_FAILED);
        }
    }

    D2S2Log_fn_exit();

    return retcode;
}


RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_close_session(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_close_session)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    RTIBool table_locked = RTI_FALSE;
    void *session_data = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;

    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_lock_sessions(
            &self->db,
            RTI_TRUE /* start_cursor */,
            RTI_FALSE /* goto_top */))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_LOCK_SESSIONS_FAILED);
        goto done;
    }
    table_locked = RTI_TRUE;

    AGENT_LOOKUP_SESSION_EA_OR_DONE(self, session_key, &session_rec);

    if (session_rec->session.intf != src)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "unexpected source agent interface");
        goto done;
    }

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
    
    session_data = session_rec->session.user_data;

    if (!NDDSA_AgentDb_release_sessionEA(&self->db, session_key))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_RELEASE_SESSION_FAILED);
        session_rec = NULL;
        goto done;
    }
    session_rec = NULL;

    if (!NDDSA_AgentDb_delete_sessionEA(&self->db, session_key))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_DELETE_SESSION_FAILED);
        goto done;
    }
    session_rec = NULL;

    table_locked = RTI_FALSE;
    if (!NDDSA_AgentDb_unlock_sessions(&self->db, RTI_TRUE))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_DB_UNLOCK_SESSIONS_FAILED);
        goto done;
    }

    D2S2_AgentServerInterface_on_session_closed(
        src,
        &self->base,
        session_key,
        session_data);
    
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
        }
    }
    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_sessionEA(&self->db, session_key))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_SESSION_FAILED);
            retcode = DDS_RETCODE_OK;
        }
    }
    if (table_locked)
    {
        if (!NDDSA_AgentDb_unlock_sessions(&self->db, RTI_TRUE))
        {
            /* TODO log */
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_UNLOCK_SESSIONS_FAILED);
            retcode = DDS_RETCODE_OK;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_resource(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_delete_resource)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec =
            NDDSA_ClientSessionRecord_from_session(session);;
    NDDSA_AttachedResource *attached = NULL;
    D2S2_ResourceId ref_resource_id = D2S2_RESOURCEID_INITIALIZER;
    void *resource_data = NULL;
    D2S2_ResourceKind resource_kind = D2S2_RESOURCEKIND_UNKNOWN;
    
    D2S2Log_fn_entry()

    /* Find resource attached to session with specified resource_id */
    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, resource_id, &attached);
    
    if (attached == NULL)
    {
        /* No resource currently attached to the session with
           the specified resource_id. We don't consider this an error,
           but since there's nothing else to do, return success
           (session will be released on done) */
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    if (attached->base.kind == D2S2_RESOURCEKIND_DATAREADER &&
        attached->read_req != NULL)
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_cancel_readI(
                    self,
                    src,
                    session_rec,
                    attached,
                    request_param))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "cancel read on resource");
            goto done;
        }
        D2S2_AgentServerInterface_on_read_complete(
            src,
            &self->base,
            &session_rec->session.base,
            session_rec->session.user_data,
            resource_id,
            attached->user_data);
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_detach_resourceEA(
                self,
                src,
                session_rec,
                attached,
                RTI_TRUE,
                request_param,
                &resource_kind,
                &ref_resource_id,
                &resource_data))
    {
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
        request_param);

    if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
    {
        session_rec = NULL;
        goto done;
    }
    session_rec = NULL;

    /* TODO iterate over existing session and delete resources whose
       native entity was deleted by this call */
    
    retcode = DDS_RETCODE_OK;
    
done:
    D2S2_ResourceId_finalize(&ref_resource_id);

    if (DDS_RETCODE_OK != retcode)
    {
        
    }
    
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_create_resource(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceRepresentation *const resource_repr,
    const D2S2_AttachedResourceId parent_id,
    const D2S2_ResourceProperties *const properties,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_create_resource)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec =
            NDDSA_ClientSessionRecord_from_session(session);
    void *resource_data = NULL;
    NDDSA_AttachedResource *attached = NULL;

    D2S2Log_fn_entry()

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_create_resourceEA(
                self,
                src,
                session_rec,
                resource_id,
                kind,
                resource_repr,
                parent_id,
                properties,
                request_param,
                &attached))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_INIT_FAILURE_s,
            D2S2_LOG_MSG_AGENT_CREATE_RESOURCE_FAILED);
        goto done;
    }
    
    /* At this point the resource exists and we can notify the caller of
       the result */
    if (DDS_RETCODE_OK !=
            D2S2_AgentServerInterface_on_resource_created(
                src,
                agent,
                &session_rec->session.base,
                session_rec->session.user_data,
                kind,
                &attached->base.resource,
                attached->base.id,
                request_param,
                &resource_data))
    {
        NDDSA_Agent_detach_resourceEA(
            self,
            src,
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


    /* TODO
       Iterate over existing sessions and:
         - update resources that might have been deleted and recreated.
         - delete resources that might have been deleted.  */
    
    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        
    }
    
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_lookup_resource(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    DDS_Boolean *const resource_exists_out,
    void **const resource_data_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec =
        NDDSA_ClientSessionRecord_from_session(session);
    NDDSA_AttachedResource *attached = NULL;
    
    UNUSED_ARG(src);
    UNUSED_ARG(self);

    if (resource_exists_out != NULL)
    {
        *resource_exists_out = DDS_BOOLEAN_FALSE;
    }
    if (resource_data_out != NULL)
    {
        *resource_data_out = NULL;
    }

    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, resource_id, &attached);
    
    if (resource_exists_out != NULL)
    {
        *resource_exists_out =
            (attached != NULL)? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
    }
    if (resource_data_out != NULL)
    {
        *resource_data_out = (attached != NULL)? attached->user_data : NULL;
    }
    
    retcode = DDS_RETCODE_OK;
    
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_read(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId reader_id,
    const D2S2_ReadSpecification *const read_spec,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_read)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec =
        NDDSA_ClientSessionRecord_from_session(session);;
    NDDSA_AttachedResource *attached = NULL;
    void *resource_data = NULL;

    D2S2Log_fn_entry()

    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, reader_id, &attached);
    
    if (attached == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "resource attached to this session");
        goto done;
    }

    if (attached->base.kind != D2S2_RESOURCEKIND_DATAREADER)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_ss,
            D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
            "reader");
        goto done;
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_readEA(
                self,
                src,
                session_rec,
                attached,
                read_spec,
                request_param,
                &resource_data))
    {
        goto done;
    }

    if (DDS_RETCODE_OK !=
            D2S2_AgentServerInterface_on_read_created(
                src,
                &self->base,
                &session_rec->session.base,
                session_rec->session.user_data,
                reader_id,
                resource_data,
                read_spec,
                request_param))
    {
        NDDSA_Agent_cancel_readI(self, src, session_rec, attached, NULL);
        goto done;
    }
    
    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        
    }
    
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_cancel_read(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId reader_id,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_cancel_read)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec =
            NDDSA_ClientSessionRecord_from_session(session);
    NDDSA_AttachedResource *attached = NULL;

    D2S2Log_fn_entry()

    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, reader_id, &attached);
    
    if (attached == NULL ||
        attached->base.kind != D2S2_RESOURCEKIND_DATAREADER)
    {
        goto done;
    }

    if (DDS_RETCODE_OK !=
                NDDSA_Agent_cancel_readI(
                    self,
                    src,
                    session_rec,
                    attached,
                    request_param))
    {
        goto done;
    }

    D2S2_AgentServerInterface_on_read_complete(
        src,
        &self->base,
        &session_rec->session.base,
        session_rec->session.user_data,
        reader_id,
        attached->user_data);
    
    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        
    }
    
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_return_loan(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    D2S2_ReceivedData *const data)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_return_loan)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_AttachedResource *attached = NULL;
    NDDSA_ReceivedData *rcvd_data = (NDDSA_ReceivedData*)data,
                       *cached_data = NULL;
    /* This function assumes to be called within the context of a
        D2S2_Agent_receive_message() call, where the session record
        has already been locked, so it is safe to just dereference
        the provided session, rather than looking it up again */
    NDDSA_ClientSessionRecord *session_rec =
        NDDSA_ClientSessionRecord_from_session(session);
    NDDSA_ResourceRecord *resource_rec = NULL;
    struct REDAInlineListNode *node = NULL;

    struct RTINtpTime ts_now = RTI_NTP_TIME_ZERO;
    RTIBool finite_request = RTI_FALSE,
            read_complete = RTI_FALSE;
    
    D2S2Log_fn_entry();

    if (&session_rec->session != rcvd_data->session)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "data does NOT belong to this session");
        goto done;
    }

    if (session_rec->session.intf != src)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "session does NOT belong to this interface");
        goto done;
    }

    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, rcvd_data->base.reader_id, &attached);
    
    if (attached == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "UNKNOWN resource for session");
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
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "no active read on reader");
        goto done;
    }

    node = REDAInlineList_getFirst(&attached->read_req->samples);
    while (node != NULL)
    {
        cached_data = NDDSA_ReceivedData_from_node(node);

        if (rcvd_data == cached_data)
        {
            break;
        }
        else
        {
            node = REDAInlineListNode_getNext(&cached_data->node);
            cached_data = NULL;
        }

    }

    if (cached_data == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "cached data NOT FOUND in reader state");
        goto done;
    }

    REDAInlineList_removeNodeEA(
        &attached->read_req->samples, &cached_data->node);

    REDAInlineList_addNodeToBackEA(
                &session_rec->session.rcvd_data_pool,
                &rcvd_data->node);

    if (DDS_SequenceNumber_compare(
        &attached->last_sample_returned,
        &rcvd_data->dds_sn) < 0)
    {
        attached->last_sample_returned = rcvd_data->dds_sn;
    }

    if (!self->clock->getTime(self->clock, &ts_now))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "get current timestamp");
        goto done;
    }

    /* lock resource state before changing read's state */
    if (!NDDSA_AgentDb_lookup_resource(&self->db, &attached->base.resource, &resource_rec))
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

    attached->read_req->samples_rcvd += 1;

    NDDSA_Read_is_complete(
        attached->read_req, &ts_now, &finite_request, &read_complete);

    if (finite_request && read_complete)
    {
        NDDSA_Read_set_status(
            attached->read_req, NDDSA_READSTATUS_COMPLETE);

        if (REDAInlineList_getFirst(&attached->read_req->samples) != NULL)
        {
            D2S2Log_warn(
                method_name,
                &RTI_LOG_ANY_s,
                "waiting to cancel completed read, because of pending samples");
            retcode = DDS_RETCODE_OK;
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
                "cancel read on resource");
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
    
    retcode = DDS_RETCODE_OK;
    
done:

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

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_write(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId writer_id,
    const D2S2_DataRepresentation *const data,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_write)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec =
            NDDSA_ClientSessionRecord_from_session(session);;
    NDDSA_AttachedResource *attached = NULL;

    D2S2Log_fn_entry()

    
    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, writer_id, &attached);
    
    // printf("*** WRITE DATA: session=%02X, writer=%04X, attached=%p, attached.kind=%d, dw.kind=%d\n",
    //     session->key.id,
    //     writer_id,
    //     attached,
    //     (attached != NULL)?attached->base.kind:0,
    //     D2S2_RESOURCEKIND_DATAWRITER);

    if (attached == NULL ||
        attached->base.kind != D2S2_RESOURCEKIND_DATAWRITER)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
            "write data");
        goto done;
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_writeEA(
                self,
                src,
                session_rec,
                attached,
                data,
                request_param))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "FAILED to write sample");
        goto done;
    }
    
    D2S2_AgentServerInterface_on_data_written(
        src,
        &self->base,
        &session_rec->session.base,
        session_rec->session.user_data,
        writer_id,
        attached->user_data,
        data,
        request_param);

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        
    }
    
    D2S2Log_fn_exit()

    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_load_resources_from_xml(
    D2S2_Agent *const agent,
    const char *const xml_url,
    const D2S2_ResourceProperties *const properties)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    char **seq_ref = NULL;
    DDS_UnsignedLong seq_len = 0,
                     url_len = 0;
    DDS_DomainParticipantFactory *factory = NULL;
    struct DDS_DomainParticipantFactoryQos factory_qos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;
    RTIBool file_url = RTI_FALSE,
            str_url = RTI_FALSE;

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_get_qos(factory, &factory_qos))
    {
        goto done;
    }

    url_len = strlen(xml_url);

    if (url_len <= 7 /* MIN PREFIX LEN */)
    {
        goto done;
    }

    file_url = (strncmp("file://", xml_url, 7) == 0);
    str_url = (strncmp("str://", xml_url, 6) == 0);

    if (!file_url && !str_url)
    {
        goto done;
    }

    if (file_url)
    {
        seq_len = DDS_StringSeq_get_length(&factory_qos.profile.url_profile);

        if (!DDS_StringSeq_ensure_length(
                &factory_qos.profile.url_profile, seq_len + 1, seq_len + 1))
        {
            goto done;
        }
        seq_ref = 
            DDS_StringSeq_get_reference(
                &factory_qos.profile.url_profile, seq_len);
    }
    else
    {
        seq_len = DDS_StringSeq_get_length(&factory_qos.profile.string_profile);

        if (!DDS_StringSeq_ensure_length(
                &factory_qos.profile.string_profile, seq_len + 1, seq_len + 1))
        {
            goto done;
        }
        seq_ref = 
            DDS_StringSeq_get_reference(
                &factory_qos.profile.string_profile, seq_len);
    }
    
    *seq_ref = DDS_String_dup(xml_url);

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_set_qos(factory, &factory_qos))
    {
        goto done;
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_create_implicit_resources(self, properties))
    {
        goto done;
    }
    
    retcode = DDS_RETCODE_OK;
    
done:
    DDS_DomainParticipantFactoryQos_finalize(&factory_qos);

    return retcode;
}

DDS_ReturnCode_t
NDDSA_Agent_receive_message(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key,
    void *const message)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_receive_message)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    DDS_Boolean dispose = DDS_BOOLEAN_FALSE;

    D2S2Log_fn_entry()

    AGENT_LOOKUP_SESSION_OR_DONE(
        self,
        session_key,
        &session_rec,
        /* on_unknown */
        retcode = DDS_RETCODE_OK;);

    if (session_rec->session.intf != src)
    {
        goto done;
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_on_session_activityI(self, session_rec))
    {
        goto done;
    }

    D2S2_AgentServerInterface_on_message_received(
        src,
        &self->base,
        &session_rec->session.base,
        session_rec->session.user_data,
        message,
        &dispose);
    
    if (dispose)
    {
        if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
        {
            session_rec = NULL;
            goto done;
        }
        session_rec = NULL;

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_close_session(&self->base, src, session_key))
        {
            goto done;
        }
    }
    
    retcode = DDS_RETCODE_OK;
    
done:
    if (session_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_session(&self->db, session_rec))
        {
            /* TODO log */
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
void
NDDSA_Agent_delete(D2S2_Agent *const agent)
{
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_Agent_finalize(self);
    RTIOsapiHeap_freeStructure(self);
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_start(D2S2_Agent *const agent)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_start)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    D2S2_AgentServerInterface *intf = NULL;

    D2S2Log_fn_entry()

    if (self->started)
    {
        D2S2Log_warn(
            method_name,
            &RTI_LOG_ANY_s,
            "agent already started yet");
        retcode = DDS_RETCODE_OK;
        goto done;
    }

    intf = (D2S2_AgentServerInterface*)
                REDAInlineList_getFirst(&self->interfaces);
    while (intf  != NULL)
    {
        if (DDS_RETCODE_OK !=
                D2S2_AgentServerInterface_on_agent_started(intf, &self->base))
        {
            goto done;
        }
        intf = (D2S2_AgentServerInterface*)
                    REDAInlineListNode_getNext(&intf->node);
    }

    self->started = RTI_TRUE;
    
    retcode = DDS_RETCODE_OK;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

#if 0
RTI_PRIVATE
RTIBool
NDDSA_Agent_find_active_datareader_resource(
    NDDSA_AgentDb *const self,
    NDDSA_ResourceRecord *const record,
    void *const param,
    RTIBool *const return_record_out)
{
    RTIBool retcode = RTI_FALSE,
            ret_rec = RTI_FALSE;
    NDDSA_Reader *reader_state = NULL;

    *return_record_out = RTI_FALSE;

    if (record->resource.base.kind != D2S2_RESOURCEKIND_DATAREADER)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    reader_state = (NDDSA_Reader*)record->resource.user_data;
    RTIOsapiSemaphore_take(reader_state->sem_requests, RTI_NTP_TIME_INFINITE);
    ret_rec = (REDAInlineList_getFirst(&reader_state->requests) != NULL);
    RTIOsapiSemaphore_give(reader_state->sem_requests);
    
    retcode = RTI_TRUE;
    
done:

    return retcode;
}
#endif

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_stop(D2S2_Agent *const agent)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_stop)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    D2S2_AgentServerInterface *intf = NULL;

    D2S2Log_fn_entry()

    if (!self->started)
    {
        D2S2Log_warn(
            method_name,
            &RTI_LOG_ANY_s,
            "agent not started yet");
        retcode = DDS_RETCODE_OK;
        goto done;
    }

#if 0
    DDS_DataReader *reader = NULL;
    struct DDS_DataReaderListener listener = DDS_DataReaderListener_INITIALIZER;
    NDDSA_Reader *reader_state = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;
    RTIBool in_iter = RTI_FALSE;
    /* Iterate over existing data-readers to disable listeners */
    if (!NDDSA_AgentDb_lock_resources(&self->db, RTI_TRUE, RTI_TRUE))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            D2S2_LOG_MSG_DB_LOCK_RESOURCES_FAILED);
        goto done;
    }
    in_iter = RTI_TRUE;

    do
    {
        if (!NDDSA_AgentDb_find_next_resourceEA(
                &self->db,
                NDDSA_Agent_find_active_datareader_resource,
                NULL,
                &resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_s,
                D2S2_LOG_MSG_DB_FIND_RESOURCE_FAILED);
            goto done;
        }
        if (resource_rec != NULL)
        {
            reader_state = (NDDSA_Reader*)resource_rec->resource.user_data;
            reader = DDS_DynamicDataReader_as_datareader(
                        reader_state->dyn_reader);

            if (DDS_RETCODE_OK !=
                    DDS_DataReader_set_listener(
                        reader, &listener, DDS_STATUS_MASK_NONE))
            {
                goto done;
            }
            // printf("***************************************\n");
            // printf("DISABLED READER: %s\n", resource_rec->resource.base.id.value.ref);
            // printf("***************************************\n");
        }
    } while (resource_rec != NULL);

    in_iter = RTI_FALSE;
    if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            D2S2_LOG_MSG_DB_UNLOCK_RESOURCES_FAILED);
        goto done;
    }
#endif

    intf = (D2S2_AgentServerInterface*)
                REDAInlineList_getFirst(&self->interfaces);
    while (intf  != NULL)
    {
        if (DDS_RETCODE_OK !=
                D2S2_AgentServerInterface_on_agent_stopped(intf, &self->base))
        {
            goto done;
        }
        intf = (D2S2_AgentServerInterface*)
                    REDAInlineListNode_getNext(&intf->node);
    }

    
    self->started = RTI_FALSE;

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
#if 0
        if (in_iter)
        {
            if (!NDDSA_AgentDb_unlock_resources(&self->db, RTI_TRUE))
            {
                D2S2Log_exception(
                    method_name,
                    &RTI_LOG_ANY_s,
                    D2S2_LOG_MSG_DB_UNLOCK_RESOURCES_FAILED);
            }
        }
#endif
    }
    D2S2Log_fn_exit()
    return retcode;
}


DDS_ReturnCode_t
NDDSA_Agent_generate_attached_resource_id(
    D2S2_Agent *const agent,
    const D2S2_ResourceId *resource_id,
    const D2S2_ResourceKind resource_kind,
    D2S2_AttachedResourceId *const id_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_generate_attached_resource_id)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    D2S2_EntityName res_name = D2S2_ENTITYNAME_INITIALIZER;
    char *resource_ref = NULL;
    const char *resource_ref_static = NULL;
    md5_state_t hash;
    DDS_UnsignedLong resource_ref_len = 0;
    md5_byte_t digest[CDR_MD5_DIGEST_SIZE] = { 0 },
               obj_kind = 0,
               obj_id[2] = { 0 };
    DDS_UnsignedShort id = 0;
    RTIBool extended = RTI_FALSE;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);

    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        obj_kind = 0x01;
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        obj_kind = 0x02;
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        obj_kind = 0x03;
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        obj_kind = 0x04;
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        obj_kind = 0x05;
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        obj_kind = 0x06;
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE:
    {
        obj_kind = 0x1F;
        extended = RTI_TRUE;
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
    {
        obj_kind = 0x2F;
        extended = RTI_TRUE;
        break;
    }
    default:
        goto done;
    }

#if DDS_AGENT_FEAT_STDAUTOID
    if (!D2S2_EntityName_from_id(&res_name, resource_id))
    {
        goto done;
    }

    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        if (!D2S2_EntityName_to_ref(
                &res_name,
                D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
                &resource_ref))
        {
            goto done;
        }
        resource_ref_static = resource_ref;
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        if (!D2S2_EntityName_component(
            &res_name,
            D2S2_ENTITYNAME_DEPTH_TOPIC,
            &resource_ref_static))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        if (!D2S2_EntityName_component(
            &res_name,
            D2S2_ENTITYNAME_DEPTH_PUBLISHER,
            &resource_ref_static))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        if (!D2S2_EntityName_component(
            &res_name,
            D2S2_ENTITYNAME_DEPTH_SUBSCRIBER,
            &resource_ref_static))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        if (!D2S2_EntityName_component(
            &res_name,
            D2S2_ENTITYNAME_DEPTH_DATAWRITER,
            &resource_ref_static))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        if (!D2S2_EntityName_component(
            &res_name,
            D2S2_ENTITYNAME_DEPTH_DATAREADER,
            &resource_ref_static))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE:
    {
        if (!D2S2_EntityName_component(
            &res_name,
            D2S2_ENTITYNAME_DEPTH_SERVICE,
            &resource_ref_static))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
    {
        if (!D2S2_EntityName_component(
            &res_name,
            D2S2_ENTITYNAME_DEPTH_SERVICE_RESOURCE,
            &resource_ref_static))
        {
            goto done;
        }
        break;
    }
    default:
        goto done;
    }
#else
    resource_ref_static = resource_id->value.ref;
#endif /* DDS_AGENT_FEAT_STDAUTOID */

    resource_ref_len = strlen(resource_ref_static);
    RTICdrMD5_init(&hash);
    RTICdrMD5_append(&hash, (md5_byte_t*)resource_ref_static, resource_ref_len);
    RTICdrMD5_finish(&hash, digest);

    obj_id[0] = digest[0];
    if (!extended)
    {
        obj_id[1] = (digest[1] & 0xf0) + obj_kind;
    }
    else
    {
        obj_id[1] = obj_kind;
    }

    if (RTIOsapiUtility_isNativeLittleEndian())
    {
        *((md5_byte_t*)&id) = obj_id[1];
        *(((md5_byte_t*)&id)+1) = obj_id[0];
    }
    else
    {
        *((md5_byte_t*)&id) = obj_id[0];
        *(((md5_byte_t*)&id)+1) = obj_id[1];
    }
    
    *id_out = (DDS_UnsignedLong)id;

    retcode = DDS_RETCODE_OK;
    
done:
    D2S2_EntityName_finalize(&res_name);
    if (resource_ref != NULL)
    {
        RTIOsapiHeap_freeString(resource_ref);
    }
    
    D2S2Log_fn_exit()
    return retcode;
}

/******************************************************************************
 * External Service functions
 ******************************************************************************/
DDS_ReturnCode_t
NDDSA_Agent_make_external_service_request(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId svc_res_id,
    const DDS_UnsignedLong svc_flags,
    const D2S2_Buffer * const svc_query,
    const D2S2_Buffer * const svc_data,
    const D2S2_Buffer * const svc_metadata,
    const DDS_Boolean no_reply,
    void *const request_param)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_make_external_service_request)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec =
            NDDSA_ClientSessionRecord_from_session(session);;
    NDDSA_AttachedResource *attached = NULL;

    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, svc_res_id, &attached);

    if (attached == NULL ||
        attached->base.kind != D2S2_RESOURCEKIND_SERVICE_RESOURCE)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
            "make service request");
        goto done;
    }

    if (DDS_RETCODE_OK !=
            NDDSA_Agent_make_external_service_requestEA(
                self,
                src,
                session_rec,
                attached,
                svc_flags,
                svc_query,
                svc_data,
                svc_metadata,
                no_reply))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "FAILED to make service request");
        goto done;
    }
    
    D2S2_AgentServerInterface_on_service_request_submitted(
        src,
        &self->base,
        &session_rec->session.base,
        session_rec->session.user_data,
        svc_res_id,
        attached->user_data,
        svc_flags,
        svc_query,
        svc_data,
        svc_metadata,
        request_param);

    retcode = DDS_RETCODE_OK;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_external_service_reply_available(
    D2S2_Agent *const agent,
    const D2S2_ResourceId *const svc_res_id,
    const D2S2_ExternalServiceRequestToken req_token)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_external_service_reply_available)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionRecord *session_rec = NULL;
    NDDSA_AttachedResource *attached = NULL;
    NDDSA_ResourceRecord *resource_rec = NULL;
    const struct RTINtpTime ts_zero = RTI_NTP_TIME_ZERO;
    NDDSA_ExternalServicePendingRequest *pending_req = NULL;
    NDDSA_ExternalServiceResource *svc_res = NULL;
    
    D2S2Log_fn_entry();

    if (!NDDSA_AgentDb_lookup_resource(&self->db, svc_res_id, &resource_rec))
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
    
    svc_res = resource_rec->resource.native.value.entity.service_resource;

    pending_req = (NDDSA_ExternalServicePendingRequest*)
        REDAInlineList_getFirst(&svc_res->requests);
    while (NULL != pending_req)
    {
        if (pending_req->request->token == req_token)
        {
            break;
        }
        pending_req = (NDDSA_ExternalServicePendingRequest*)
            REDAInlineListNode_getNext(&pending_req->node);
    }

    if (NULL == pending_req)
    {
        /* unknown request */
        /* TODO log */
        goto done;
    }
    
    if (!NDDSA_Agent_post_event(
            self,
            &pending_req->request->listener,
            &pending_req->request->listener_storage,
            sizeof(NDDSA_ExternalServiceRequest*),
            &ts_zero))
    {
        /* TODO log */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_return_external_service_reply(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    D2S2_ReceivedData *const data)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_return_external_service_reply)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *self = (NDDSA_Agent*)agent;
    NDDSA_AttachedResource *attached = NULL;
    NDDSA_ExternalServiceReply *reply =
        (NDDSA_ExternalServiceReply*)data;
    /* This function assumes to be called within the context of a
        D2S2_Agent_receive_message() call, where the session record
        has already been locked, so it is safe to just dereference
        the provided session, rather than looking it up again */
    NDDSA_ClientSessionRecord *session_rec =
        NDDSA_ClientSessionRecord_from_session(session);
    NDDSA_ResourceRecord *resource_rec = NULL;

    D2S2Log_fn_entry();

    if (&session_rec->session != reply->session)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "reply does NOT belong to this session");
        goto done;
    }

    if (session_rec->session.intf != src)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "session does NOT belong to this interface");
        goto done;
    }

    NDDSA_ClientSession_find_attached_resource(
        &session_rec->session, reply->base.reader_id, &attached);
    
    if (attached == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "UNKNOWN resource for session");
        goto done;
    }

    if (attached->base.kind != D2S2_RESOURCEKIND_SERVICE_RESOURCE)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_ss,
            D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND,
            "service resource");
        goto done;
    }

    if (attached->svc_req == NULL)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "no active request on service resource");
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

    /* TODO allow a request to be open ended like a read(). At the
        moment it is cancelled after the first reply */
    if (DDS_RETCODE_OK !=
        NDDSA_Agent_cancel_external_service_requestEA(
            self, src, session_rec, resource_rec, attached))
    {
        /* TODO log */
        goto done;
    }

    retcode = DDS_RETCODE_OK;
    
done:
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

/******************************************************************************
 * Session event functions
 ******************************************************************************/

RTIBool
NDDSA_Agent_on_session_event(
    const struct RTIEventGeneratorListener *me,
    struct RTINtpTime *newTime, struct RTINtpTime *newSnooze,
    const struct RTINtpTime *now, const struct RTINtpTime *time,
    const struct RTINtpTime *snooze,
    const struct RTIEventGeneratorListenerStorage *listenerStorage,
    struct REDAWorker *worker)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_on_session_event)
    NDDSA_Agent *const self =
        (NDDSA_Agent*) listenerStorage->field[0];
    D2S2_AgentServerInterface *const src =
        (D2S2_AgentServerInterface*) listenerStorage->field[1];
    NDDSA_ClientSessionEvent *const event =
        (NDDSA_ClientSessionEvent*) listenerStorage->field[2];
    const RTIBool cancelled = (listenerStorage->field[3] == NULL);
    NDDSA_ClientSessionRecord *session_rec = NULL;
    DDS_Boolean reschedule = DDS_BOOLEAN_FALSE;

    D2S2Log_fn_entry()

    UNUSED_ARG(now);
    UNUSED_ARG(time);
    UNUSED_ARG(snooze);
    UNUSED_ARG(worker);
    UNUSED_ARG(newSnooze);
    UNUSED_ARG(newTime);
    UNUSED_ARG(me);

    AGENT_LOOKUP_SESSION_OR_DONE(
        self, &event->base.session_key, &session_rec, /* on_unknown */);

    if (session_rec->session.intf != src)
    {
        goto done;
    }

    if (cancelled)
    {
        goto done;
    }

    event->on_event(&self->base, &session_rec->session.base, &event->base, &reschedule);

    if (reschedule)
    {
        if (DDS_RETCODE_OK !=
                NDDSA_Agent_fire_session_event(
                    &self->base, &event->base, &event->delay_ts))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                "NDDSA_Agent_fire_session_event");
            goto done;
        }
    }

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

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_create_session_event(
    D2S2_Agent *const agent,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    void *const event_listener,
    D2S2_Agent_OnSessionEventCallback on_event,
    D2S2_ClientSessionEvent **const event_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_create_session_event)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionEvent *event = NULL;
    NDDSA_ClientSessionRecord *session_rec =
        NDDSA_ClientSessionRecord_from_session(session);

    D2S2Log_fn_entry()

    if (session_rec->session.intf != src)
    {
        goto done;
    }

    event = (NDDSA_ClientSessionEvent*)
                REDAFastBufferPool_getBuffer(self->pool_events_session);
    if (event == NULL)
    {
        goto done;
    }
    RTIOsapiMemory_zero(event, sizeof(NDDSA_ClientSessionEvent*));
    event->base.intf = src;
    event->base.session_key = session_rec->session.base.key;
    event->base.listener = event_listener;
    event->on_event = on_event;
    event->listener_storage.field[0] = self;
    event->listener_storage.field[1] = src;
    event->listener_storage.field[2] = event;
    event->listener_storage.field[3] = NULL;
    
    *event_out = &event->base;

    retcode = DDS_RETCODE_OK;
    
done:

    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_fire_session_event(
    D2S2_Agent *const agent,
    D2S2_ClientSessionEvent *const uevent,
    const struct RTINtpTime *const delay)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_fire_session_event)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionEvent *event =
        (NDDSA_ClientSessionEvent*)uevent;
    
    D2S2Log_fn_entry()

    event->delay_ts = *delay;
    event->listener_storage.field[3] = event;

    if (!NDDSA_Agent_post_event(
            self,
            &self->session_event_listener,
            &event->listener_storage,
            sizeof(NDDSA_Agent*) +
                sizeof(D2S2_AgentServerInterface*) +
                    sizeof(NDDSA_ClientSessionEvent*),
            delay))
    {
        goto done;
    }
    
    retcode = DDS_RETCODE_OK;
    
done:

    return retcode;
}

RTI_PRIVATE
DDS_ReturnCode_t
NDDSA_Agent_delete_session_event(
    D2S2_Agent *const agent,
    D2S2_ClientSessionEvent *const uevent)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_delete_session_event)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NDDSA_Agent *const self = (NDDSA_Agent*)agent;
    NDDSA_ClientSessionEvent *event =
        (NDDSA_ClientSessionEvent*)uevent;
    const struct RTINtpTime ts_zero = RTI_NTP_TIME_ZERO;

    D2S2Log_fn_entry()

    event->listener_storage.field[3] = NULL;
    
    if (!NDDSA_Agent_post_event(
            self,
            &self->session_event_listener,
            &event->listener_storage,
            sizeof(NDDSA_Agent*) +
                sizeof(D2S2_AgentServerInterface*) +
                    sizeof(NDDSA_ClientSessionEvent*),
            &ts_zero))
    {
        goto done;
    }

    REDAFastBufferPool_returnBuffer(self->pool_events_session, event);
    
    retcode = DDS_RETCODE_OK;
    
done:

    return retcode;
}


const D2S2_AgentIntf NDDSA_Agent_fv_Intf = {
    NDDSA_Agent_delete,
    NDDSA_Agent_register_interface,
    NDDSA_Agent_register_external_service_plugin,
    NDDSA_Agent_open_session,
    NDDSA_Agent_close_session,
    NDDSA_Agent_create_resource,
    NDDSA_Agent_delete_resource,
    NDDSA_Agent_lookup_resource,
    NDDSA_Agent_read,
    NDDSA_Agent_cancel_read,
    NDDSA_Agent_return_loan,
    NDDSA_Agent_write,
    NDDSA_Agent_receive_message,
    NDDSA_Agent_make_external_service_request,
    NDDSA_Agent_return_external_service_reply,
    NDDSA_Agent_external_service_reply_available,
    NDDSA_Agent_load_resources_from_xml,
    NDDSA_Agent_start,
    NDDSA_Agent_stop,
    NDDSA_Agent_generate_attached_resource_id,
    NDDSA_Agent_create_session_event,
    NDDSA_Agent_fire_session_event,
    NDDSA_Agent_delete_session_event
};


#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */