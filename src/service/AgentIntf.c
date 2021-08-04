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

#include "Agent.h"
#include "ProxyClient.h"

void
NANO_XRCE_AgentInterface_on_interface_registered(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server)
{
    UNUSED_ARG(intf);
    UNUSED_ARG(server);
    /* nothing to do here */
}

void
NANO_XRCE_AgentInterface_on_interface_disposed(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server)
{
    UNUSED_ARG(intf);
    UNUSED_ARG(server);
    /* nothing to do here */
}

void
NANO_XRCE_AgentInterface_on_before_interface_disposed(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server)
{
    UNUSED_ARG(intf);
    UNUSED_ARG(server);
    /* nothing to do here */
}

DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_agent_started(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const agent)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(agent);
    
    NANO_PCOND(self != NULL)
    
    NANO_CHECK_RC(
        NANO_XRCE_Agent_enable(self),
        NANO_LOG_ERROR_MSG("FAILED to enable XRCE agent"));
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return (rc == NANO_RETCODE_OK)?DDS_RETCODE_OK:DDS_RETCODE_ERROR;
}

DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_agent_stopped(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const agent)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(agent);

    NANO_PCOND(self != NULL)
    
    NANO_CHECK_RC(
        NANO_XRCE_Agent_disable(self),
        NANO_LOG_ERROR_MSG("FAILED to disable XRCE agent"));
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return (rc == NANO_RETCODE_OK)?DDS_RETCODE_OK:DDS_RETCODE_ERROR;
}

void
NANO_XRCE_Agent_on_periodic_heartbeat(
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    D2S2_ClientSessionEvent *const event,
    DDS_Boolean *const reschedule_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)event->intf;
    NANO_XRCE_ProxyClient *const client =
        (NANO_XRCE_ProxyClient*)event->listener;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(server);
    UNUSED_ARG(session);
    
    UNUSED_ARG(self);
    NANO_PCOND(self != NULL)

    if (client->disposed)
    {
        /* already disposed */
        rc = NANO_RETCODE_OK;
        goto done;
    }
    // NANO_LOG_WARNING_MSG("periodic HB")
    NANO_CHECK_RC(
        NANO_XRCE_Session_send_periodic_heartbeats(
            &client->session, 0, 1), //NANO_XRCE_RELIABLESTREAM_SENDQUEUE_LENGTH_MAX / 2),
        NANO_LOG_ERROR_MSG("FAILED to send periodic heartbeat"));
    
    // NANO_CHECK_RC(
    //     NANO_XRCE_Session_send_periodic_acknacks(&client->session, 0),
    //     NANO_LOG_ERROR_MSG("FAILED to send periodic heartbeat"));
    
    *reschedule_out = DDS_BOOLEAN_TRUE;
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}




DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_session_opened(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ClientLocatorMapping *mapping = NULL;
    const NANO_XRCE_ClientLocatorMapping def_mapping =
        NANO_XRCE_CLIENTLOCATORMAPPING_INITIALIZER;
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    NANO_LOG_INFO("session OPENED",
        NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session)))

    client->dds_session = session;

    if (client->cache_mapping)
    {
        client->cache_mapping = NANO_BOOL_FALSE;

        RTIOsapiSemaphore_take(self->mappings_mutex, RTI_NTP_TIME_INFINITE);

        NANO_LOG_INFO("CACHING client's locator mapping",
            NANO_LOG_KEY("key", *NANO_XRCE_Session_key(&client->session))
            NANO_LOG_LOCATOR("locator", &client->transport.bind_entry.locator))

        /* Check if a mapping already exists or if we are adding a new one */
        NANO_CHECK_RC(
            NANO_XRCE_Agent_find_client_locator_mapping(
                self, &client->transport.bind_entry.locator, &mapping),
            RTIOsapiSemaphore_give(self->mappings_mutex);
            NANO_LOG_ERROR("FAILED to search for client locator",
                NANO_LOG_LOCATOR("src", &client->transport.bind_entry.locator)));
        
        if (mapping == NULL)
        {
            RTIOsapiHeap_reallocateArray(
                &self->props.client_mappings,
                self->props.client_mappings_len + 1,
                NANO_XRCE_ClientLocatorMapping);
            if (self->props.client_mappings == NULL)
            {
                RTIOsapiSemaphore_give(self->mappings_mutex);
                goto done;
            }
            mapping = &(self->props.client_mappings[self->props.client_mappings_len]);
            *mapping = def_mapping;
            self->props.client_mappings_len += 1;
            NANO_LOG_INFO("NEW client mapping",
                NANO_LOG_KEY("client",*NANO_XRCE_Session_key(&client->session))
                NANO_LOG_LOCATOR("locator",&client->transport.bind_entry.locator)
                NANO_LOG_PTR("mapping",mapping))
        }
        else
        {
            NANO_LOG_INFO("FOUND EXISTING client mapping",
                NANO_LOG_KEY("client",*NANO_XRCE_Session_key(&client->session))
                NANO_LOG_LOCATOR("locator",&client->transport.bind_entry.locator)
                NANO_LOG_PTR("mapping",mapping)
                NANO_LOG_KEY("mapping.client",mapping->client_key)
                NANO_LOG_LOCATOR("mapping.locator",&mapping->locator))
        }

        mapping->client_key = *NANO_XRCE_Session_key(&client->session);
        NANO_CHECK_RC(
            NANO_XRCE_TransportLocator_copy(
                &mapping->locator, &client->transport.bind_entry.locator),
            RTIOsapiSemaphore_give(self->mappings_mutex);
            NANO_LOG_ERROR_MSG("FAILED to copy mapping locator"));

        NANO_LOG_INFO("STORED client mapping",
            NANO_LOG_KEY("client",mapping->client_key)
            NANO_LOG_LOCATOR("locator",&mapping->locator)
            NANO_LOG_PTR("mapping",mapping))
        
        RTIOsapiSemaphore_give(self->mappings_mutex);
    }

    /* Mark session as "connected" */
    NANO_XRCE_Session_connected(&client->session);
    
    if (NANO_RETCODE_OK !=
            NANO_XRCE_Agent_reply_to_connection(self, client))
    {
        NANO_LOG_ERROR_MSG("FAILED to reply to client connection")
        goto done;
    }

    if (!NANO_Time_is_infinite(&self->props.heartbeat_period) &&
        !NANO_Time_is_zero(&self->props.heartbeat_period))
    {
        struct RTINtpTime hb_delay = RTI_NTP_TIME_ZERO;
        NANO_u64 hb_period_ms = 0;
        DDS_UnsignedLong delay_s = 0,
                         delay_ms = 0;

        if (DDS_RETCODE_OK !=
                D2S2_Agent_create_session_event(
                    server,
                    &self->base,
                    session,
                    client,
                    NANO_XRCE_Agent_on_periodic_heartbeat,
                    &client->event_hb))
        {
            NANO_LOG_ERROR_MSG("FAILED to create heartbeat event")
            goto done;
        }

        NANO_Time_to_millis(&self->props.heartbeat_period, &hb_period_ms);

        
        delay_s = hb_period_ms / 1000;
        delay_ms = hb_period_ms - (1000 * delay_s);

        RTINtpTime_packFromMillisec(hb_delay, delay_s, delay_ms);
        
        NANO_LOG_INFO("SCHEDULE hb event",
            NANO_LOG_U64("period_ms", hb_period_ms)
            NANO_LOG_U32("delay_s", delay_s)
            NANO_LOG_U32("delay_ms", delay_ms))

        if (DDS_RETCODE_OK !=
                D2S2_Agent_fire_session_event(
                    server, client->event_hb, &hb_delay))
        {
            NANO_LOG_ERROR_MSG("FAILED to fire heartbeat event")
            goto done;
        }
    }

    retcode = DDS_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT
    return retcode;
}


DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_session_reset(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const old_session_data,
    void *const session_data,
    void **const session_data_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client =
        (NANO_XRCE_ProxyClient*)old_session_data;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(server);
    UNUSED_ARG(session_data);
    
    NANO_PCOND(self != NULL)

    NANO_LOG_INFO("session RESET",
        NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session)))
    
    client->dds_session = session;

    /* Dismiss all forward requests associated with the client */
    NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
        self, client, NULL, D2S2_ATTACHEDRESOURCEID_INVALID, NANO_BOOL_FALSE /* confirmed */);

    NANO_XRCE_Session_reset_state(&client->session, NANO_BOOL_FALSE /* finalized */);

    NANO_CHECK_RC(
        NANO_XRCE_Agent_reply_to_connection(self, client),
        NANO_LOG_ERROR_MSG("FAILED to reply to client connection"));
    
    /* Mark session as "connected" */
    NANO_XRCE_Session_connected(&client->session);

    *session_data_out = client;
    
    retcode = DDS_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT
    return retcode;
}


