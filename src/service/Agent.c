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

#include "clock/clock_highResolution.h"

#include "Agent.h"
#include "ProxyClient.h"

#define OP_REPLY_LOG        NANO_LOG_TRACE

NANO_PRIVATE
NANO_RetCode
NANO_XRCE_AgentProperties_copy(
    NANO_XRCE_AgentProperties *const self,
    const NANO_XRCE_AgentProperties *const from)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(from != NULL)
    
    self->client_session_timeout = from->client_session_timeout;
    self->auto_client_mapping = from->auto_client_mapping;
    
    self->client_mappings_len = from->client_mappings_len;
    if (self->client_mappings_len == 0)
    {
        if (self->client_mappings != NULL)
        {
            RTIOsapiHeap_freeArray(self->client_mappings);
        }
        self->client_mappings = NULL;
    }
    else
    {
        RTIOsapiHeap_allocateArray(
            &self->client_mappings,
            self->client_mappings_len,
            NANO_XRCE_ClientLocatorMapping);
        if (self->client_mappings == NULL)
        {
            goto done;
        }
        /* TODO transform client_mappings in to a sequence and do a
           proper deep copy (right now "string" locators will not be
           deep-copied, and they are effectively not supported) */
        memcpy(
            self->client_mappings,
            from->client_mappings,
            self->client_mappings_len * 
                sizeof(NANO_XRCE_ClientLocatorMapping));
    }

    self->heartbeat_period = from->heartbeat_period;
    self->confirm_all_requests = from->confirm_all_requests;
    self->auto_delete_resources = from->auto_delete_resources;
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

D2S2_AgentServerInterface*
NANO_XRCE_Agent_as_interface(NANO_XRCE_Agent *const self)
{
    return &self->base;
}


NANO_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_register_user_stream(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    const NANO_XRCE_StreamId stream_id,
    NANO_XRCE_Stream **const stream_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_StreamStorage *storage = NULL;

    NANO_XRCE_StreamStorageRecord *record = NULL;
    NANO_XRCE_Stream *stream = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    UNUSED_ARG(self);

    NANO_CHECK_RC(
        NANO_XRCE_ProxyClient_allocate_stream_storage(client, &record),
        NANO_LOG_ERROR_MSG("FAILED to allocate STREAM STORAGE"));
    
    storage = &record->storage;
    
    NANO_CHECK_RC(
        NANO_XRCE_Session_register_stream(
            &client->session, stream_id, storage),
        NANO_LOG_ERROR_MSG("FAILED to register user stream"));
    
    stream = NANO_XRCE_Session_lookup_stream(&client->session, stream_id);
    NANO_PCOND(stream != NULL)

    *stream_out = stream;
    
    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (record != NULL)
        {
            NANO_XRCE_ProxyClient_release_stream_storage(client, record);
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_XRCE_Stream*
NANO_XRCE_Agent_lookup_stream(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    const NANO_XRCE_StreamId stream_id)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Stream *res = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    res = NANO_XRCE_Session_lookup_stream(&client->session, stream_id);
    if (res == NULL &&
        stream_id != NANO_XRCE_STREAMID_NONE &&
        stream_id != NANO_XRCE_STREAMID_BUILTIN_BEST_EFFORTS &&
        stream_id != NANO_XRCE_STREAMID_BUILTIN_RELIABLE)
    {
        NANO_LOG_INFO("registering new USER STREAM",
            NANO_LOG_KEY("session.key", client->session.key)
            NANO_LOG_SESSIONID("session.id", client->session.id)
            NANO_LOG_STREAMID("stream", stream_id))
        
        NANO_CHECK_RC(
            NANO_XRCE_Agent_register_user_stream(
                self, client, stream_id, &res),
            NANO_LOG_ERROR_MSG("FAILED to allocate user stream"));
    }
    NANO_PCOND(res != NULL)
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_PTR(res)
    return res;
}

NANO_XRCE_ProxyClientRequest*
NANO_XRCE_Agent_new_client_request(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    const NANO_XRCE_MessageHeader *const msg_hdr,
    const NANO_XRCE_SubmessageHeader *const submsg_hdr,
    const NANO_XRCE_RequestId *const req_id,
    const NANO_XRCE_ObjectId *const obj_id,
    const NANO_bool reply)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ProxyClientRequest *request = NULL,
                        def_request = NANO_XRCE_PROXYCLIENTREQUEST_INITIALIZER;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    request = (NANO_XRCE_ProxyClientRequest*)
                    REDAFastBufferPool_getBuffer(client->requests_pool);
    if (request == NULL)
    {
        goto done;
    }
    *request = def_request;

    NANO_XRCE_MessageHeader_copy(&request->msg_hdr, msg_hdr);
    request->submsg_hdr = *submsg_hdr;
    request->client = client;
    request->request = *req_id;
    request->object_id = *obj_id;
    request->reply = reply;

    /* Pre-allocate a reply buffer for requests exchanged on a reliable stream:
        this way we prevent the case where we might successfully carry out
        an operation but later fail to allocate a reply message to notify
        the client of the outcome. */
    if (NANO_XRCE_StreamId_is_reliable(request->msg_hdr.stream_id) &&
        request->reply)
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_allocate_reply_message(
                self,
                client,
                NULL /* reply_stream */,
                request->msg_hdr.stream_id,
                NANO_XRCE_STATUSPAYLOAD_SERIALIZED_SIZE_MAX,
                NULL /* user_payload */,
                &request->reply_mbuf,
                NULL /* reply_stream_out */),
            NANO_LOG_ERROR("FAILED to allocate reply message",
                NANO_LOG_STREAMID("stream_id", request->msg_hdr.stream_id)
                NANO_LOG_USIZE("payload_size",
                    NANO_XRCE_STATUSPAYLOAD_SERIALIZED_SIZE_MAX)));
    }

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (request != NULL)
        {
            REDAFastBufferPool_returnBuffer(client->requests_pool, request);
            request = NULL;
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return request;
}

void
NANO_XRCE_Agent_return_client_request(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request)
{
    NANO_LOG_FN_ENTRY
    UNUSED_ARG(self);
    REDAFastBufferPool_returnBuffer(request->client->requests_pool, request);
    NANO_LOG_FN_EXIT
}


NANO_RetCode
NANO_XRCE_Agent_find_client_locator_mapping(
    NANO_XRCE_Agent *const self,
    const NANO_XRCE_TransportLocator *const locator,
    NANO_XRCE_ClientLocatorMapping **const mapping_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ClientLocatorMapping *mapping = NULL;
    NANO_usize i = 0;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    *mapping_out = NULL;

    for (i = 0; i < self->props.client_mappings_len; i++)
    {
        mapping = &(self->props.client_mappings[i]);
        if (0 == NANO_XRCE_TransportLocator_compare(&mapping->locator, locator))
        {
            *mapping_out = mapping;
            break;
        }
    }

    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_XRCE_Agent*
NANO_XRCE_Agent_new(NANO_XRCE_AgentProperties *const properties)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    const NANO_XRCE_Agent def_agent = NANO_XRCE_AGENT_INITIALIZER;
    NANO_XRCE_Agent *self = NULL;

    NANO_LOG_FN_ENTRY

    RTIOsapiHeap_allocateStructure(&self, NANO_XRCE_Agent);
    if (self == NULL)
    {
        goto done;
    }

    *self = def_agent;

    NANO_CHECK_RC(
        NANO_XRCE_Agent_initialize(self, properties),
        NANO_LOG_ERROR_MSG("FAILED to initialize agent"));
    
    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc && self != NULL)
    {
        RTIOsapiHeap_freeStructure(self);
        self = NULL;
    }
    
    NANO_LOG_FN_EXIT_PTR(self)
    return self;
}

void
NANO_XRCE_Agent_delete(NANO_XRCE_Agent *const self)
{
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    NANO_XRCE_Agent_finalize(self);
    RTIOsapiHeap_freeStructure(self);
    
    NANO_LOG_FN_EXIT
}

NANO_RetCode
NANO_XRCE_Agent_initialize(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_AgentProperties *const properties)
{
    const NANO_XRCE_Agent def_self = NANO_XRCE_AGENT_INITIALIZER;
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    struct REDAFastBufferPoolProperty pool_props =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(properties != NULL)

    *self = def_self;
    
    self->base.intf = &NANO_XRCE_gv_AgentInterfaceApi;

    NANO_CHECK_RC(
        NANO_XRCE_AgentProperties_copy(&self->props, properties),
        NANO_LOG_ERROR_MSG("FAILED to copy properties"));

    REDAInlineList_init(&self->transports);
    
    pool_props.multiThreadedAccess = 0;
    self->transports_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_AgentTransportRecord, &pool_props);
    if (self->transports_pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate transports pool")
        goto done;
    }

    /* The following pools are created w/multi-threaded access enabled */
    pool_props.multiThreadedAccess = 1;
    self->clients_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_ProxyClient, &pool_props);
    if (self->clients_pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate clients pool")
        goto done;
    }

    self->pool_storage_stream =
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_StreamStorageRecord, &pool_props);
    if (self->pool_storage_stream == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate stream storage pool")
        goto done;
    }

    self->pool_storage_session =
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_SessionStorageRecord, &pool_props);
    if (self->pool_storage_session == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate session storage pool")
        goto done;
    }

    self->reads_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_ReaderState, &pool_props);
    if (self->reads_pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate reads pool")
        goto done;
    }

    self->service_requests_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_ServiceRequestState, &pool_props);
    if (self->service_requests_pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate service requests pool")
        goto done;
    }

    self->mappings_mutex = RTIOsapiSemaphore_new(
                                RTI_OSAPI_SEMAPHORE_KIND_MUTEX, NULL);
    if (self->mappings_mutex == NULL)
    {
        goto done;
    }

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        /* TODO finalize created resources */
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}


void
NANO_XRCE_Agent_finalize(NANO_XRCE_Agent *const self)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_AgentTransportRecord *transport_rec = NULL,
                                   *prev_rec = NULL;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    /* Delete all registered transports */
    transport_rec =
        (NANO_XRCE_AgentTransportRecord*)
            REDAInlineList_getFirst(&self->transports);

    while (transport_rec != NULL)
    {
        // rc = NANO_XRCE_AgentTransport_close(transport_rec->transport);
        // if (NANO_RETCODE_OK != rc)
        // {
        //     NANO_LOG_ERROR("FAILED to close transport",
        //         NANO_LOG_PTR("agent", self)
        //         NANO_LOG_PTR("transport",transport_rec->transport)
        //         NANO_LOG_RC(rc))
        //     continue;
        // }
        NANO_XRCE_AgentTransport_finalize(transport_rec->transport);

        prev_rec = transport_rec;
        transport_rec =
            (NANO_XRCE_AgentTransportRecord*)
                REDAInlineListNode_getNext(&transport_rec->node);

        REDAFastBufferPool_returnBuffer(self->transports_pool, prev_rec);
    }
    REDAInlineList_removeAllNodesEA(&self->transports);

    if (self->props.client_mappings != NULL)
    {
        RTIOsapiHeap_freeArray(self->props.client_mappings);
    }

    REDAFastBufferPool_delete(self->transports_pool);
    self->transports_pool = NULL;

    REDAFastBufferPool_delete(self->clients_pool);
    self->clients_pool = NULL;

    REDAFastBufferPool_delete(self->pool_storage_stream);
    self->pool_storage_stream = NULL;

    REDAFastBufferPool_delete(self->pool_storage_session);
    self->pool_storage_session = NULL;

    REDAFastBufferPool_delete(self->reads_pool);
    self->reads_pool = NULL;

    RTIOsapiSemaphore_delete(self->mappings_mutex);
    self->mappings_mutex = NULL;

    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