void
NANO_XRCE_AgentInterface_on_session_closed(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    const D2S2_ClientSessionKey *const session_key,
    void *const session_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ProxyClientRequest *const request = client->dispose_request;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    UNUSED_ARG(server);
    UNUSED_ARG(session_key);

    NANO_LOG_INFO("session CLOSED",
        NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session)))

    client->dispose_request = NULL;
    if (request != NULL)
    {
        request->reply = NANO_BOOL_TRUE;
        NANO_CHECK_RC(
            NANO_XRCE_Agent_reply_to_operation(
                self, 
                request,
                NANO_RETCODE_OK),
            NANO_XRCE_Agent_return_client_request(self, request);
            NANO_LOG_ERROR_MSG("FAILED to send reply to client"));
        
        NANO_XRCE_Agent_return_client_request(self, request);
    }

    NANO_CHECK_RC(
        NANO_XRCE_Agent_delete_client_session(self, client),
        NANO_LOG_ERROR_MSG("FAILED to delete client state"));
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

void
NANO_XRCE_AgentInterface_on_session_timed_out(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(session);
    
    NANO_PCOND(self != NULL)

    NANO_LOG_INFO("session TIMED OUT",
        NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session)))
    
    if (client->event_hb != NULL)
    {
        if (DDS_RETCODE_OK !=
                D2S2_Agent_delete_session_event(server, client->event_hb))
        {
            NANO_LOG_ERROR_MSG("FAILED to delete heartbeat event")
            goto done;
        }
    }

    NANO_CHECK_RC(
        NANO_XRCE_Agent_dispose_client_session(self, client, NULL),
        NANO_LOG_ERROR_MSG("FAILED to delete client state"));
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_resource_created(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id,
    const D2S2_AttachedResourceId attach_id,
    void *const request_data,
    void **const resource_data_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ProxyClientRequest *const request =
        (NANO_XRCE_ProxyClientRequest*)request_data;
    NANO_XRCE_ObjectId obj_id = NANO_XRCE_OBJECTID_INVALID;
    void *resource_data = NULL;
    NANO_XRCE_ReaderState *reader_state = NULL;
    static const NANO_XRCE_ReaderState def_reader_state =
        NANO_XRCE_READERSTATE_INITIALIZER;
    NANO_XRCE_ExternalServiceResourceState *svc_res_state = NULL;
    static const NANO_XRCE_ExternalServiceResourceState def_svc_res_state =
        NANO_XRCE_EXERNALSERVICERESOURCESTATE_INITIALIZER;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(server);
    UNUSED_ARG(session);
    UNUSED_ARG(resource_id);
    
    NANO_PCOND(self != NULL)

    NANO_XRCE_ObjectId_from_u16(&obj_id, attach_id);

    NANO_LOG_INFO("resource CREATED",
        NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_OBJID("obj_id",obj_id)
        NANO_LOG_PTR("resource_data", resource_data)
        NANO_LOG_PTR("request_data",request_data))
    
    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        /* If the resource is a DataReader then we allocate a ReadRequest
        object that we will use to store the reader's read configuration */
        reader_state = (NANO_XRCE_ReaderState*)
                        REDAFastBufferPool_getBuffer(self->reads_pool);
        if (reader_state == NULL)
        {
            NANO_LOG_ERROR_MSG("FAILED to allocate reader state")
            goto done;
        }
        *reader_state = def_reader_state ;
        resource_data = reader_state ;
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
    {
        svc_res_state = (NANO_XRCE_ExternalServiceResourceState*)
            REDAFastBufferPool_getBuffer(self->service_requests_pool);
        if (svc_res_state == NULL)
        {
            NANO_LOG_ERROR_MSG("FAILED to allocate service resource state")
            goto done;
        }
        *svc_res_state = def_svc_res_state;
        resource_data = svc_res_state;
        break;
    }
    default:
    {
        break;
    }
    }

    if (request != NULL)
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_reply_to_operation(
                self, 
                request,
                NANO_RETCODE_OK),
            NANO_LOG_ERROR_MSG("FAILED to send reply to client"));
    }
    
    *resource_data_out = resource_data;

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (reader_state != NULL)
        {
            REDAFastBufferPool_returnBuffer(self->reads_pool, reader_state);
        }
    }
    NANO_LOG_FN_EXIT_RC(rc)
    return (rc == NANO_RETCODE_OK)? DDS_RETCODE_OK : DDS_RETCODE_ERROR;
}