NANO_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_deserialize_client_repr(
    NANO_XRCE_Agent *const self,
    const NANO_XRCE_ClientKey client_key,
    const NANO_XRCE_SessionId session_id,
    const NANO_MessageBuffer *const msg,
    const NANO_usize msg_len,
    NANO_XRCE_ClientRepresentation *const client_repr,
    NANO_bool *const key_changed)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    const NANO_u8 *data_ptr = NULL;
    NANO_usize data_len = 0,
               dser_len = 0,
               min_msg_size = 0;
    NANO_XRCE_SubmessageHeader submsg_hdr =
        NANO_XRCE_SUBMESSAGEHEADER_INITIALIZER;
    NANO_XRCE_ClientKey invalid_key = NANO_XRCE_CLIENTKEY_INVALID;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(self);
    
    NANO_PCOND(self != NULL)

    *key_changed = NANO_BOOL_FALSE;

    min_msg_size = 
        NANO_XRCE_SessionId_header_size(session_id) +
        NANO_XRCE_CREATECLIENTPAYLOAD_HEADER_SERIALIZED_SIZE_MIN;

    if (msg_len < min_msg_size)
    {
        /* message is too short */
        goto done;
    }
    
    /* Deserialize ClientRepresentation from msg */
    data_ptr = NANO_MessageBuffer_data_ptr(msg) +
                NANO_XRCE_INLINEHEADERBUFFER_SUBMSG_OFFSET(session_id);
    data_len = msg_len -
                NANO_XRCE_INLINEHEADERBUFFER_SUBMSG_OFFSET(session_id);

#define dptr        (data_ptr + dser_len)

    submsg_hdr.id = *((NANO_XRCE_SubmessageId*)dptr);
    dser_len += sizeof(NANO_XRCE_SubmessageId);

    submsg_hdr.flags = *((NANO_XRCE_SubmessageFlags*)dptr);
    dser_len += sizeof(NANO_XRCE_SubmessageFlags);

    submsg_hdr.length = NANO_u16_from_ptr_le(dptr);
    dser_len += sizeof(NANO_u16);

    if (data_len - submsg_hdr.length > dser_len)
    {
        /* Submessage says it's longer than the actual data */
        NANO_LOG_WARNING(
            "submessage length exceeds message size",
            NANO_LOG_U16("remaining_msg", data_len - dser_len)
            NANO_LOG_U16("submsg_len", submsg_hdr.length))
        goto done;
    }

    switch (submsg_hdr.id)
    {
    case NANO_XRCE_SUBMESSAGEID_CREATE_CLIENT:
    {
        NANO_CDR_Stream cdr_stream = NANO_CDR_STREAM_INITIALIZER;

        NANO_CHECK_RC(
            NANO_CDR_Stream_initialize(
                &cdr_stream, 
                (NANO_u8*)dptr,
                submsg_hdr.length,
                NANO_XRCE_SubmessageFlags_is_little_endian(submsg_hdr.flags)?
                    NANO_CDR_ENDIANNESS_LITTLE : NANO_CDR_ENDIANNESS_BIG,
                NANO_BOOL_FALSE /* owned */),
            NANO_LOG_ERROR_MSG("FAILED to initialize CDRStream"));
        
        NANO_CHECK_RC(
            NANO_XRCE_ClientRepresentation_deserialize_cdr(
                client_repr, &cdr_stream),
            NANO_LOG_ERROR("FAILED to deserialize ClientRepresentation",
                NANO_LOG_MBUF("msg", msg)));

        dser_len += NANO_CDR_Stream_offset(&cdr_stream);
        
        /* TODO check that (dser_len == 0): we can't check it now because
           the properties sequence is not deserialized at the moment. */
        break;
    }
    default:
    {
        NANO_LOG_ERROR(
            "expected CREATE_CLIENT, found invalid submsg",
            NANO_LOG_SUBMSGHDR("submsg", submsg_hdr))
        break;
    }
    }
    
    if (NANO_XRCE_ClientKey_compare(&client_key, &invalid_key) != 0 &&
        NANO_XRCE_ClientKey_compare(
            &client_key, &client_repr->client_key) != 0)
    {
        *key_changed = NANO_BOOL_TRUE;
    }

    if (!NANO_XRCE_Cookie_is_valid(&client_repr->xrce_cookie))
    {
        /* invalid cookie */
        goto done;
    }

    if (!NANO_XRCE_Version_is_compatible(&client_repr->xrce_version))
    {
        /* invalid version */
        goto done;
    }
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}


NANO_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_process_create_client(
    NANO_XRCE_Agent *const self,
    const NANO_XRCE_ClientKey client_key,
    const NANO_XRCE_SessionId session_id,
    const NANO_MessageBuffer *const msg,
    const NANO_usize msg_len,
    NANO_XRCE_AgentTransport *const src_transport,
    const NANO_XRCE_TransportLocator *const src_locator,
    const NANO_bool key_in_header,
    const NANO_bool missing_mapping)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ClientRepresentation client_repr =
        NANO_XRCE_CLIENTREPRESENTATION_INITIALIZER;
    NANO_bool unknown_key = NANO_BOOL_FALSE,
              key_changed = NANO_BOOL_FALSE;
    D2S2_ClientKey client_key_payload = D2S2_CLIENTKEY_INVALID;
    D2S2_ClientSessionKey session_key = D2S2_CLIENTSESSIONKEY_INITIALIZER;
    D2S2_ClientSessionProperties session_props =
        D2S2_CLIENTSESSIONPROPERTIES_INITIALIZER;
    NANO_XRCE_ProxyClient *client = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    unknown_key = !NANO_XRCE_ClientKey_is_valid(&client_key);
    /* Caller should have already validated the key if it was in the message */
    NANO_PCOND(!unknown_key || !key_in_header)
    UNUSED_ARG(unknown_key);

    NANO_CHECK_RC(
        NANO_XRCE_Agent_deserialize_client_repr(
            self,
            client_key,
            session_id,
            msg,
            msg_len,
            &client_repr,
            &key_changed),
        NANO_LOG_ERROR("FAILED to deserialize CREATE_CLIENT payload",
            NANO_LOG_MBUF("msg",msg)));

    if (!NANO_XRCE_ClientRepresentation_is_valid(&client_repr))
    {
        NANO_LOG_ERROR_MSG("INVALID client representation")
        goto done;
    }

    client_key_payload = NANO_XRCE_ClientKey_to_u32(&client_repr.client_key);

    if (key_changed && key_in_header)
    {
        NANO_LOG_ERROR("MISMATCHED client keys",
            NANO_LOG_H32("hdr.key", session_key.client)
            NANO_LOG_H32("payload.key", client_key_payload))
        goto done;
    }    

    session_key.client = client_key_payload;
    session_key.id = client_repr.session_id;

    if (!NANO_Time_is_infinite(&self->props.client_session_timeout))
    {
        session_props.timeout.sec =
            self->props.client_session_timeout.sec;
        session_props.timeout.nanosec =
            self->props.client_session_timeout.nanosec;
        NANO_LOG_INFO("SESSION timeout",
            NANO_LOG_I32("sec", self->props.client_session_timeout.sec)
            NANO_LOG_U32("nanosec", self->props.client_session_timeout.nanosec))
    }

    client = (NANO_XRCE_ProxyClient*)
            REDAFastBufferPool_getBuffer(self->clients_pool);
    if (client == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate ProxyClient");
        goto done;
    }

    if (NANO_RETCODE_OK !=
            NANO_XRCE_ProxyClient_initialize(
                client, &client_repr, self))
    {
        NANO_LOG_ERROR_MSG("FAILED to initialize proxy client")
        goto done;
    }

    client->cache_mapping = (missing_mapping || key_changed);

    NANO_CHECK_RC(
        NANO_XRCE_ProxyClientTransport_set_bind_entry(
            &client->transport, src_transport, src_locator),
        NANO_LOG_ERROR_MSG("FAILED to set proxy client transport's bind entry"));

    NANO_LOG_INFO("OPEN session on DDS Agent",
            NANO_LOG_H32("client_key",session_key.client)
            NANO_LOG_H8("session",session_key.id))


    if (DDS_RETCODE_OK != 
            D2S2_Agent_open_session(
                self->base.agent,
                &self->base,
                &session_key,
                &session_props,
                client))
    {
        goto done;
    }

    rc = NANO_RETCODE_OK;
    
done:
    NANO_XRCE_ClientRepresentation_finalize(&client_repr);
    if (NANO_RETCODE_OK != rc)
    {
        if (client != NULL)
        {
            NANO_XRCE_ProxyClient_finalize(client);
            REDAFastBufferPool_returnBuffer(self->clients_pool, client);
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_dispose_client_session(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_ProxyClientRequest *const request)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    if (client->disposed)
    {
        /* already disposed */
        rc = NANO_RETCODE_OK;
        goto done;
    }

    NANO_LOG_INFO("DISPOSING client session",
        NANO_LOG_KEY("key",client->session.key)
        NANO_LOG_SESSIONID("id",client->session.id)
        NANO_LOG_LOCATOR("locator",&client->transport.bind_entry.locator))
    
    client->disposed = NANO_BOOL_TRUE;
    client->dispose_request = request;

    if (client->dispose_request != NULL)
    {
        /* If the session is being disposed upon a client request (and not
           because it timed out or the agent is being disposed) then
           disable reply flag so upstream code will not send a reply
           (as it does for other DELETE messages). We will send a reply
           from the on_session_closed() callback */
        client->dispose_request->reply = NANO_BOOL_FALSE;
    }

    /* Dismiss all forward requests associated with the client */
    NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
        self, client, NULL, D2S2_ATTACHEDRESOURCEID_INVALID,
        NANO_BOOL_TRUE /* confirmed */);

    rc = NANO_RETCODE_OK;
    
done:
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_delete_client_session(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_UserStreamsStorage user_streams =
        NANO_XRCE_USERSTREAMSSTORAGE_INITIALIZER;
    NANO_XRCE_BestEffortStream *be_stream = NULL;
    NANO_XRCE_ReliableStream *rel_stream = NULL;
    NANO_usize i = 0;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    user_streams = *client->session.storage->streams;
    
    NANO_XRCE_ProxyClient_finalize(client);
    REDAFastBufferPool_returnBuffer(self->clients_pool, client);

#if 0
    for (i = 0; i < user_streams.user_streams_be_len; i++)
    {
        be_stream = user_streams.user_streams_be + i;
        
        REDAFastBufferPool_returnBuffer(
            self->pool_storage_stream,
            NANO_XRCE_StreamStorageRecord_from_storage(
                be_stream->base.storage));
    }

    for (i = 0; i < user_streams.user_streams_rel_len; i++)
    {
        rel_stream = user_streams.user_streams_rel + i;
        NANO_XRCE_ReliableStream_finalize(rel_stream);

        REDAFastBufferPool_returnBuffer(
            self->pool_storage_stream,
            NANO_XRCE_StreamStorageRecord_from_storage(
                rel_stream->base.storage));
    }
#endif
    
    rc = NANO_RETCODE_OK;
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_XRCE_Agent_release_reply_message(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    const NANO_XRCE_StreamId reply_stream_id,
    NANO_MessageBuffer *const payload)
{
    NANO_XRCE_Stream *reply_stream = NULL;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    reply_stream =
        NANO_XRCE_Agent_lookup_stream(self, client, reply_stream_id);
    NANO_PCOND(reply_stream != NULL)

    NANO_LOG_INFO("RELEASE reply message",
        NANO_LOG_PTR("stream", reply_stream)
        NANO_LOG_STREAMID("strea_id", reply_stream_id)
        NANO_LOG_MBUF("mbuf", payload))
    NANO_XRCE_Session_release_message(&client->session, reply_stream, payload);
    
    NANO_LOG_FN_EXIT
    return;
}

NANO_RetCode
NANO_XRCE_Agent_allocate_reply_message(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_Stream *reply_stream_in,
    const NANO_XRCE_StreamId reply_stream_id,
    const NANO_usize payload_size,
    const NANO_u8 *const user_payload,
    NANO_MessageBuffer **const payload_out,
    NANO_XRCE_Stream **const reply_stream_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_MessageBuffer *payload_msg = NULL;
    NANO_XRCE_Stream *reply_stream = reply_stream_in;
    NANO_MessageBufferType buffer_type =
        (user_payload != NULL)?
            NANO_XRCE_MESSAGETYPE_EXTERNAL_PAYLOAD :
            NANO_XRCE_MESSAGETYPE_INLINE_PAYLOAD;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    *payload_out = NULL;
    if (reply_stream_out != NULL)
    {
        *reply_stream_out = NULL;
    }

    if (reply_stream == NULL)
    {
        reply_stream =
            NANO_XRCE_Agent_lookup_stream(self, client, reply_stream_id);
        if (reply_stream == NULL)
        {
            NANO_LOG_ERROR("UNKNOWN reply stream",
                NANO_LOG_KEY("session.key", client->session.key)
                NANO_LOG_SESSIONID("session.id", client->session.id)
                NANO_LOG_STREAMID("stream", reply_stream_id))
            goto done;
        }
    }
    NANO_PCOND(reply_stream != NULL)
    NANO_PCOND(NANO_XRCE_Stream_id(reply_stream) == reply_stream_id);

    NANO_LOG_TRACE("ALLOCATE reply message",
        NANO_LOG_KEY("session.key", client->session.key)
        NANO_LOG_SESSIONID("session.id", client->session.id)
        NANO_LOG_STREAMID("stream", reply_stream_id)
        NANO_LOG_STR("buffer_type", NANO_MessageBufferType_to_str(buffer_type))
        NANO_LOG_USIZE("payload_size", payload_size)
        NANO_LOG_PTR("user_payload", user_payload))

    payload_msg = NANO_XRCE_Session_allocate_message(
                    &client->session,
                    reply_stream,
                    buffer_type,
                    payload_size,
                    user_payload);
                    
    if (payload_msg == NULL)
    {
        rc = NANO_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }

    *payload_out = payload_msg;
    if (reply_stream_out != NULL)
    {
        *reply_stream_out = reply_stream;
    }
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}


NANO_RetCode
NANO_XRCE_Agent_reply_to_connection(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Stream *reply_stream = NULL;
    NANO_MessageBuffer *payload_msg = NULL;
    NANO_XRCE_AgentRepresentation *agent_repr = NULL;
    NANO_XRCE_Cookie def_cookie = NANO_XRCE_COOKIE_DEFAULT;
    NANO_XRCE_Version def_version = NANO_XRCE_VERSION_DEFAULT;
    NANO_XRCE_VendorId def_vendor = NANO_XRCE_VENDORID_DEFAULT;
    NANO_u8 *ptr = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    NANO_CHECK_RC(
        NANO_XRCE_Agent_allocate_reply_message(
            self,
            client,
            NULL /* reply_stream */,
            NANO_XRCE_STREAMID_NONE,
            NANO_XRCE_AGENTREPRESENTATION_SERIALIZED_SIZE_MAX,
            NULL /* user_payload */,
            &payload_msg,
            &reply_stream),
        NANO_LOG_ERROR("FAILED to allocate reply message",
            NANO_LOG_USIZE("payload_size",
                NANO_XRCE_AGENTREPRESENTATION_SERIALIZED_SIZE_MAX)));
        
    if (payload_msg == NULL)
    {
        goto done;
    }

    agent_repr = (NANO_XRCE_AgentRepresentation*)
                    NANO_MessageBuffer_data_ptr(payload_msg);
    
    agent_repr->xrce_cookie = def_cookie;
    agent_repr->xrce_version = def_version;
    agent_repr->xrce_vendor_id = def_vendor;
    
    ptr = NANO_MessageBuffer_data_ptr(payload_msg) + 
          NANO_OSAPI_MEMBER_OFFSET(NANO_XRCE_AgentRepresentation, xrce_vendor_id) +
          sizeof(NANO_XRCE_VendorId);
    /* Never serialize properties */
    *((NANO_bool*)ptr) = NANO_BOOL_FALSE;

    NANO_CHECK_RC(
        NANO_XRCE_Session_send(
            &client->session, 
            reply_stream,
            NANO_XRCE_SUBMESSAGEID_STATUS_AGENT,
            NANO_XRCE_SUBMESSAGEFLAGS_DEFAULT,
            payload_msg,
            NULL),
        NANO_LOG_ERROR("failed to send STATUS_AGENT reply",
            NANO_LOG_RC(rc)));

    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}


NANO_RetCode
NANO_XRCE_Agent_reply_to_operation(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const req,
    NANO_RetCode op_rc)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_StatusPayload *status_payload = NULL;
    NANO_XRCE_Stream *reply_stream = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(req->reply)
    
    /* Replies are always sent on the same stream as the original message.
       For reliable streams, we pre-allocate the reply message, while for 
       best-effort ones we allocate it and send it in one context. */
    NANO_PCOND(
        !NANO_XRCE_StreamId_is_reliable(req->msg_hdr.stream_id) ||
        req->reply_mbuf != NULL)
    
    if (!NANO_XRCE_StreamId_is_reliable(req->msg_hdr.stream_id))
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_allocate_reply_message(
                self,
                req->client,
                NULL /* reply_stream */,
                req->msg_hdr.stream_id,
                NANO_XRCE_STATUSPAYLOAD_SERIALIZED_SIZE_MAX,
                NULL /* user_payload */,
                &req->reply_mbuf,
                &reply_stream),
            NANO_LOG_ERROR("FAILED to allocate reply message",
                NANO_LOG_USIZE("payload_size",
                    NANO_XRCE_STATUSPAYLOAD_SERIALIZED_SIZE_MAX)));
    }
    else
    {
        NANO_PCOND(req->reply_mbuf != NULL)

        reply_stream = 
            NANO_XRCE_Agent_lookup_stream(
                self, req->client, req->msg_hdr.stream_id);
        if (reply_stream == NULL)
        {
            goto done;
        }
    }

    /* serialize STATUS payload */
    status_payload = (NANO_XRCE_StatusPayload*)
                    NANO_MessageBuffer_data_ptr(req->reply_mbuf);
    
    status_payload->base.related_request.request_id = req->request;
    status_payload->base.related_request.object_id = req->object_id;
    status_payload->base.result.status =
        NANO_XRCE_StatusValue_from_rc(op_rc);
    status_payload->base.result.implementation_status = op_rc;

    OP_REPLY_LOG("SEND op RESULT",
        NANO_LOG_MSGHDR("msg", req->msg_hdr)
        NANO_LOG_SUBMSGHDR("op",req->submsg_hdr)
        NANO_LOG_REQID("req_id", req->request)
        NANO_LOG_OBJID("obj_id", req->object_id)
        NANO_LOG_RC(status_payload->base.result.implementation_status)
        NANO_LOG_H8("status",status_payload->base.result.status))

    NANO_CHECK_RC(
        NANO_XRCE_Session_send(
            &req->client->session, 
            reply_stream,
            NANO_XRCE_SUBMESSAGEID_STATUS,
            NANO_XRCE_SUBMESSAGEFLAGS_DEFAULT,
            req->reply_mbuf,
            NULL),
        /* send() consumes the message even on error,
            so don't try to reply again */
        req->reply_mbuf = NULL;
        req->reply = NANO_BOOL_FALSE;
        NANO_LOG_ERROR("failed to send STATUS reply",
            NANO_LOG_RC(rc)));
    req->reply_mbuf = NULL;
    req->reply = NANO_BOOL_FALSE;
    
    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (!NANO_XRCE_StreamId_is_reliable(req->msg_hdr.stream_id) &&
            req->reply_mbuf != NULL)
        {
            NANO_XRCE_Agent_release_reply_message(
                self, req->client, req->msg_hdr.stream_id, req->reply_mbuf);
            req->reply_mbuf = NULL;
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_receive_message(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_TransportLocator *const src,
    NANO_MessageBuffer *const msg,
    const NANO_usize msg_len,
    NANO_bool *const retained)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SessionId sid = 0;
    NANO_XRCE_ClientKey client_key = NANO_XRCE_CLIENTKEY_INVALID;
    D2S2_ClientSessionKey session_key = D2S2_CLIENTSESSIONKEY_INITIALIZER;
    NANO_bool msg_has_key = NANO_BOOL_FALSE,
              invalid_client_key = NANO_BOOL_FALSE,
              msg_has_session = NANO_BOOL_FALSE;
    NANO_usize min_msg_size = 0;
    const NANO_XRCE_ClientKey invalid_key = NANO_XRCE_CLIENTKEY_INVALID;
    NANO_XRCE_ClientLocatorMapping *mapping = NULL;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(msg != NULL)
    NANO_PCOND(msg_len >= sizeof(NANO_XRCE_MESSAGEHEADER_SERIALIZED_SIZE_MIN))
    
    *retained = NANO_BOOL_FALSE;

    NANO_LOG_TRACE("RECV", 
        NANO_LOG_PTR("agent",self)
        NANO_LOG_PTR("transport",transport)
        NANO_LOG_LOCATOR("src",src)
        NANO_LOG_USIZE("msg_len",msg_len)
        NANO_LOG_MBUF("msg", msg))
    
    sid = NANO_XRCE_InlineHeaderBuffer_session_id(msg);
    
    min_msg_size = NANO_XRCE_SessionId_header_size(sid);
    if (min_msg_size > msg_len)
    {
        NANO_LOG_ERROR("invalid message length",
            NANO_LOG_SESSIONID("session", sid)
            NANO_LOG_USIZE("min_msg_size", min_msg_size)
            NANO_LOG_USIZE("msg_len", msg_len))
        goto done;
    }

    msg_has_key = NANO_XRCE_SessionId_has_client_key(sid);

    if (msg_has_key)
    {

        client_key = *NANO_XRCE_InlineHeaderBuffer_clientkey_ptr(msg);
        
        invalid_client_key =
            (NANO_XRCE_ClientKey_compare(&client_key, &invalid_key) == 0);
        
        if (invalid_client_key)
        {
            NANO_LOG_ERROR("invalid client key",
                NANO_LOG_KEY("key", client_key))
            goto done;
        }
    }
    else
    {
        RTIOsapiSemaphore_take(self->mappings_mutex, RTI_NTP_TIME_INFINITE);
        NANO_CHECK_RC(
            NANO_XRCE_Agent_find_client_locator_mapping(self, src, &mapping),
            RTIOsapiSemaphore_give(self->mappings_mutex);
            NANO_LOG_ERROR("NO MAPPING found for locator",
                NANO_LOG_LOCATOR("src",src)));
        
        if (mapping == NULL && !self->props.auto_client_mapping)
        {
            NANO_LOG_ERROR_MSG("no client mapping found")
            goto done;
        }
        else if (mapping != NULL)
        {
            NANO_LOG_DEBUG("FOUND client mapping",
                NANO_LOG_LOCATOR("src",src)
                NANO_LOG_KEY("client",mapping->client_key)
                NANO_LOG_LOCATOR("locator",&mapping->locator)
                NANO_LOG_PTR("mapping",mapping))
            client_key = mapping->client_key;
        }
        RTIOsapiSemaphore_give(self->mappings_mutex);
    }

    msg_has_session = 
        !(sid == NANO_XRCE_SESSIONID_NONE_WITH_CLIENT ||
            sid == NANO_XRCE_SESSIONID_NONE_WITHOUT_CLIENT);
    
    if (!msg_has_session)
    {
        /* an invalid session id can be sent by clients only when 
         * trying to establish a new connection with a CREATE_CLIENT
         * message. */
        NANO_LOG_DEBUG("ASSERT client session",
            NANO_LOG_PTR("transport",transport)
            NANO_LOG_LOCATOR("src", src)
            NANO_LOG_KEY("key", client_key)
            NANO_LOG_SESSIONID("session",sid))

        NANO_CHECK_RC(
            NANO_XRCE_Agent_process_create_client(
                self,
                client_key,
                sid,
                msg,
                msg_len,
                transport,
                src,
                msg_has_key,
                (mapping == NULL)),
            NANO_LOG_ERROR("FAILED to process CREATE_CLIENT",
                NANO_LOG_KEY("client",client_key)
                NANO_LOG_SESSIONID("session",sid)
                NANO_LOG_MBUF("msg",msg)));
    }
    else
    {
        if (!msg_has_key && mapping == NULL)
        {
            /* we received a message without a client key but with a
              session id, and we didn't find a locator->key mapping, so
              we don't know who to deliver the message.
              
              This will need to be changed once we support Discovery
              messages, which do not belong to any specific client. */
            NANO_LOG_TRACE("UNDELIVERABLE message",
                NANO_LOG_PTR("transport",transport)
                NANO_LOG_LOCATOR("src", src)
                NANO_LOG_MBUF("msg",msg))
            goto done;
        }

        NANO_LOG_DEBUG("RECEIVE msg on client session",
            NANO_LOG_PTR("transport",transport)
            NANO_LOG_LOCATOR("src", src)
            NANO_LOG_KEY("key", client_key)
            NANO_LOG_SESSIONID("session",sid))
        
        session_key.client = NANO_XRCE_ClientKey_to_u32(&client_key);
        session_key.id = sid;

        if (DDS_RETCODE_OK !=
                D2S2_Agent_receive_message(
                    self->base.agent,
                    &self->base,
                    &session_key,
                    msg))
        {
            NANO_LOG_ERROR_MSG("D2S2_Agent_receive_message FAILED")
            goto done;
        }
    }
    
    
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_PRIVATE
void
NANO_XRCE_Agent_on_message_received(
    NANO_XRCE_AgentTransportListener *const listener,
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_TransportLocator *const src,
    NANO_MessageBuffer *const msg,
    const NANO_usize msg_len,
    NANO_bool *const retained)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_XRCE_Agent *self = (NANO_XRCE_Agent*)listener->user_data;

    NANO_LOG_FN_ENTRY

    if (NANO_RETCODE_OK != 
            NANO_XRCE_Agent_receive_message(
                self, transport, src, msg, msg_len ,retained))
    {
        NANO_LOG_WARNING("FAILED to receive message",
            NANO_LOG_MBUF("msg",msg))
        goto done;
    }

    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

NANO_RetCode
NANO_XRCE_Agent_register_transport(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_AgentTransportProperties *const properties)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_AgentTransportRecord *rec = NULL;
    NANO_XRCE_AgentTransportListener listener =
        NANO_XRCE_AGENTTRANSPORTLISTENER_INITIALIZER;
    NANO_bool transport_initd = NANO_BOOL_FALSE;
    NANO_XRCE_AgentTransportImplProperties impl_props =
        NANO_XRCE_AGENTTRANSPORTIMPLPROPERTIES_INITIALIZER;
    // struct REDAWorkerFactory *worker_factory = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(transport != NULL)
    
    listener.on_message_received = NANO_XRCE_Agent_on_message_received;
    listener.user_data = self;

    impl_props.agent = self;

    NANO_CHECK_RC(
        NANO_XRCE_AgentTransport_initialize(
            transport, &listener, properties, &impl_props),
        NANO_LOG_ERROR("FAILED to initialize transport",
            NANO_LOG_PTR("agent",self)
            NANO_LOG_PTR("transport",transport)
            NANO_LOG_PTR("properties",properties)));
    transport_initd = NANO_BOOL_TRUE;

    rec = REDAFastBufferPool_getBuffer(self->transports_pool);
    if (rec == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate transport entry")
        goto done;
    }
    REDAInlineListNode_init(&rec->node);

    rec->transport = transport;
    rec->ea = impl_props.transport_ea;

    REDAInlineList_addNodeToBackEA(&self->transports, &rec->node);

    NANO_LOG_INFO("transport REGISTERED",
        NANO_LOG_PTR("agent",self)
        NANO_LOG_PTR("transport",rec->transport))
    
    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (transport_initd)
        {
            NANO_XRCE_AgentTransport_finalize(transport);
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_enable(NANO_XRCE_Agent *const self)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_AgentTransportRecord *transport_rec = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    if (NANO_XRCE_Agent_flags(self, NANO_XRCE_AGENTFLAGS_ENABLED))
    {
        NANO_LOG_ERROR("agent ALREADY enabled",
            NANO_LOG_PTR("agent",self))
        goto done;
    }

    transport_rec = 
        (NANO_XRCE_AgentTransportRecord*) 
            REDAInlineList_getFirst(&self->transports);
    
    if (transport_rec == NULL)
    {
        NANO_LOG_ERROR("no transport registered on agent",
            NANO_LOG_PTR("agent",self))
        goto done;
    }

    while (transport_rec != NULL)
    {
        NANO_PCOND(transport_rec->transport != NULL)

        NANO_LOG_INFO("ENABLING agent transport",
            NANO_LOG_PTR("agent",self)
            NANO_LOG_PTR("transport",transport_rec->transport))

        NANO_CHECK_RC(
            NANO_XRCE_AgentTransport_listen(transport_rec->transport),
            NANO_LOG_ERROR("FAILED to enable transport",
                NANO_LOG_PTR("agent",self)
                NANO_LOG_PTR("transport",transport_rec->transport)));
        
        transport_rec =
            (NANO_XRCE_AgentTransportRecord*)
                REDAInlineListNode_getNext(&transport_rec->node);
    }

    NANO_XRCE_Agent_flags_set(self, NANO_XRCE_AGENTFLAGS_ENABLED)
    
    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        transport_rec = 
            (NANO_XRCE_AgentTransportRecord*) 
                REDAInlineList_getFirst(&self->transports);
        while (transport_rec != NULL)
        {
            NANO_XRCE_AgentTransport_close(transport_rec->transport);
            transport_rec =
                (NANO_XRCE_AgentTransportRecord*)
                    REDAInlineListNode_getNext(&transport_rec->node);
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_disable(NANO_XRCE_Agent *const self)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_AgentTransportRecord *transport_rec = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    if (!NANO_XRCE_Agent_flags(self, NANO_XRCE_AGENTFLAGS_ENABLED))
    {
        NANO_LOG_ERROR("agent NOT enabled",
            NANO_LOG_PTR("agent",self))
        goto done;
    }

    NANO_XRCE_Agent_flags_unset(self, NANO_XRCE_AGENTFLAGS_ENABLED)

    transport_rec = 
            (NANO_XRCE_AgentTransportRecord*) 
                REDAInlineList_getFirst(&self->transports);
    while (transport_rec != NULL)
    {
        rc = NANO_XRCE_AgentTransport_close(transport_rec->transport);
        if (NANO_RETCODE_OK != rc)
        {
            NANO_LOG_ERROR("FAILED to close transport",
                NANO_LOG_PTR("agent", self)
                NANO_LOG_PTR("transport",transport_rec->transport)
                NANO_LOG_RC(rc))
            continue;
        }
        transport_rec =
            (NANO_XRCE_AgentTransportRecord*)
                REDAInlineListNode_getNext(&transport_rec->node);
    }
    
    rc = NANO_RETCODE_OK;
    
done:
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

RTI_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_create_entity(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    const NANO_XRCE_ObjectId *const parent_obj_id,
    const NANO_XRCE_ObjectKind obj_kind,
    const D2S2_ResourceKind obj_kind_dds,
    const NANO_XRCE_ObjectKind parent_kind,
    const D2S2_ResourceKind parent_kind_dds,
    const NANO_XRCE_RepresentationFormat repr_fmt,
    const char *const repr_ref,
    const char *const repr_xml,
    const NANO_XRCE_DomainId *const domain_id)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    D2S2_ResourceRepresentation res_repr =
        D2S2_RESOURCEREPRESENTATION_INITIALIZER;
    D2S2_AttachedResourceId ares_id = 0,
                            parent_id = 0;
    D2S2_ResourceProperties properties = D2S2_RESOURCEPROPERTIES_INITIALIZER;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(parent_kind_dds);
    
    NANO_PCOND(self != NULL)

    ares_id = NANO_XRCE_ObjectId_to_u16(&request->object_id);

    if (NANO_XRCE_ObjectId_kind(&request->object_id) != obj_kind)
    {
        NANO_LOG_ERROR("UNEXPECTED object kind",
            NANO_LOG_OBJID("obj_id",request->object_id)
            NANO_LOG_H8("type",NANO_XRCE_ObjectId_kind(&request->object_id))
            NANO_LOG_H8("expected",obj_kind))
        goto done;
    }

    if (parent_obj_id != NULL)
    {
        if (NANO_XRCE_ObjectId_kind(parent_obj_id) == parent_kind)
        {
#if 0
            NANO_LOG_ERROR("UNEXPECTED parent kind",
                NANO_LOG_OBJID("obj_id",*parent_obj_id)
                NANO_LOG_H8("type",NANO_XRCE_ObjectId_kind(parent_obj_id))
                NANO_LOG_H8("expected", parent_kind))
            goto done;
#endif
            parent_id = NANO_XRCE_ObjectId_to_u16(parent_obj_id);
        }

    }
    
    switch (repr_fmt)
    {
    case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
    {
        if (!D2S2_ResourceRepresentation_initialize_ref(&res_repr, repr_ref))
        {
            NANO_LOG_ERROR("FAILED to initialize REF representation",
                NANO_LOG_STR("str",repr_ref))
            goto done;
        }
        break;
    }
    case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
    {
        if (!D2S2_ResourceRepresentation_initialize_xml(&res_repr, repr_xml))
        {
            NANO_LOG_ERROR("FAILED to initialize XML representation",
                NANO_LOG_STR("str",repr_xml))
            goto done;
        }
        break;
    }
    default:
    {
        NANO_LOG_ERROR("UNSUPPORTED object representation",
            NANO_LOG_H8("format",repr_fmt))
        goto done;
    }
    }

    /* parse properties from flags */
    properties.replace =
        NANO_XRCE_SubmessageFlags_CREATE_replace(request->submsg_hdr.flags);
    properties.reuse =
        NANO_XRCE_SubmessageFlags_CREATE_reuse(request->submsg_hdr.flags);
    if (domain_id != NULL)
    {
        properties.domain_id = *domain_id;
    }

    NANO_LOG_INFO("CREATING object",
        NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&request->client->session))
        NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&request->client->session))
        NANO_LOG_STREAMID("stream", request->msg_hdr.stream_id)
        NANO_LOG_REQID("req_id",request->request)
        NANO_LOG_OBJID("obj_id",request->object_id)
        NANO_LOG_STR("obj_kind",D2S2_ResourceKind_to_str(obj_kind_dds))
        NANO_LOG_H8("repr_fmt", repr_fmt)
        NANO_LOG_STR("repr_ref", repr_ref)
        NANO_LOG_STR("repr_xml", repr_xml))

    if (DDS_RETCODE_OK !=
            D2S2_Agent_create_resource(
                self->base.agent,
                &self->base,
                request->client->dds_session,
                ares_id,
                obj_kind_dds,
                &res_repr,
                parent_id /* parent */,
                &properties,
                request /* request data */))
    {
        NANO_LOG_ERROR_MSG("FAILED to create resource on agent")
        goto done;
    }
    
    rc = NANO_RETCODE_OK;
    