void
NANO_XRCE_AgentInterface_on_resource_deleted(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id,
    const D2S2_AttachedResourceId attach_id,
    void *const resource_data,
    void *const request_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ProxyClientRequest *const request =
        (NANO_XRCE_ProxyClientRequest*)request_data;
    NANO_XRCE_ReaderState *reader_state = NULL;
    NANO_XRCE_ExternalServiceResourceState *svc_res_state = NULL;
    
    NANO_XRCE_ObjectId obj_id = NANO_XRCE_OBJECTID_INVALID;
    
    NANO_LOG_FN_ENTRY
    
    UNUSED_ARG(server);
    UNUSED_ARG(session);

    NANO_PCOND(self != NULL)

    NANO_XRCE_ObjectId_from_u16(&obj_id, attach_id);

    NANO_LOG_INFO("resource DELETED",
        NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_OBJID("obj_id",obj_id)
        NANO_LOG_STR("id", resource_id->value.ref)
        NANO_LOG_PTR("resource_data", resource_data)
        NANO_LOG_PTR("request_data",request_data))


    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        reader_state = (NANO_XRCE_ReaderState*) resource_data;
        REDAFastBufferPool_returnBuffer(self->reads_pool, reader_state);
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
    {
        svc_res_state = (NANO_XRCE_ExternalServiceResourceState*) resource_data;
        REDAFastBufferPool_returnBuffer(self->service_requests_pool, svc_res_state);
        break;
    }
    default:
    {
        break;
    }
    }

    if (request != NULL)
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_reply_to_operation(
                self, 
                request,
                NANO_RETCODE_OK),
            NANO_LOG_WARNING_MSG(
                "FAILED to send reply to client about deleted resource"));
    }
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}


void
NANO_XRCE_AgentInterface_on_resource_updated(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id,
    const D2S2_AttachedResourceId attach_id,
    void *const resource_data,
    void *const request_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(client);
    UNUSED_ARG(self);
    UNUSED_ARG(request_data);
    UNUSED_ARG(resource_data);
    UNUSED_ARG(attach_id);
    UNUSED_ARG(resource_id);
    UNUSED_ARG(resource_kind);
    UNUSED_ARG(session);
    UNUSED_ARG(server);
    
    NANO_PCOND(self != NULL)
    
    
    
    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_data_available(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const reader_data,
    const D2S2_ReceivedData *const data,
    DDS_Boolean *const retained_out,
    DDS_Boolean *const try_again_out)
{
    UNUSED_ARG(server);

    return NANO_XRCE_Agent_on_data_received(
        (NANO_XRCE_Agent*)intf,
        session,
        session_data,
        reader_id,
        reader_data,
        data,
        retained_out,
        try_again_out);
}

DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_read_created(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const reader_data,
    const D2S2_ReadSpecification *const read_spec,
    void *const request_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ReaderState *const reader_state =
        (NANO_XRCE_ReaderState*)reader_data;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    UNUSED_ARG(request_data);
    UNUSED_ARG(read_spec);
    UNUSED_ARG(reader_id);
    UNUSED_ARG(session);
    UNUSED_ARG(server);


    NANO_LOG_INFO("subscription CREATED",
        NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_OBJID("reader", reader_state->req->object_id)
        NANO_LOG_REQID("req_id",reader_state->req->request)
        NANO_LOG_U32("max_samples",reader_state->read_spec_dds.max_samples)
        NANO_LOG_U32("max_elapsed_time",reader_state->read_spec_dds.max_elapsed_time)
        NANO_LOG_U32("min_pace_period",reader_state->read_spec_dds.min_pace_period)
        NANO_LOG_STR("content_filter",reader_state->read_spec_dds.content_filter_expr))
    
    reader_state->forward = NANO_BOOL_TRUE;

    if (reader_state->req->reply)
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_reply_to_operation(
                self, 
                reader_state->req,
                NANO_RETCODE_OK),
            NANO_LOG_ERROR_MSG("FAILED to send reply to client"));
    }
    
    rc = NANO_RETCODE_OK;

done:
    if (NANO_RETCODE_OK != rc)
    {
        /* Request will be disposed upstream upon error */
        reader_state->req = NULL;
    }
    
    NANO_LOG_FN_EXIT_RC(rc)

    return (rc == NANO_RETCODE_OK)? DDS_RETCODE_OK : DDS_RETCODE_ERROR;
}