done:
    
    D2S2_ResourceRepresentation_finalize(&res_repr);

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_create_participant(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_DomainParticipantRepresentation *const obj_repr);

#define NANO_XRCE_Agent_create_participant(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            NULL,\
            NANO_XRCE_OBJK_PARTICIPANT,\
            D2S2_RESOURCEKIND_DOMAINPARTICIPANT,\
            NANO_XRCE_OBJK_INVALID,\
            D2S2_RESOURCEKIND_UNKNOWN,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            &(or_)->domain_id)

NANO_RetCode
NANO_XRCE_Agent_create_topic(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_TopicRepresentation *const obj_repr);

#define NANO_XRCE_Agent_create_topic(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->participant_id,\
            NANO_XRCE_OBJK_TOPIC,\
            D2S2_RESOURCEKIND_TOPIC,\
            NANO_XRCE_OBJK_PARTICIPANT,\
            D2S2_RESOURCEKIND_DOMAINPARTICIPANT,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)

NANO_RetCode
NANO_XRCE_Agent_create_publisher(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_PublisherRepresentation *const obj_repr);

#if NANO_FEAT_ALL_BY_REF
#define NANO_XRCE_Agent_create_publisher(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_PUBLISHER,\
            D2S2_RESOURCEKIND_PUBLISHER,\
            NANO_XRCE_OBJK_PARTICIPANT,\
            D2S2_RESOURCEKIND_DOMAINPARTICIPANT,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)
#else
#define NANO_XRCE_Agent_create_publisher(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_PUBLISHER,\
            D2S2_RESOURCEKIND_PUBLISHER,\
            NANO_XRCE_OBJK_PARTICIPANT,\
            D2S2_RESOURCEKIND_DOMAINPARTICIPANT,\
            (or_)->base.repr.format,\
            NULL,\
            (or_)->base.repr.value.xml.value,\
            NULL)
#endif /* NANO_FEAT_ALL_BY_REF */

NANO_RetCode
NANO_XRCE_Agent_create_subscriber(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_SubscriberRepresentation *const obj_repr);

#if NANO_FEAT_ALL_BY_REF
#define NANO_XRCE_Agent_create_subscriber(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_SUBSCRIBER,\
            D2S2_RESOURCEKIND_SUBSCRIBER,\
            NANO_XRCE_OBJK_PARTICIPANT,\
            D2S2_RESOURCEKIND_DOMAINPARTICIPANT,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)
#else
#define NANO_XRCE_Agent_create_subscriber(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_SUBSCRIBER,\
            D2S2_RESOURCEKIND_SUBSCRIBER,\
            NANO_XRCE_OBJK_PARTICIPANT,\
            D2S2_RESOURCEKIND_DOMAINPARTICIPANT,\
            (or_)->base.repr.format,\
            NULL,\
            (or_)->base.repr.value.xml.value,\
            NULL)
#endif /* NANO_FEAT_ALL_BY_REF */


NANO_RetCode
NANO_XRCE_Agent_create_datawriter(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_DataWriterRepresentation *const obj_repr);

#if NANO_FEAT_ALL_BY_REF
#define NANO_XRCE_Agent_create_datawriter(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_DATAWRITER,\
            D2S2_RESOURCEKIND_DATAWRITER,\
            NANO_XRCE_OBJK_PUBLISHER,\
            D2S2_RESOURCEKIND_PUBLISHER,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)
#else
#define NANO_XRCE_Agent_create_datawriter(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_DATAWRITER,\
            D2S2_RESOURCEKIND_DATAWRITER,\
            NANO_XRCE_OBJK_PUBLISHER,\
            D2S2_RESOURCEKIND_PUBLISHER,\
            (or_)->base.repr.format,\
            NULL,\
            (or_)->base.repr.value.xml.value,\
            NULL)
#endif /* NANO_FEAT_ALL_BY_REF */


NANO_RetCode
NANO_XRCE_Agent_create_datareader(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_DataReaderRepresentation *const obj_repr);

#if NANO_FEAT_ALL_BY_REF
#define NANO_XRCE_Agent_create_datareader(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_DATAREADER,\
            D2S2_RESOURCEKIND_DATAREADER,\
            NANO_XRCE_OBJK_SUBSCRIBER,\
            D2S2_RESOURCEKIND_SUBSCRIBER,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)

#else
#define NANO_XRCE_Agent_create_datareader(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            &(or_)->parent_id,\
            NANO_XRCE_OBJK_DATAREADER,\
            D2S2_RESOURCEKIND_DATAREADER,\
            NANO_XRCE_OBJK_SUBSCRIBER,\
            D2S2_RESOURCEKIND_SUBSCRIBER,\
            (or_)->base.repr.format,\
            NULL,\
            (or_)->base.repr.value.xml.value,\
            NULL)
#endif /* NANO_FEAT_ALL_BY_REF */


NANO_RetCode
NANO_XRCE_Agent_create_type(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_TypeRepresentation *const obj_repr);

#define NANO_XRCE_Agent_create_type(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            NULL,\
            NANO_XRCE_OBJK_TYPE,\
            D2S2_RESOURCEKIND_TYPE,\
            NANO_XRCE_OBJK_INVALID,\
            D2S2_RESOURCEKIND_UNKNOWN,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)

NANO_RetCode
NANO_XRCE_Agent_create_domain(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_DomainRepresentation *const obj_repr);

#define NANO_XRCE_Agent_create_domain(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            NULL,\
            NANO_XRCE_OBJK_DOMAIN,\
            D2S2_RESOURCEKIND_DOMAIN,\
            NANO_XRCE_OBJK_INVALID,\
            D2S2_RESOURCEKIND_UNKNOWN,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)

NANO_RetCode
NANO_XRCE_Agent_create_qosprofile(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_QosProfileRepresentation *const obj_repr);

#define NANO_XRCE_Agent_create_qosprofile(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            NULL,\
            NANO_XRCE_OBJK_QOSPROFILE,\
            D2S2_RESOURCEKIND_QOSPROFILE,\
            NANO_XRCE_OBJK_INVALID,\
            D2S2_RESOURCEKIND_UNKNOWN,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)

NANO_RetCode
NANO_XRCE_Agent_create_application(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_ApplicationRepresentation *const obj_repr);

#define NANO_XRCE_Agent_create_application(s_,r_,or_)\
    NANO_XRCE_Agent_create_entity(\
            (s_),\
            (r_),\
            NULL,\
            NANO_XRCE_OBJK_APPLICATION,\
            D2S2_RESOURCEKIND_APPLICATION,\
            NANO_XRCE_OBJK_INVALID,\
            D2S2_RESOURCEKIND_UNKNOWN,\
            (or_)->base.repr.format,\
            (or_)->base.repr.value.ref.value,\
            (or_)->base.repr.value.xml.value,\
            NULL)

NANO_RetCode
NANO_XRCE_Agent_on_submsg_create(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_CreatePayload *const submsg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    switch (submsg->object_repr.kind)
    {
    case NANO_XRCE_OBJK_PARTICIPANT:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_participant(
                self, request, &submsg->object_repr.value.participant),
            NANO_LOG_ERROR_MSG("FAILED to create DomainParticipant"));
        break;
    }
    case NANO_XRCE_OBJK_TOPIC:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_topic(
                self, request, &submsg->object_repr.value.topic),
            NANO_LOG_ERROR_MSG("FAILED to create Topic"));
        break;
    }
    case NANO_XRCE_OBJK_PUBLISHER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_publisher(
                self, request, &submsg->object_repr.value.publisher),
            NANO_LOG_ERROR_MSG("FAILED to create Publisher"));
        break;
    }
    case NANO_XRCE_OBJK_SUBSCRIBER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_subscriber(
                self, request, &submsg->object_repr.value.subscriber),
            NANO_LOG_ERROR_MSG("FAILED to create Subscriber"));
        break;
    }
    case NANO_XRCE_OBJK_DATAWRITER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_datawriter(
                self, request, &submsg->object_repr.value.data_writer),
            NANO_LOG_ERROR_MSG("FAILED to create DataWriter"));
        break;
    }
    case NANO_XRCE_OBJK_DATAREADER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_datareader(
                self, request, &submsg->object_repr.value.data_reader),
            NANO_LOG_ERROR_MSG("FAILED to create DataReader"));
        break;
    }
    case NANO_XRCE_OBJK_APPLICATION:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_application(
                self, request, &submsg->object_repr.value.application),
            NANO_LOG_ERROR_MSG("FAILED to create Application"));
        break;
    }
    case NANO_XRCE_OBJK_DOMAIN:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_domain(
                self, request, &submsg->object_repr.value.domain),
            NANO_LOG_ERROR_MSG("FAILED to create Domain"));
        break;
    }
    case NANO_XRCE_OBJK_QOSPROFILE:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_qosprofile(
                self, request, &submsg->object_repr.value.qos_profile),
            NANO_LOG_ERROR_MSG("FAILED to create QosProfile"));
        break;
    }
    case NANO_XRCE_OBJK_TYPE:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_create_type(
                self, request, &submsg->object_repr.value.type),
            NANO_LOG_ERROR_MSG("FAILED to create Type"));
        break;
    }
    default:
    {
        NANO_LOG_ERROR("UNEXPECTED object kind",
            NANO_LOG_H8("kind",submsg->object_repr.kind))
        goto done;
    }
    }

    rc = NANO_RETCODE_OK;
    
done:
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}


NANO_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_delete_entity(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    const NANO_XRCE_ObjectKind obj_kind)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ObjectKind id_kind = NANO_XRCE_OBJK_INVALID;
    D2S2_AttachedResourceId att_resource_id = 0;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    id_kind = NANO_XRCE_ObjectId_kind(&request->object_id);

    if (id_kind != obj_kind)
    {
        NANO_LOG_ERROR("object id has INVALID kind",
            NANO_LOG_OBJID("obj_id", request->object_id)
            NANO_LOG_H8("obj_id_kind",id_kind)
            NANO_LOG_H8("expected_kind",obj_kind))
        goto done;
    }

    att_resource_id = NANO_XRCE_ObjectId_to_u16(&request->object_id);

    if (id_kind == NANO_XRCE_OBJK_DATAREADER)
    {
        /* We must return any sample that might have been forwarded
           by this reader on some reliable stream and haven't been
           ACK'd yet. */
        NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
            self, request->client, NULL, att_resource_id, NANO_BOOL_FALSE /* confirmed */);
    }

    if (DDS_RETCODE_OK !=
            D2S2_Agent_delete_resource(
                self->base.agent,
                &self->base,
                request->client->dds_session,
                att_resource_id,
                request))
    {
        NANO_LOG_ERROR_MSG("FAILED to delete resource")
        goto done;
    }
    
    NANO_LOG_INFO("DELETED object",
        NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&request->client->session))
        NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&request->client->session))
        NANO_LOG_OBJID("obj_id",request->object_id)
        NANO_LOG_H8("type", id_kind))
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