void
NANO_XRCE_AgentInterface_on_read_complete(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const reader_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ReaderState *const reader_state =
        (NANO_XRCE_ReaderState*)reader_data;

    NANO_LOG_FN_ENTRY
    
    UNUSED_ARG(self);
    UNUSED_ARG(reader_id);
    UNUSED_ARG(session);
    UNUSED_ARG(server);

    NANO_PCOND(self != NULL)
    
    NANO_LOG_INFO("subscription COMPLETE",
        NANO_LOG_KEY("client",
            *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session",
            *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_REQID("req_id", reader_state->req->request))
    
    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

DDS_ReturnCode_t
NANO_XRCE_AgentInterface_on_release_read_samples(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const reader_data)
{
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ReaderState *const reader_state =
        (NANO_XRCE_ReaderState*)reader_data;
    NANO_XRCE_Stream *stream = NULL;

    NANO_LOG_FN_ENTRY
    
    UNUSED_ARG(self);
    UNUSED_ARG(reader_id);
    UNUSED_ARG(session);
    UNUSED_ARG(server);

    NANO_PCOND(self != NULL)
    
    NANO_LOG_INFO("releasing pending samples",
        NANO_LOG_KEY("client",
            *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session",
            *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_REQID("req_id", reader_state->req->request))

    stream =
        NANO_XRCE_Agent_lookup_stream(self, client, reader_state->stream_id);
    NANO_PCOND(stream != NULL)

    NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
        self, client, stream, reader_id, NANO_BOOL_FALSE /* confirmed */);

    rc = DDS_RETCODE_OK;

    NANO_LOG_FN_EXIT
    return rc;
}

void
NANO_XRCE_AgentInterface_on_data_written(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const server,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId writer_id,
    void *const writer_data,
    const D2S2_DataRepresentation *const data,
    void *const request_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_WriteRequest *const write_req = 
        (NANO_XRCE_WriteRequest*)request_data;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(server);
    UNUSED_ARG(session);
    UNUSED_ARG(writer_id);
    UNUSED_ARG(writer_data);
    UNUSED_ARG(data);
    
    NANO_PCOND(self != NULL)

    NANO_LOG_DEBUG("data WRITTEN",
        NANO_LOG_KEY("client",
            *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session",
            *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_REQID("req_id", write_req->req->request))
    
    if (write_req->req->reply)
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_reply_to_operation(
                self, 
                write_req->req,
                NANO_RETCODE_OK),
            NANO_LOG_ERROR_MSG("FAILED to send reply to client"));
    }

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        /* request will be disposed upstream on error */
        write_req->req = NULL;
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

void
NANO_XRCE_AgentInterface_on_message_received(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    void *const message,
    DDS_Boolean *const dispose_out)
{
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_MessageBuffer *const msg = (NANO_MessageBuffer*)message;
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_bool retained = NANO_BOOL_FALSE;
    NANO_XRCE_Stream *stream = NULL;
    NANO_XRCE_StreamId stream_id = NANO_XRCE_STREAMID_NONE;
    NANO_XRCE_SessionId sid = NANO_XRCE_SESSIONID_NONE_WITHOUT_CLIENT;

    UNUSED_ARG(session);
    UNUSED_ARG(agent);

    if (client->disposed)
    {
        NANO_LOG_DEBUG_MSG("SUBMSG ignored because client is disposed")
        goto done;
    }

    /* pre-parse message header and allocate a */
    if (!NANO_XRCE_Session_parse_xrce_message_header(
                &client->session, msg, &sid, &stream_id))
    {
        /* ignore invalid messages */
        goto done;
    }

    stream = NANO_XRCE_Agent_lookup_stream(self, client, stream_id);
    if (stream == NULL)
    {
        NANO_LOG_ERROR("UNKNOWN stream",
            NANO_LOG_KEY("session.key", client->session.key)
            NANO_LOG_SESSIONID("session.id", client->session.id)
            NANO_LOG_STREAMID("stream", stream_id))
        goto done;
    }
    NANO_PCOND(stream != NULL)

    NANO_CHECK_RC(
        NANO_XRCE_Session_receive_message(
            &client->session, stream, msg, &retained),
        NANO_LOG_ERROR_MSG("FAILED to receive message in XRCE session"));

    NANO_PCOND(!retained)

    *dispose_out = 
        (client->disposed)? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
    
done:

    return;
}

void
NANO_XRCE_AgentInterface_on_service_request_submitted(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId svc_resource_id,
    void *const resource_data,
    const D2S2_ExternalServiceRequestFlags svc_flags,
    const D2S2_Buffer * const svc_query,
    const D2S2_Buffer * const svc_data,
    const D2S2_Buffer * const svc_metadata,
    void *const request_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ProxyClientRequest *request =
        (NANO_XRCE_ProxyClientRequest*)request_data;
    NANO_XRCE_ExternalServiceResourceState *svc_res_state =
        (NANO_XRCE_ExternalServiceResourceState*)resource_data;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(agent);
    UNUSED_ARG(session);
    
    NANO_PCOND(self != NULL)

    NANO_LOG_DEBUG("request SUBMITTED",
        NANO_LOG_KEY("client",
            *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session",
            *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_REQID("req_id", request->request))

    if (request->reply)
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_reply_to_operation(self, request, NANO_RETCODE_OK),
            NANO_LOG_ERROR_MSG("FAILED to send reply to client"));
    }

    if (!NANO_XRCE_SubmessageFlags_SERVICEREQUEST_oneway(request->submsg_hdr.flags))
    {
        svc_res_state->req = request;
        request = NULL;
    }

    rc = NANO_RETCODE_OK;

done:
    if (NULL != request)
    {
        NANO_XRCE_Agent_return_client_request(self, request);
    }
    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

void
NANO_XRCE_AgentInterface_on_release_service_replies(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId svc_resource_id,
    void *const resource_data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *const client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_ExternalServiceResourceState *svc_res_state =
        (NANO_XRCE_ExternalServiceResourceState*)resource_data;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(agent);
    UNUSED_ARG(session);
    
    NANO_PCOND(self != NULL)

    NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
        self, client, NULL, svc_resource_id, NANO_BOOL_TRUE /* confirmed */);
    if (NULL != svc_res_state->req)
    {
        NANO_XRCE_Agent_return_client_request(self, svc_res_state->req);
        svc_res_state->req = NULL;
    }

    rc = NANO_RETCODE_OK;

done:
    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

void
NANO_XRCE_AgentInterface_on_service_reply_available(
    D2S2_AgentServerInterface *const intf,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId svc_resource_id,
    void *const resource_data,
    const D2S2_ExternalServiceReplyStatus svc_reply_status,
    const DDS_UnsignedLong data_len,
    const DDS_UnsignedLong metadata_len,
    const D2S2_ReceivedData *const reply,
    DDS_Boolean * const retained_out,
    DDS_Boolean * const try_again_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Agent *const self = (NANO_XRCE_Agent*)intf;
    NANO_XRCE_ProxyClient *client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_Session *xrce_session = &client->session;
    NANO_XRCE_ObjectId obj_id = NANO_XRCE_OBJECTID_INVALID;
    NANO_XRCE_ExternalServiceResourceState *svc_res_state =
        (NANO_XRCE_ExternalServiceResourceState*)resource_data;
    const D2S2_XcdrData *xcdr_data = NULL;
    NANO_XRCE_Stream *reply_stream = NULL;
    NANO_XRCE_ServiceReplyPayload *data_reply = NULL;
    NANO_MessageBuffer *mbuf_header = NULL,
                        *mbuf_payload = NULL;
    NANO_bool fwd_req_added = NANO_BOOL_FALSE;
    NANO_XRCE_ForwardDataRequest *fwd_req = NULL;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(session);

    NANO_XRCE_ObjectId_from_u16(&obj_id, svc_resource_id);

    NANO_LOG_DEBUG("request reply AVAILABLE",
        NANO_LOG_KEY("client",
            *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session",
            *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_OBJID("obj_id", obj_id))

    *retained_out = DDS_BOOLEAN_FALSE;
    *try_again_out = DDS_BOOLEAN_FALSE;

    if (client->disposed)
    {
        NANO_LOG_WARNING_MSG("data IGNORED because client is DISPOSED")
        rc = NANO_RETCODE_OK;
        goto done;
    }

    if (reply->data->fmt != D2S2_DATAREPRESENTATIONFORMAT_XCDR)
    {
        NANO_LOG_WARNING("UNSUPPORTED data format",
            NANO_LOG_H32("fmt", reply->data->fmt))
        rc = NANO_RETCODE_OK;
        goto done;
    }

    if (NULL == svc_res_state->req)
    {
        NANO_LOG_WARNING("UNEXPECTED service reply",
            NANO_LOG_KEY("session.key",*(NANO_XRCE_Session_key(&client->session)))
            NANO_LOG_SESSIONID("session.id",*(NANO_XRCE_Session_id(&client->session)))
            NANO_LOG_OBJID("obj_id", obj_id))
        rc = NANO_RETCODE_OK;
        goto done;
    }

    xcdr_data = &reply->data->value.xcdr;

    if (xcdr_data->buffer.data_len == 0)
    {
        NANO_LOG_WARNING_MSG("REFUSING to deliver empty message")
        rc = NANO_RETCODE_OK;
        goto done;
    }

    reply_stream =
        NANO_XRCE_Agent_lookup_stream(
            self, client, svc_res_state->req->msg_hdr.stream_id);
    if (reply_stream == NULL)
    {
        NANO_LOG_ERROR("UNKNOWN stream for DATA forward",
            NANO_LOG_KEY("session.key",*(NANO_XRCE_Session_key(&client->session)))
            NANO_LOG_SESSIONID("session.id",*(NANO_XRCE_Session_id(&client->session)))
            NANO_LOG_OBJID("obj_id", obj_id)
            NANO_LOG_STREAMID("stream_id", svc_res_state->req->msg_hdr.stream_id))
        goto done;
    }

    /* Allocate a forward data request to store state until we
        get notified by send_complete() (or discard it after send, if
        we are sending on best-effort) */
    
    NANO_CHECK_RC(
        NANO_XRCE_Agent_allocate_reply_message(
            self,
            client,
            reply_stream,
            svc_res_state->req->msg_hdr.stream_id,
            NANO_XRCE_SERVICEREPLYPAYLOAD_HEADER_SERIALIZED_SIZE_MAX,
            NULL,
            &mbuf_header,
            NULL /* reply_stream_out */),
        NANO_LOG_TRACE("FAILED to allocate mbuf for SERVICE_REPLY header",
            NANO_LOG_USIZE("payload_size",
                xcdr_data->buffer.data_len))
        *try_again_out = DDS_BOOLEAN_TRUE;
        rc = NANO_RETCODE_OK;
        );
    
    NANO_PCOND(mbuf_header != NULL)

    /* Serialize SERVICE_REPLY message header to buffer */
    data_reply = (NANO_XRCE_ServiceReplyPayload*)
        NANO_MessageBuffer_data_ptr(mbuf_header);
    
    data_reply->base.related_request.request_id = svc_res_state->req->request;
    data_reply->base.related_request.object_id = svc_res_state->req->object_id;
    data_reply->base.result.status = NANO_XRCE_STATUS_OK;
    data_reply->base.result.implementation_status = NANO_RETCODE_OK;
    NANO_u16_serialize(svc_reply_status, &data_reply->status, NANO_CDR_ENDIANNESS_NATIVE);
    NANO_u32_serialize(data_len, &data_reply->data_len, NANO_CDR_ENDIANNESS_NATIVE);
    NANO_u32_serialize(metadata_len, &data_reply->metadata_len, NANO_CDR_ENDIANNESS_NATIVE);
    data_reply->has_payload = xcdr_data->buffer.data_len > 0;

    if (data_reply->has_payload)
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_allocate_reply_message(
                self,
                client,
                reply_stream,
                svc_res_state->req->msg_hdr.stream_id,
                xcdr_data->buffer.data_len,
                xcdr_data->buffer.data,
                &mbuf_payload,
                NULL /* reply_stream_out */),
            NANO_LOG_TRACE("FAILED to allocate mbuf for SERVICE_REPLY payload",
                NANO_LOG_USIZE("payload_size",
                    xcdr_data->buffer.data_len))
            NANO_XRCE_Agent_release_reply_message(
                self,
                client,
                svc_res_state->req->msg_hdr.stream_id,
                mbuf_header);
            *try_again_out = DDS_BOOLEAN_TRUE;
            rc = NANO_RETCODE_OK;);
        NANO_PCOND(mbuf_payload != NULL)
        NANO_MessageBuffer_append(mbuf_header, mbuf_payload);
    }

    fwd_req = (NANO_XRCE_ForwardDataRequest*)
        REDAFastBufferPool_getBuffer(client->forwards_pool);
    if (fwd_req == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate forward request")
        goto done;
    }

    NANO_LOG_DEBUG("FORWARDING service reply",
        NANO_LOG_KEY("client", *NANO_XRCE_Session_key(xrce_session))
        NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(xrce_session))
        NANO_LOG_STREAMID("stream", svc_res_state->req->msg_hdr.stream_id)
        NANO_LOG_USIZE("data_len", data_len)
        NANO_LOG_USIZE("metadata_len", metadata_len)
        NANO_LOG_USIZE("xcdr_data->len", xcdr_data->buffer.data_len)
        NANO_LOG_BOOL("has_payload", data_reply->has_payload)
        NANO_LOG_USIZE("offset(has_payload)", NANO_OSAPI_MEMBER_OFFSET(NANO_XRCE_ServiceReplyPayload, has_payload))
        NANO_LOG_MBUF("payload", mbuf_header))
    
    /* cache state so that we can look it up in on_send_complete */
    REDAInlineListNode_init(&fwd_req->node);
    fwd_req->rcvd_data = reply;
    fwd_req->stream_id = svc_res_state->req->msg_hdr.stream_id;
    fwd_req->reader_id = obj_id;

    REDAInlineList_addNodeToBackEA(&client->forward_replies, &fwd_req->node);
    fwd_req_added = NANO_BOOL_TRUE;

    NANO_CHECK_RC(
        NANO_XRCE_Session_send(
            xrce_session, 
            reply_stream,
            NANO_XRCE_SUBMESSAGEID_SERVICE_REPLY,
            NANO_XRCE_SUBMESSAGEFLAGS_ENDIANNESS,
            mbuf_header,
            &fwd_req->sn),
        mbuf_header = NULL;
        NANO_LOG_ERROR("failed to send SERVICE_REPLY message",
            NANO_LOG_RC(rc)));
    mbuf_header = NULL;
    
    if (!NANO_XRCE_StreamId_is_reliable(svc_res_state->req->msg_hdr.stream_id))
    {
        REDAInlineList_removeNodeEA(&client->forwards, &fwd_req->node);
        REDAFastBufferPool_returnBuffer(client->forwards_pool, fwd_req);
    }
    else
    {
        *retained_out = NANO_BOOL_TRUE;
    }
    /* Reliable samples are always removed by on_send_complete */
    fwd_req_added = NANO_BOOL_FALSE;
    fwd_req = NULL;

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (fwd_req != NULL)
        {
            if (fwd_req_added)
            {
                REDAInlineList_removeNodeEA(&client->forwards, &fwd_req->node);
            }
            REDAFastBufferPool_returnBuffer(client->forwards_pool, fwd_req);
        }
        if (mbuf_header != NULL)
        {
            NANO_XRCE_Agent_release_reply_message(
                self,
                client,
                svc_res_state->req->msg_hdr.stream_id,
                mbuf_header);
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
}

const D2S2_AgentServerInterfaceApi NANO_XRCE_gv_AgentInterfaceApi =
{
    NANO_XRCE_AgentInterface_on_session_opened,
    NANO_XRCE_AgentInterface_on_session_reset,
    NANO_XRCE_AgentInterface_on_session_closed,
    NANO_XRCE_AgentInterface_on_session_timed_out,
    NANO_XRCE_AgentInterface_on_resource_created,
    NANO_XRCE_AgentInterface_on_resource_deleted,
    NANO_XRCE_AgentInterface_on_resource_updated,
    NANO_XRCE_AgentInterface_on_read_created,
    NANO_XRCE_AgentInterface_on_read_complete,
    NANO_XRCE_AgentInterface_on_release_read_samples,
    NANO_XRCE_AgentInterface_on_data_available,
    NANO_XRCE_AgentInterface_on_data_written,
    NANO_XRCE_AgentInterface_on_interface_registered,
    NANO_XRCE_AgentInterface_on_interface_disposed,
    NANO_XRCE_AgentInterface_on_before_interface_disposed,
    NANO_XRCE_AgentInterface_on_message_received,
    NANO_XRCE_AgentInterface_on_agent_started,
    NANO_XRCE_AgentInterface_on_agent_stopped,
    NANO_XRCE_AgentInterface_on_service_request_submitted,
    NANO_XRCE_AgentInterface_on_release_service_replies,
    NANO_XRCE_AgentInterface_on_service_reply_available
};