#define NANO_XRCE_Agent_delete_participant(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_PARTICIPANT)

#define NANO_XRCE_Agent_delete_topic(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_TOPIC)

#define NANO_XRCE_Agent_delete_publisher(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_PUBLISHER)

#define NANO_XRCE_Agent_delete_subscriber(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_SUBSCRIBER)

#define NANO_XRCE_Agent_delete_datawriter(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_DATAWRITER)

#define NANO_XRCE_Agent_delete_datareader(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_DATAREADER)

#define NANO_XRCE_Agent_delete_type(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_TYPE)

#define NANO_XRCE_Agent_delete_domain(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_DOMAIN)

#define NANO_XRCE_Agent_delete_qosprofile(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_QOSPROFILE)

#define NANO_XRCE_Agent_delete_application(s_,r_) \
    NANO_XRCE_Agent_delete_entity(\
        (s_), (r_), NANO_XRCE_OBJK_APPLICATION)

NANO_RetCode
NANO_XRCE_Agent_on_submsg_delete(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_DeletePayload *const submsg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ObjectKind tgt_kind = NANO_XRCE_OBJK_INVALID;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    tgt_kind = NANO_XRCE_ObjectId_kind(&submsg->base.object_id);

    switch (tgt_kind)
    {
    case NANO_XRCE_OBJK_PARTICIPANT:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_participant(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete DomainParticipant"));
        break;
    }
    case NANO_XRCE_OBJK_TOPIC:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_topic(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete Topic"));
        break;
    }
    case NANO_XRCE_OBJK_PUBLISHER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_publisher(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete Publisher"));
        break;
    }
    case NANO_XRCE_OBJK_SUBSCRIBER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_subscriber(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete Subscriber"));
        break;
    }
    case NANO_XRCE_OBJK_DATAWRITER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_datawriter(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete DataWriter"));
        break;
    }
    case NANO_XRCE_OBJK_DATAREADER:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_datareader(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete DataReader"));
        break;
    }
    case NANO_XRCE_OBJK_APPLICATION:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_application(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete Application"));
        break;
    }
    case NANO_XRCE_OBJK_DOMAIN:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_domain(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete Domain"));
        break;
    }
    case NANO_XRCE_OBJK_QOSPROFILE:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_qosprofile(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete QosProfile"));
        break;
    }
    case NANO_XRCE_OBJK_TYPE:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_delete_type(self, request),
            NANO_LOG_ERROR_MSG("FAILED to delete Type"));
        break;
    }
    case NANO_XRCE_OBJK_CLIENT:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_dispose_client_session(
                self, request->client, request),
            NANO_LOG_ERROR_MSG("FAILED to dispose client"));

        break;
    }
    default:
    {
        NANO_LOG_ERROR("UNEXPECTED object kind",
            NANO_LOG_H8("kind",tgt_kind))
        goto done;
    }
    }
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

RTI_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_write_data(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_WriteRequest *const request,
    const NANO_XRCE_SampleData *const data)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    D2S2_AttachedResourceId writer_id = D2S2_ATTACHEDRESOURCEID_INVALID;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    request->data_repr.fmt = D2S2_DATAREPRESENTATIONFORMAT_XCDR;
    request->data_repr.value.xcdr.buffer.data = data->serialized_data.value;
    request->data_repr.value.xcdr.buffer.data_len = data->serialized_data.value_len;
    request->data_repr.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_1;
    request->data_repr.value.xcdr.little_endian = 
        NANO_XRCE_SubmessageFlags_is_little_endian(request->req->submsg_hdr.flags);

    writer_id = NANO_XRCE_ObjectId_to_u16(&request->req->object_id);

    if (DDS_RETCODE_OK !=
            D2S2_Agent_write(
                self->base.agent,
                &self->base,
                request->req->client->dds_session,
                writer_id,
                &request->data_repr,
                request))
    {
        NANO_LOG_ERROR("FAILED to write data on DDS Agent",
            NANO_LOG_BYTES("buffer",
                request->data_repr.value.xcdr.buffer.data,
                request->data_repr.value.xcdr.buffer.data_len))
        goto done;
    }
    
    NANO_LOG_TRACE_FN("DATA written",
        NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&request->req->client->session))
        NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&request->req->client->session))
        NANO_LOG_REQID("req_id",request->req->request)
        NANO_LOG_BYTES("data",
            request->data_repr.value.xcdr.buffer.data,
            request->data_repr.value.xcdr.buffer.data_len))

    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

RTI_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_write_data_seq(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_WriteRequest *const request,
    const NANO_XRCE_SampleDataSeq *const data_seq)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY


    UNUSED_ARG(self);
    UNUSED_ARG(data_seq);
    UNUSED_ARG(request);
    
    NANO_PCOND(self != NULL)
    
    /* body */
    
    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

RTI_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_write_sample(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_WriteRequest *const request,
    const NANO_XRCE_Sample *const sample)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(sample);

    UNUSED_ARG(request);
    UNUSED_ARG(self);
    
    NANO_PCOND(self != NULL)
    
    /* body */
    
    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

RTI_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_write_sample_seq(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_WriteRequest *const request,
    const NANO_XRCE_SampleSeq *const sample_seq)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(self);
    UNUSED_ARG(request);
    UNUSED_ARG(sample_seq);
    
    NANO_PCOND(self != NULL)
    
    /* body */
    
    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

RTI_PRIVATE
NANO_RetCode
NANO_XRCE_Agent_write_packed_samples(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_WriteRequest *const request,
    const NANO_XRCE_PackedSamples *const packed_samples)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(packed_samples);
    UNUSED_ARG(request);
    UNUSED_ARG(self);

    NANO_PCOND(self != NULL)
    
    /* body */
    
    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_on_submsg_write(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_WriteDataPayload *const submsg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    D2S2_AttachedResourceId ares_id = 0;
    NANO_XRCE_WriteRequest *write_req = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    if (NANO_XRCE_ObjectId_kind(&submsg->base.object_id) !=
            NANO_XRCE_OBJK_DATAWRITER)
    {
        goto done;
    }

    ares_id = NANO_XRCE_ObjectId_to_u16(&submsg->base.object_id);
    UNUSED_ARG(ares_id);
    
    write_req = (NANO_XRCE_WriteRequest*)
                    REDAFastBufferPool_getBuffer(request->client->writes_pool);
    if (write_req == NULL)
    {
        goto done;
    }

    write_req->req = request;
    write_req->payload = submsg;

    NANO_LOG_DEBUG("WRITE_DATA REQUEST received",
        NANO_LOG_KEY("client",*NANO_XRCE_Session_key(&request->client->session))
        NANO_LOG_SESSIONID("session",*NANO_XRCE_Session_id(&request->client->session))
        NANO_LOG_OBJID("obj_id",request->object_id)
        NANO_LOG_PTR("client",request->client)
        NANO_LOG_PTR("write_req",write_req)
        NANO_LOG_PTR("req",write_req->req))
    
    switch (submsg->data_repr.format)
    {
    case NANO_XRCE_FORMAT_DATA:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_write_data(
                self,
                write_req,
                &submsg->data_repr.value.data),
            NANO_LOG_ERROR_MSG("FAILED to write DATA"));
        break;
    }
    case NANO_XRCE_FORMAT_SAMPLE:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_write_sample(
                self,
                write_req,
                &submsg->data_repr.value.sample),
            NANO_LOG_ERROR_MSG("FAILED to write SAMPLE"));
        break;
    }
    case NANO_XRCE_FORMAT_DATA_SEQ:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_write_data_seq(
                self,
                write_req,
                &submsg->data_repr.value.data_seq),
            NANO_LOG_ERROR_MSG("FAILED to write DATA_SEQ"));
        break;
    }
    case NANO_XRCE_FORMAT_SAMPLE_SEQ:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_write_sample_seq(
                self,
                write_req,
                &submsg->data_repr.value.sample_seq),
            NANO_LOG_ERROR_MSG("FAILED to write SAMPLE_SEQ"));
        break;
    }
    case NANO_XRCE_FORMAT_PACKED_SAMPLES:
    {
        NANO_CHECK_RC(
            NANO_XRCE_Agent_write_packed_samples(
                self,
                write_req,
                &submsg->data_repr.value.packed_samples),
            NANO_LOG_ERROR_MSG("FAILED to write PACKED_SAMPLES"));
        break;
    }
    default:
    {
        NANO_LOG_ERROR("UNKNOWN data format for WRITE",
            NANO_LOG_H8("format", submsg->data_repr.format))
        goto done;
    }
    }
    
    /* TODO what happens if the write has not completed yet? */
    NANO_XRCE_Agent_return_client_request(self, request);

    rc = NANO_RETCODE_OK;
    
done:
    if (write_req != NULL)
    {
        REDAFastBufferPool_returnBuffer(request->client->writes_pool, write_req);
    }
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

DDS_ReturnCode_t
NANO_XRCE_Agent_on_data_received(
    NANO_XRCE_Agent *const self,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const reader_data,
    const D2S2_ReceivedData *const data,
    DDS_Boolean *const retained_out,
    DDS_Boolean *const try_again_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ReaderState *const reader_state = (NANO_XRCE_ReaderState*)reader_data;
    NANO_XRCE_ProxyClient *client = (NANO_XRCE_ProxyClient*)session_data;
    NANO_XRCE_Session *xrce_session = &client->session;

    NANO_MessageBuffer *mbuf_data = NULL;
    NANO_XRCE_Stream *reply_stream = NULL;
    NANO_XRCE_SubmessageFlags msg_flags = NANO_XRCE_SUBMESSAGEFLAGS_INITIALIZER;
    const D2S2_XcdrData *xcdr_data = NULL;
    NANO_bool payload_le = NANO_BOOL_FALSE;
    NANO_bool fwd_req_added = NANO_BOOL_FALSE;
    NANO_XRCE_ForwardDataRequest *fwd_req = NULL;
    NANO_XRCE_ObjectId obj_id = NANO_XRCE_OBJECTID_INVALID;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(session);

    NANO_XRCE_ObjectId_from_u16(&obj_id, reader_id);

    NANO_LOG_DEBUG("data AVAILABLE",
        NANO_LOG_KEY("client",
            *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session",
            *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_OBJID("obj_id", obj_id))

    *retained_out = DDS_BOOLEAN_FALSE;
    *try_again_out = DDS_BOOLEAN_FALSE;

    if (!reader_state->forward)
    {
        NANO_LOG_WARNING_MSG("data IGNORED because read request is not active yet")
        rc = NANO_RETCODE_OK;
        goto done;
    }

    if (client->disposed)
    {
        NANO_LOG_WARNING_MSG("data IGNORED because client is DISPOSED")
        rc = NANO_RETCODE_OK;
        goto done;
    }

    if (data->data->fmt != D2S2_DATAREPRESENTATIONFORMAT_XCDR)
    {
        NANO_LOG_WARNING("UNSUPPORTED data format",
            NANO_LOG_H32("fmt", data->data->fmt))
        rc = NANO_RETCODE_OK;
        goto done;
    }

    xcdr_data = &data->data->value.xcdr;

    if (xcdr_data->buffer.data_len == 0)
    {
        NANO_LOG_WARNING_MSG("REFUSING to deliver empty message")
        rc = NANO_RETCODE_OK;
        goto done;
    }

    reply_stream =
        NANO_XRCE_Agent_lookup_stream(self, client, reader_state->stream_id);
    if (reply_stream == NULL)
    {
        NANO_LOG_ERROR("UNKNOWN stream for DATA forward",
            NANO_LOG_KEY("session.key",*(NANO_XRCE_Session_key(&client->session)))
            NANO_LOG_SESSIONID("session.id",*(NANO_XRCE_Session_id(&client->session)))
            NANO_LOG_OBJID("obj_id", obj_id)
            NANO_LOG_STREAMID("stream_id", reader_state->stream_id))
        goto done;
    }

    /* Allocate a forward data request to store state until we
        get notified by send_complete() (or discard it after send, if
        we are sending on best-effort) */
    
    switch (reader_state->read_fmt)
    {
    case NANO_XRCE_FORMAT_DATA:
    {
        NANO_XRCE_RelatedObjectRequest *data_reply = NULL;
        NANO_MessageBuffer *mbuf_header = NULL,
                           *mbuf_payload = NULL;
        
        NANO_CHECK_RC(
            NANO_XRCE_Agent_allocate_reply_message(
                self,
                client,
                reply_stream,
                reader_state->stream_id,
                NANO_XRCE_RELATEDOBJECTREQUEST_SERIALIZED_SIZE_MAX,
                NULL,
                &mbuf_header,
                NULL /* reply_stream_out */),
            NANO_LOG_TRACE("FAILED to allocate mbuf for DATA header",
                NANO_LOG_USIZE("payload_size",
                    xcdr_data->buffer.data_len))
            *try_again_out = DDS_BOOLEAN_TRUE;
            rc = NANO_RETCODE_OK;
            );
        
        NANO_PCOND(mbuf_header != NULL)

        /* Serialize DATA message header to buffer */
        data_reply = (NANO_XRCE_RelatedObjectRequest*)
                            NANO_MessageBuffer_data_ptr(mbuf_header);
        data_reply->request_id = reader_state->req->request;
        data_reply->object_id = reader_state->req->object_id;
         
        NANO_CHECK_RC(
            NANO_XRCE_Agent_allocate_reply_message(
                self,
                client,
                reply_stream,
                reader_state->stream_id,
                /* the data passed by the agent contains the XCDR header,
                   which XRCE doesn't like, so we remove it from the length
                   and adjust the data pointer accordingly */
                xcdr_data->buffer.data_len - sizeof(DDS_Long),
                xcdr_data->buffer.data + sizeof(DDS_Long),
                &mbuf_payload,
                NULL /* reply_stream_out */),
            NANO_LOG_TRACE("FAILED to allocate mbuf for DATA payload",
                NANO_LOG_USIZE("payload_size",
                    xcdr_data->buffer.data_len))
            NANO_XRCE_Agent_release_reply_message(
                self,
                client,
                reader_state->stream_id,
                mbuf_header);
            *try_again_out = DDS_BOOLEAN_TRUE;
            rc = NANO_RETCODE_OK;);
        
        NANO_PCOND(mbuf_payload != NULL)
        payload_le = xcdr_data->little_endian;

        NANO_MessageBuffer_append(mbuf_header, mbuf_payload);

        mbuf_data = mbuf_header;
        break;
    }
    default:
    {
        NANO_LOG_ERROR("UNSUPPORTED read format, cannot forward sample",
            NANO_LOG_H8("fmt", reader_state->read_fmt))
        goto done;
    }
    }

    while (mbuf_data != NULL)
    {
        NANO_MessageBuffer *nxt_send = NULL;

        fwd_req = (NANO_XRCE_ForwardDataRequest*)
            REDAFastBufferPool_getBuffer(client->forwards_pool);
        if (fwd_req == NULL)
        {
            NANO_LOG_ERROR_MSG("FAILED to allocate forward request")
            goto done;
        }

        NANO_XRCE_SubmessageFlags_DATA_set_format(
            &msg_flags,
            reader_state->read_fmt,
            payload_le);

        NANO_LOG_DEBUG("FORWARDING sample(s)",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(xrce_session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(xrce_session))
            NANO_LOG_STREAMID("stream", reader_state->stream_id)
            NANO_LOG_H8("format", reader_state->read_fmt)
            NANO_LOG_H8("flags.little_endian", NANO_XRCE_SubmessageFlags_is_little_endian(msg_flags))
            NANO_LOG_H8("flags.format", NANO_XRCE_SubmessageFlags_DATA_format(msg_flags))
            NANO_LOG_USIZE("payload",NANO_MessageBuffer_data_len(mbuf_data)))
        
        /* cache state so that we can look it up in on_send_complete */
        REDAInlineListNode_init(&fwd_req->node);
        fwd_req->rcvd_data = data;
        fwd_req->stream_id = reader_state->stream_id;
        fwd_req->reader_id = obj_id;

        REDAInlineList_addNodeToBackEA(&client->forwards, &fwd_req->node);
        fwd_req_added = NANO_BOOL_TRUE;

        nxt_send = NANO_MessageBuffer_next_msg(mbuf_data);
        NANO_MessageBuffer_unlink_msg(mbuf_data);

        NANO_CHECK_RC(
            NANO_XRCE_Session_send(
                xrce_session, 
                reply_stream,
                NANO_XRCE_SUBMESSAGEID_DATA,
                msg_flags,
                mbuf_data,
                &fwd_req->sn),
            mbuf_data = NULL;
            NANO_LOG_ERROR("failed to send DATA reply",
                NANO_LOG_RC(rc)));
        mbuf_data = NULL;
        
        if (!NANO_XRCE_StreamId_is_reliable(reader_state->stream_id))
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

        mbuf_data = nxt_send;
    }

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
        if (mbuf_data != NULL)
        {
            NANO_XRCE_Agent_release_reply_message(
                self,
                client,
                reader_state->stream_id,
                mbuf_data);
        }
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return (rc == NANO_RETCODE_OK)?DDS_RETCODE_OK:DDS_RETCODE_ERROR;
}


NANO_RetCode
NANO_XRCE_Agent_on_submsg_read(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_ReadDataPayload *const submsg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    D2S2_AttachedResourceId ares_id = 0;
    NANO_XRCE_ReaderState *reader_state = NULL;
    D2S2_ClientSessionKey session_key =
        D2S2_CLIENTSESSIONKEY_INITIALIZER;
    NANO_XRCE_ProxyClient *const client = request->client;
    void *resource_data = NULL;
    DDS_Boolean resource_exists = DDS_BOOLEAN_FALSE;
    NANO_XRCE_Stream *stream = NULL;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    if (NANO_XRCE_ObjectId_kind(&submsg->base.object_id) !=
            NANO_XRCE_OBJK_DATAREADER)
    {
        NANO_LOG_ERROR("READ target not a DATAREADER",
            NANO_LOG_REQID("req_id",submsg->base.request_id)
            NANO_LOG_OBJID("object",submsg->base.object_id))
        goto done;
    }

    if (submsg->read_spec.data_format != NANO_XRCE_FORMAT_DATA)
    {
        NANO_LOG_ERROR("UNSUPPORTED data format for READ",
            NANO_LOG_H8("fmt", submsg->read_spec.data_format))
        goto done;
    }

    ares_id = NANO_XRCE_ObjectId_to_u16(&submsg->base.object_id);

    session_key.client =
        NANO_XRCE_ClientKey_to_u32(NANO_XRCE_Session_key(&client->session));
    session_key.id = *NANO_XRCE_Session_id(&client->session);
    
    if (DDS_RETCODE_OK != 
            D2S2_Agent_lookup_resource(
                self->base.agent,
                &self->base,
                client->dds_session,
                ares_id,
                &resource_exists,
                &resource_data))
    {
        NANO_LOG_ERROR_MSG("FAILED to lookup READER data")
        goto done;
    }
    if (!resource_exists || resource_data == NULL)
    {
        NANO_LOG_ERROR("FAILED to lookup reader state",
            NANO_LOG_KEY("session.key",*NANO_XRCE_Session_key(&client->session))
            NANO_LOG_SESSIONID("session.id",*NANO_XRCE_Session_id(&client->session))
            NANO_LOG_OBJID("reader_id",submsg->base.object_id)
            NANO_LOG_H32("dds.session.client", session_key.client)
            NANO_LOG_H32("dds.session.id", session_key.id)
            NANO_LOG_H32("dds.resource_id", ares_id)
            NANO_LOG_BOOL("resource.exists", resource_exists)
            NANO_LOG_PTR("resource.data", resource_data))
        goto done;
    }

    reader_state = (NANO_XRCE_ReaderState*) resource_data;

    if (reader_state->req != NULL)
    {
        NANO_XRCE_Agent_return_client_request(self, reader_state->req);
    }

    reader_state->forward = NANO_BOOL_FALSE;
    reader_state->req = request;
    reader_state->read_fmt = submsg->read_spec.data_format;
    reader_state->stream_id = submsg->read_spec.preferred_stream_id;

    stream =
        NANO_XRCE_Agent_lookup_stream(self, client, reader_state->stream_id);
    if (stream == NULL)
    {
        NANO_LOG_ERROR("FAILED to lookup stream",
            NANO_LOG_KEY("session.key",*NANO_XRCE_Session_key(&client->session))
            NANO_LOG_SESSIONID("session.id",*NANO_XRCE_Session_id(&client->session))
            NANO_LOG_STREAMID("stream", reader_state->stream_id))
        goto done;
    }

    if (NANO_XRCE_Stream_is_reliable(stream))
    {
        /* Dismiss existing forward requests & empty stream send queue */
        NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
            self, client, stream, ares_id, NANO_BOOL_FALSE /* confirmed */);
    }

    if (submsg->read_spec.has_delivery_ctrl)
    {
        reader_state->read_spec_dds.max_samples =
            submsg->read_spec.delivery_ctrl.max_samples;
        reader_state->read_spec_dds.max_elapsed_time =
            submsg->read_spec.delivery_ctrl.max_elapsed_time;
        reader_state->read_spec_dds.max_bytes_per_second =
            submsg->read_spec.delivery_ctrl.max_bytes_per_second;
        reader_state->read_spec_dds.min_pace_period =
            submsg->read_spec.delivery_ctrl.min_pace_period;
        
    }

    if (submsg->read_spec.has_content_filter_expr)
    {
        reader_state->read_spec_dds.content_filter_expr =
            submsg->read_spec.content_filter_expr.value;
    }
    
    NANO_LOG_DEBUG("CREATING new SUBSCRIPTION",
        NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&client->session))
        NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&client->session))
        NANO_LOG_OBJID("reader", reader_state->req->object_id)
        NANO_LOG_REQID("req_id",reader_state->req->request)
        NANO_LOG_U32("max_samples",reader_state->read_spec_dds.max_samples)
        NANO_LOG_U32("max_elapsed_time",reader_state->read_spec_dds.max_elapsed_time)
        NANO_LOG_U32("min_pace_period",reader_state->read_spec_dds.min_pace_period)
        NANO_LOG_STR("content_filter",reader_state->read_spec_dds.content_filter_expr))

    if (DDS_RETCODE_OK !=
            D2S2_Agent_read(
                self->base.agent,
                &self->base,
                client->dds_session,
                ares_id,
                &reader_state->read_spec_dds,
                NULL))
    {
        NANO_LOG_ERROR_MSG("FAILED to create read on agent")
        goto done;
    }

    
    rc = NANO_RETCODE_OK;
    
done:

    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_on_submsg_getinfo(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_GetInfoPayload *const submsg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ObjectKind req_kind = NANO_XRCE_OBJK_INVALID;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(self);
    
    NANO_PCOND(self != NULL)

    req_kind = NANO_XRCE_ObjectId_kind(&submsg->base.object_id);

    switch (req_kind)
    {
    case NANO_XRCE_OBJK_CLIENT:
    {
        NANO_XRCE_ObjectId liveliness_id = NANO_XRCE_OBJECTID_INVALID;
        liveliness_id.value[0] = client->session.key.value[2];
        liveliness_id.value[1] = client->session.key.value[3];
        NANO_XRCE_ObjectId_combine(
            &liveliness_id, NANO_XRCE_OBJK_CLIENT, &liveliness_id);
        
        /* Possible assertion of liveliness */
        if (NANO_OSAPI_Memory_compare(
                submsg->base.request_id.value,
                client->session.key.value, 2) != 0 &&
            NANO_OSAPI_Memory_compare(
                submsg->base.object_id.value,
                liveliness_id.value, 2) != 0)
        {
            NANO_LOG_WARNING("MALFORMED liveliness assertion",
                NANO_LOG_KEY("key", client->session.key)
                NANO_LOG_SESSIONID("session", client->session.id))
        }
        else
        {
            NANO_LOG_DEBUG("liveliness ASSERTED",
                    NANO_LOG_KEY("key", client->session.key)
                    NANO_LOG_SESSIONID("session", client->session.id))
        }

        break;
    }
    default:
        /* Other requests not handled yet */
        break;
    }
    
    
    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_XRCE_Agent_on_submsg_servicereq(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClientRequest *const request,
    NANO_XRCE_ServiceRequestPayload *const submsg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ObjectKind req_kind = NANO_XRCE_OBJK_INVALID;
    D2S2_AttachedResourceId ares_id = 0;
    D2S2_ClientSessionKey session_key = D2S2_CLIENTSESSIONKEY_INITIALIZER;
    void *resource_data = NULL;
    NANO_XRCE_ExternalServiceResourceState *svc_res_state = NULL;
    DDS_Boolean resource_exists = DDS_BOOLEAN_FALSE;
    NANO_XRCE_ProxyClient *const client = request->client;
    D2S2_Buffer svc_query = D2S2_BUFFER_INITIALIZER;
    D2S2_Buffer svc_data = D2S2_BUFFER_INITIALIZER;
    D2S2_Buffer svc_metadata = D2S2_BUFFER_INITIALIZER;
    const NANO_u8 *payload_ptr = NULL;

    NANO_LOG_FN_ENTRY
    UNUSED_ARG(self);
    NANO_PCOND(self != NULL)

    req_kind = NANO_XRCE_ObjectId_kind(&submsg->base.object_id);
  
    if (req_kind != NANO_XRCE_OBJK_SERVICE_RESOURCE)
    {
        NANO_LOG_ERROR("SERVICE_REQUEST target not an SERVICE_RESOURCE",
            NANO_LOG_REQID("req_id",submsg->base.request_id)
            NANO_LOG_OBJID("object",submsg->base.object_id))
        goto done;
    }
    if (!submsg->has_payload &&
        (submsg->query_len > 0 || submsg->data_len > 0 || submsg->metadata_len > 0))
    {
        NANO_LOG_WARNING("inconsistent SERVICE_REQUEST",
            NANO_LOG_BOOL("has_payload", submsg->has_payload)
            NANO_LOG_U32("query_len", submsg->query_len)
            NANO_LOG_U32("data_len", submsg->data_len)
            NANO_LOG_U32("metadata_len", submsg->metadata_len))
        goto done;
    }

    ares_id = NANO_XRCE_ObjectId_to_u16(&submsg->base.object_id);

    session_key.client =
        NANO_XRCE_ClientKey_to_u32(NANO_XRCE_Session_key(&client->session));
    session_key.id = *NANO_XRCE_Session_id(&client->session);
    
    if (DDS_RETCODE_OK != 
            D2S2_Agent_lookup_resource(
                self->base.agent,
                &self->base,
                client->dds_session,
                ares_id,
                &resource_exists,
                &resource_data))
    {
        NANO_LOG_ERROR_MSG("FAILED to lookup SERVICE_RESOURCE resource")
        goto done;
    }
    if (!resource_exists || resource_data == NULL)
    {
        NANO_LOG_ERROR("FAILED to lookup SERVICE_RESOURCE resource",
            NANO_LOG_KEY("session.key",*NANO_XRCE_Session_key(&client->session))
            NANO_LOG_SESSIONID("session.id",*NANO_XRCE_Session_id(&client->session))
            NANO_LOG_OBJID("resource_id",submsg->base.object_id)
            NANO_LOG_H32("dds.session.client", session_key.client)
            NANO_LOG_H32("dds.session.id", session_key.id)
            NANO_LOG_H32("dds.resource_id", ares_id)
            NANO_LOG_BOOL("resource.exists", resource_exists)
            NANO_LOG_PTR("resource.data", resource_data))
        goto done;
    }

    svc_res_state = (NANO_XRCE_ExternalServiceResourceState*)resource_data;
    if (NULL != svc_res_state->req)
    {
        /* Dismiss existing forward requests & empty stream send queue */
        NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
            self, client, NULL, ares_id, NANO_BOOL_TRUE /* confirmed */);
        svc_res_state->req = NULL;
    }
    svc_res_state->forward = NANO_BOOL_TRUE;

    if (submsg->has_payload)
    {
        payload_ptr = NANO_XRCE_BinData_contiguous_buffer(&submsg->payload);
    }

    if (submsg->query_len > 0)
    {
        svc_query.data = (NANO_u8*)payload_ptr;
        svc_query.data_len = submsg->query_len;
        payload_ptr += svc_query.data_len;
    }

    if (submsg->data_len > 0)
    {
        svc_data.data = (NANO_u8*)payload_ptr;
        svc_data.data_len = submsg->data_len;
        payload_ptr += svc_data.data_len;
    }

    if (submsg->metadata_len > 0)
    {
        svc_metadata.data = (NANO_u8*)payload_ptr;
        svc_metadata.data_len = submsg->metadata_len;
        payload_ptr += svc_metadata.data_len;
    }

    if (DDS_RETCODE_OK !=
            D2S2_Agent_make_external_service_request(
                self->base.agent,
                &self->base,
                client->dds_session,
                ares_id,
                submsg->flags,
                &svc_query,
                &svc_data,
                &svc_metadata,
                NANO_XRCE_SubmessageFlags_SERVICEREQUEST_oneway(request->submsg_hdr.flags),
                request))
    {
        NANO_LOG_ERROR("FAILED to perform external service request",
            NANO_LOG_KEY("session.key",*NANO_XRCE_Session_key(&client->session))
            NANO_LOG_SESSIONID("session.id",*NANO_XRCE_Session_id(&client->session))
            NANO_LOG_OBJID("resource_id",submsg->base.object_id)
            NANO_LOG_H32("dds.session.client", session_key.client)
            NANO_LOG_H32("dds.session.id", session_key.id)
            NANO_LOG_H32("dds.resource_id", ares_id))
        goto done;
    }

    rc = NANO_RETCODE_OK;

done:
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_PRIVATE
void
NANO_XRCE_Agent_dismiss_client_fwd_data_requests_list(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_Stream *const req_stream,
    const D2S2_AttachedResourceId reader_id,
    const DDS_Boolean confirmed,
    struct REDAInlineList *const list)
{
    struct REDAInlineListNode *node = NULL;
    NANO_XRCE_ForwardDataRequest *fwd_req = NULL;
    NANO_XRCE_Stream *stream = NULL;
    const NANO_bool any_request = reader_id != D2S2_ATTACHEDRESOURCEID_INVALID;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    node = REDAInlineList_getFirst(list);
    while (node != NULL)
    {
        fwd_req = (NANO_XRCE_ForwardDataRequest*)node;
        
        if ((req_stream != NULL &&
                NANO_XRCE_Stream_id(req_stream) != fwd_req->stream_id) ||
            (!any_request && fwd_req->rcvd_data->reader_id != reader_id))
        {
            NANO_LOG_DEBUG("NOT dismissed FORWARD REQUEST",
                NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
                NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session))
                NANO_LOG_STREAMID("stream_id", fwd_req->stream_id)
                NANO_LOG_SN("sn", fwd_req->sn))
            node = REDAInlineListNode_getNext(node);
            continue;
        }
        
        stream =
            NANO_XRCE_Agent_lookup_stream(self, client, fwd_req->stream_id);
        NANO_PCOND(stream != NULL)


        NANO_LOG_INFO("dismiss FORWARD REQUEST",
            NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
            NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session))
            NANO_LOG_STREAMID("stream_id", fwd_req->stream_id)
            NANO_LOG_SN("sn", fwd_req->sn))

        if (!NANO_XRCE_Stream_is_reliable(stream))
        {
            NANO_LOG_ERROR("UNEXPECTED cached request on best-effort stream",
                NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
                NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session))
                NANO_LOG_STREAMID("stream_id", fwd_req->stream_id)
                NANO_LOG_SN("sn", fwd_req->sn))
        }
        if (0 == NANO_XRCE_Session_dismiss_send_queue_up_to(
                    &client->session,
                    (NANO_XRCE_ReliableStream*)stream,
                    &fwd_req->sn,
                    confirmed))
        {
            NANO_LOG_ERROR("FAILED to dismiss message from send queue",
                NANO_LOG_KEY("session.key", *NANO_XRCE_Session_key(&client->session))
                NANO_LOG_SESSIONID("session.id", *NANO_XRCE_Session_id(&client->session))
                NANO_LOG_STREAMID("stream_id", fwd_req->stream_id)
                NANO_LOG_SN("sn", fwd_req->sn))
            goto done;
        }
        
        node = REDAInlineList_getFirst(list);
    }

done:
    NANO_LOG_FN_EXIT
    return;
}


void
NANO_XRCE_Agent_dismiss_client_fwd_data_requests(
    NANO_XRCE_Agent *const self,
    NANO_XRCE_ProxyClient *const client,
    NANO_XRCE_Stream *const req_stream,
    const D2S2_AttachedResourceId reader_id,
    const DDS_Boolean confirmed)
{
    struct REDAInlineListNode *node = NULL;
    NANO_XRCE_ForwardDataRequest *fwd_req = NULL;
    NANO_XRCE_Stream *stream = NULL;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    NANO_XRCE_Agent_dismiss_client_fwd_data_requests_list(
        self, client, req_stream, reader_id, confirmed, &client->forwards);
    NANO_XRCE_Agent_dismiss_client_fwd_data_requests_list(
        self, client, req_stream, reader_id, confirmed, &client->forward_replies);

    NANO_LOG_FN_EXIT
    return;
}
