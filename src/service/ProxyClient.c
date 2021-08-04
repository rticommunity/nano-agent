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

#include "ProxyClient.h"
#include "StreamStorage.h"
#include "Agent.h"


NANO_RetCode
NANO_XRCE_ProxyClient_allocate_stream_storage(
    NANO_XRCE_ProxyClient *const self,
    NANO_XRCE_StreamStorageRecord **const record_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_StreamStorageRecord *record = NULL;
    const NANO_XRCE_StreamStorage def_storage =
            NANO_XRCE_STREAMSTORAGE_INITIALIZER;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(record_out != NULL)

    *record_out = NULL;

    record =
        (NANO_XRCE_StreamStorageRecord*)
            REDAFastBufferPool_getBuffer(
                self->agent->pool_storage_stream);
    if (record == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate stream storage")
        goto done;
    }
    record->storage = def_storage;
    REDAInlineListNode_init(&record->node);
    REDAInlineList_addNodeToBackEA(&self->stream_storage, &record->node);
    
    *record_out = record;

    rc = NANO_RETCODE_OK;
    
done:
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_XRCE_ProxyClient_release_stream_storage(
    NANO_XRCE_ProxyClient *const self,
    NANO_XRCE_StreamStorageRecord *const record)
{
    REDAInlineList_removeNodeEA(&self->stream_storage, &record->node);
    REDAFastBufferPool_returnBuffer(
        self->agent->pool_storage_stream, record);
}

RTI_PRIVATE
void
NANO_XRCE_CreatePayload_finalize(NANO_XRCE_CreatePayload *const self)
{
    switch (self->object_repr.kind)
    {
    case NANO_XRCE_OBJK_AGENT:
    {
        break;
    }
    case NANO_XRCE_OBJK_CLIENT:
    {
        break;
    }
    case NANO_XRCE_OBJK_PARTICIPANT:
    {
        switch (self->object_repr.value.participant.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.participant.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.participant.base.repr.value.ref.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_IN_BINARY:
        {
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_TOPIC:
    {
        switch (self->object_repr.value.topic.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.topic.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.topic.base.repr.value.ref.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_IN_BINARY:
        {
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_PUBLISHER:
    {
        switch (self->object_repr.value.publisher.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.publisher.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.publisher.base.repr.value.ref.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_IN_BINARY:
        {
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_SUBSCRIBER:
    {
        switch (self->object_repr.value.subscriber.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.subscriber.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.subscriber.base.repr.value.ref.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_IN_BINARY:
        {
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_DATAREADER:
    {
        switch (self->object_repr.value.data_reader.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.data_reader.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.data_reader.base.repr.value.ref.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_IN_BINARY:
        {
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_DATAWRITER:
    {
        switch (self->object_repr.value.data_writer.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.data_writer.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.data_writer.base.repr.value.ref.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_IN_BINARY:
        {
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_TYPE:
    {
        switch (self->object_repr.value.type.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.type.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.type.base.repr.value.ref.value);
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_DOMAIN:
    {
        switch (self->object_repr.value.domain.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.domain.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.domain.base.repr.value.ref.value);
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_QOSPROFILE:
    {
        switch (self->object_repr.value.qos_profile.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.qos_profile.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.qos_profile.base.repr.value.ref.value);
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_APPLICATION:
    {
        switch (self->object_repr.value.application.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.application.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.application.base.repr.value.ref.value);
            break;
        }
        default:
            break;
        }
        break;
    }
    case NANO_XRCE_OBJK_SERVICE:
    {
        switch (self->object_repr.value.service.base.repr.format)
        {
        case NANO_XRCE_REPRESENTATION_AS_XML_STRING:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.service.base.repr.value.xml.value);
            break;
        }
        case NANO_XRCE_REPRESENTATION_BY_REFERENCE:
        {
            RTIOsapiHeap_freeString(
                self->object_repr.value.service.base.repr.value.ref.value);
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

NANO_PRIVATE
void
NANO_XRCE_ReadDataPayload_finalize(
    NANO_XRCE_ReadDataPayload *const self)
{
    if (self->read_spec.content_filter_expr.value != NULL)
    {
        RTIOsapiHeap_freeString(self->read_spec.content_filter_expr.value);
        self->read_spec.content_filter_expr.value = NULL;
    }
}

NANO_PRIVATE
NANO_RetCode
NANO_XRCE_ProxyClient_on_submsg(
    NANO_XRCE_SessionListener *const listener,
    NANO_XRCE_Session *const session,
    const NANO_MessageBuffer *const mbuf,
    const NANO_XRCE_MessageHeader *const msg_hdr,
    const NANO_XRCE_SubmessageHeader *const submsg_hdr,
    const NANO_u8 *const payload)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ProxyClient *self = (NANO_XRCE_ProxyClient*) listener->user_data;
    NANO_XRCE_Agent *agent = self->agent;
    NANO_CDR_Stream stream = NANO_CDR_STREAM_INITIALIZER;
    NANO_XRCE_RequestId op_req_id = NANO_XRCE_REQUESTID_INVALID;
    NANO_XRCE_ObjectId op_obj_id = NANO_XRCE_OBJECTID_INVALID;
    NANO_XRCE_ProxyClientRequest *req = NULL;


    UNUSED_ARG(session);
    UNUSED_ARG(mbuf);

    NANO_PCOND(self != NULL)
    NANO_PCOND(!self->disposed)

    NANO_LOG_TRACE("SUBMSG recvd by ProxyClient",
        NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&self->session))
        NANO_LOG_MSGHDR("msg",*msg_hdr)
        NANO_LOG_SUBMSGHDR("submsg",*submsg_hdr)
        NANO_LOG_BYTES("payload",payload,submsg_hdr->length))

    NANO_CHECK_RC(
        NANO_CDR_Stream_initialize(
            &stream,
            (NANO_u8*) payload,
            submsg_hdr->length,
            NANO_XRCE_SubmessageFlags_is_little_endian(submsg_hdr->flags),
            NANO_BOOL_FALSE /* owned */),
        NANO_LOG_ERROR_MSG("FAILED to initialize CDR Stream"));
    
    switch (submsg_hdr->id)
    {
    case NANO_XRCE_SUBMESSAGEID_CREATE:
    {
        NANO_XRCE_CreatePayload payload = NANO_XRCE_CREATEPAYLOAD_INITIALIZER;
        
        NANO_CHECK_RC(
            NANO_XRCE_CreatePayload_deserialize_cdr(&payload, &stream),
            NANO_LOG_ERROR_MSG("FAILED to deserialize CREATE payload"));
        
        op_req_id = payload.base.request_id;
        op_obj_id = payload.base.object_id;

        req = NANO_XRCE_Agent_new_client_request(
                agent,
                self,
                msg_hdr,
                submsg_hdr,
                &op_req_id,
                &op_obj_id,
                NANO_BOOL_TRUE);
        if (req == NULL)
        {
            NANO_XRCE_CreatePayload_finalize(&payload);
            rc = NANO_RETCODE_TRY_AGAIN;
            goto done;
        }

        NANO_LOG_DEBUG("CREATE request",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&self->session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&self->session))
            NANO_LOG_STREAMID("stream", msg_hdr->stream_id)
            NANO_LOG_REQID("req_id", op_req_id)
            NANO_LOG_OBJID("obj_id", op_obj_id))

        NANO_CHECK_RC(
            NANO_XRCE_Agent_on_submsg_create(agent, req, &payload),
            NANO_XRCE_CreatePayload_finalize(&payload);
            NANO_LOG_ERROR_MSG("FAILED to handle CREATE submessage"));
        NANO_XRCE_CreatePayload_finalize(&payload);
        break;
    }
    case NANO_XRCE_SUBMESSAGEID_DELETE:
    {
        NANO_XRCE_DeletePayload payload = NANO_XRCE_DELETEPAYLOAD_INITIALIZER;

        NANO_CHECK_RC(
            NANO_XRCE_DeletePayload_deserialize_cdr(&payload, &stream),
            NANO_LOG_ERROR_MSG("FAILED to deserialize DELETE payload"));
        
        op_req_id = payload.base.request_id;
        op_obj_id = payload.base.object_id;

        req = NANO_XRCE_Agent_new_client_request(
                agent,
                self,
                msg_hdr,
                submsg_hdr,
                &op_req_id,
                &op_obj_id,
                NANO_BOOL_TRUE);
        if (req == NULL)
        {
            rc = NANO_RETCODE_TRY_AGAIN;
            goto done;
        }

        NANO_LOG_DEBUG("DELETE request",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&self->session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&self->session))
            NANO_LOG_STREAMID("stream", msg_hdr->stream_id)
            NANO_LOG_REQID("req_id", op_req_id)
            NANO_LOG_OBJID("obj_id", op_obj_id))

        NANO_CHECK_RC(
            NANO_XRCE_Agent_on_submsg_delete(agent, req, &payload),
            NANO_LOG_ERROR_MSG("FAILED to handle DELETE submessage"));
        
        break;
    }
    case NANO_XRCE_SUBMESSAGEID_WRITE_DATA:
    {
        NANO_XRCE_WriteDataPayload payload =
            NANO_XRCE_WRITEDATAPAYLOAD_INITIALIZER;
        
        /* Data format is stored in submsghdr flags */
        payload.data_repr.format =
            NANO_XRCE_SubmessageFlags_DATA_format(submsg_hdr->flags);

        NANO_CHECK_RC(
            NANO_XRCE_WriteDataPayload_deserialize_cdr(&payload, &stream),
            NANO_LOG_ERROR_MSG("FAILED to deserialize WRITE_DATA payload"));

        op_req_id = payload.base.request_id;
        op_obj_id = payload.base.object_id;

        req = NANO_XRCE_Agent_new_client_request(
                agent,
                self,
                msg_hdr,
                submsg_hdr,
                &op_req_id,
                &op_obj_id,
                NANO_XRCE_SubmessageFlags_WRITEDATA_confirm(submsg_hdr->flags) ||
                self->agent->props.confirm_all_requests);
        if (req == NULL)
        {
            rc = NANO_RETCODE_TRY_AGAIN;
            goto done;
        }

        NANO_LOG_DEBUG("WRITE_DATA request",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&self->session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&self->session))
            NANO_LOG_STREAMID("stream", msg_hdr->stream_id)
            NANO_LOG_REQID("req_id", op_req_id)
            NANO_LOG_OBJID("obj_id", op_obj_id)
            NANO_LOG_PTR("req", req)
            NANO_LOG_PTR("client",self)
            NANO_LOG_PTR("req->client",req->client))
        
        NANO_PCOND(self == req->client)

        NANO_CHECK_RC(
            NANO_XRCE_Agent_on_submsg_write(agent, req, &payload),
            NANO_LOG_ERROR_MSG("FAILED to handle WRITE_DATA submessage"));
        
        break;
    }
    case NANO_XRCE_SUBMESSAGEID_READ_DATA:
    {
        NANO_XRCE_ReadDataPayload payload =
            NANO_XRCE_READDATAPAYLOAD_INITIALIZER;

        NANO_CHECK_RC(
            NANO_XRCE_ReadDataPayload_deserialize_cdr(&payload, &stream),
            NANO_LOG_ERROR_MSG("FAILED to deserialize READ_DATA payload"));
        
        op_req_id = payload.base.request_id;
        op_obj_id = payload.base.object_id;

        req = NANO_XRCE_Agent_new_client_request(
                agent,
                self,
                msg_hdr,
                submsg_hdr,
                &op_req_id,
                &op_obj_id,
                NANO_XRCE_SubmessageFlags_READDATA_confirm(submsg_hdr->flags) ||
                    self->agent->props.confirm_all_requests);
        if (req == NULL)
        {
            NANO_XRCE_ReadDataPayload_finalize(&payload);
            rc = NANO_RETCODE_TRY_AGAIN;
            goto done;
        }

        NANO_LOG_DEBUG("READ_DATA request",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&self->session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&self->session))
            NANO_LOG_STREAMID("stream", msg_hdr->stream_id)
            NANO_LOG_REQID("req_id", op_req_id)
            NANO_LOG_OBJID("obj_id", op_obj_id))

        NANO_CHECK_RC(
            NANO_XRCE_Agent_on_submsg_read(agent, req, &payload),
            NANO_XRCE_ReadDataPayload_finalize(&payload);

            req = NULL;
            NANO_LOG_ERROR_MSG("FAILED to handle READ submessage"));
        NANO_XRCE_ReadDataPayload_finalize(&payload);
        break;
    }
    case NANO_XRCE_SUBMESSAGEID_GET_INFO:
    {
        NANO_XRCE_GetInfoPayload payload = NANO_XRCE_GETINFOPAYLOAD_INITIALIZER;

        NANO_CHECK_RC(
            NANO_XRCE_GetInfoPayload_deserialize_cdr(&payload, &stream),
            NANO_LOG_ERROR_MSG("FAILED to deserialize GET_INFO payload"));
        
        NANO_CHECK_RC(
            NANO_XRCE_Agent_on_submsg_getinfo(agent, self, &payload),
            NANO_LOG_ERROR_MSG("FAILED to handle GET_INFO submessage"));
        
        break;
    }
    case NANO_XRCE_SUBMESSAGEID_SERVICE_REQUEST:
    {
        NANO_XRCE_ServiceRequestPayload payload = NANO_XRCE_SERVICEREQUESTPAYLOAD_INITIALIZER;

        NANO_CHECK_RC(
            NANO_XRCE_ServiceRequestPayload_deserialize_cdr(&payload, &stream),
            NANO_LOG_ERROR_MSG("FAILED to deserialize SERVICE_REQUEST payload"));

        op_req_id = payload.base.request_id;
        op_obj_id = payload.base.object_id;

        req = NANO_XRCE_Agent_new_client_request(
                agent,
                self,
                msg_hdr,
                submsg_hdr,
                &op_req_id,
                &op_obj_id,
                NANO_XRCE_SubmessageFlags_SERVICEREQUEST_confirm(submsg_hdr->flags) ||
                    self->agent->props.confirm_all_requests);
        if (req == NULL)
        {
            rc = NANO_RETCODE_TRY_AGAIN;
            goto done;
        }

        NANO_LOG_DEBUG("SERVICE request",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&self->session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(&self->session))
            NANO_LOG_STREAMID("stream", msg_hdr->stream_id)
            NANO_LOG_REQID("req_id", op_req_id)
            NANO_LOG_OBJID("obj_id", op_obj_id)
            NANO_LOG_H16("svc_flags", payload.flags)
            NANO_LOG_H16("query_len", payload.query_len)
            NANO_LOG_H32("data_len", payload.data_len)
            NANO_LOG_H32("metadata_len", payload.metadata_len)
            NANO_LOG_BOOL("has_payload", payload.has_payload)
            NANO_LOG_BOOL("confirm", NANO_XRCE_SubmessageFlags_SERVICEREQUEST_confirm(submsg_hdr->flags))
            NANO_LOG_BOOL("reply", req->reply)
            NANO_LOG_PTR("payload", NANO_XRCE_BinData_contiguous_buffer(&payload.payload)))

        NANO_CHECK_RC(
            NANO_XRCE_Agent_on_submsg_servicereq(agent, req, &payload),
            NANO_LOG_ERROR_MSG("FAILED to handle SERVICE_REQUEST submessage"));
        
        break;
    }
    default:
    {
        NANO_LOG_WARNING("ignoring UNEXPECTED submessage",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(&self->session))
            NANO_LOG_MSGHDR("msg",*msg_hdr)
            NANO_LOG_SUBMSGHDR("submsg",*submsg_hdr))
        rc = NANO_RETCODE_OK;
        goto done;
    }
    }

    rc = NANO_RETCODE_OK;

done:
    if (req != NULL)
    {
        if (req->reply)
        {
            if (NANO_RETCODE_OK !=
                    NANO_XRCE_Agent_reply_to_operation(agent, req, rc))
            {
                NANO_LOG_ERROR_MSG("FAILED to send reply to client")
            }
        }
        if (/* return all failed requests here... */
            (NANO_RETCODE_OK != rc ||
            /* ...and also all successful ones which are not READ_DATA or
                WRITE_DATA or... */
            (submsg_hdr->id != NANO_XRCE_SUBMESSAGEID_READ_DATA &&
                submsg_hdr->id != NANO_XRCE_SUBMESSAGEID_WRITE_DATA &&
                submsg_hdr->id != NANO_XRCE_SUBMESSAGEID_WRITE_DATA)) &&
            /* ...but not the DELETE(CLIENT) ones, and... */
            !(submsg_hdr->id == NANO_XRCE_SUBMESSAGEID_DELETE &&
                NANO_XRCE_ObjectId_kind(&req->object_id) ==
                    NANO_XRCE_OBJK_CLIENT) &&
            /* ... neither SERVICE_REQUEST that are not oneway */
            !(submsg_hdr->id == NANO_XRCE_SUBMESSAGEID_SERVICE_REQUEST &&
                !NANO_XRCE_SubmessageFlags_SERVICEREQUEST_oneway(submsg_hdr->flags)))
        {
            NANO_XRCE_Agent_return_client_request(agent, req);
        }
    }

    return rc;
}

NANO_RetCode
NANO_XRCE_ProxyClient_dismiss_forward_request(
    NANO_XRCE_ProxyClient *self,
    const NANO_XRCE_StreamId stream_id,
    const NANO_XRCE_SeqNum *const data_sn)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ForwardDataRequest *fwd_req = NULL;
    const D2S2_ReceivedData *rcvd_data = NULL;
    NANO_bool is_data = NANO_BOOL_TRUE;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    fwd_req = (NANO_XRCE_ForwardDataRequest*)
        REDAInlineList_getFirst(&self->forwards);

    while (fwd_req != NULL)
    {
        NANO_i8 cmp_res = 0;

        if (fwd_req->stream_id == stream_id)
        {
            NANO_XRCE_SeqNum_compare(&fwd_req->sn, data_sn, &cmp_res);
            if (cmp_res == 0)
            {
                break;
            }
        }

        fwd_req = (NANO_XRCE_ForwardDataRequest*)
            REDAInlineListNode_getNext(&fwd_req->node);
    }

    if (fwd_req == NULL)
    {
        is_data = NANO_BOOL_FALSE;

        fwd_req = (NANO_XRCE_ForwardDataRequest*)
            REDAInlineList_getFirst(&self->forward_replies);

        while (fwd_req != NULL)
        {
            NANO_i8 cmp_res = 0;

            if (fwd_req->stream_id == stream_id)
            {
                NANO_XRCE_SeqNum_compare(&fwd_req->sn, data_sn, &cmp_res);
                if (cmp_res == 0)
                {
                    break;
                }
            }

            fwd_req = (NANO_XRCE_ForwardDataRequest*)
                REDAInlineListNode_getNext(&fwd_req->node);
        }

        if (fwd_req == NULL)
        {
            rc = NANO_RETCODE_OK;
            goto done;
        }
        REDAInlineList_removeNodeEA(&self->forward_replies, &fwd_req->node);
    }
    else
    {
        REDAInlineList_removeNodeEA(&self->forwards, &fwd_req->node);
    }

    rcvd_data = fwd_req->rcvd_data;
    REDAFastBufferPool_returnBuffer(self->forwards_pool, fwd_req);


    NANO_LOG_TRACE("DISMISSED forward request",
            NANO_LOG_KEY("session.key",*NANO_XRCE_Session_key(&self->session))
            NANO_LOG_SESSIONID("session.id",*NANO_XRCE_Session_id(&self->session))
            NANO_LOG_STREAMID("stream", stream_id)
            NANO_LOG_SN("sn", *data_sn))

    rc = NANO_RETCODE_OK;
    
done:

    if (rcvd_data != NULL)
    {
        if (is_data)
        {
            if (DDS_RETCODE_OK !=
                    D2S2_Agent_return_loan(
                        self->agent->base.agent,
                        &self->agent->base,
                        self->dds_session,
                        (D2S2_ReceivedData*)rcvd_data))
            {
                NANO_LOG_WARNING("FAILED to return loan to agent",
                    NANO_LOG_KEY("session.key",*NANO_XRCE_Session_key(&self->session))
                    NANO_LOG_SESSIONID("session.id",*NANO_XRCE_Session_id(&self->session)))
                rc = NANO_RETCODE_ERROR;
            }
        }
        else
        {
            if (DDS_RETCODE_OK !=
                    D2S2_Agent_return_external_service_reply(
                        self->agent->base.agent,
                        &self->agent->base,
                        self->dds_session,
                        (D2S2_ReceivedData*)rcvd_data))
            {
                NANO_LOG_WARNING("FAILED to return loan to agent",
                    NANO_LOG_KEY("session.key",*NANO_XRCE_Session_key(&self->session))
                    NANO_LOG_SESSIONID("session.id",*NANO_XRCE_Session_id(&self->session)))
                rc = NANO_RETCODE_ERROR;
            }
        }
    }
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_PRIVATE
void
NANO_XRCE_ProxyClient_on_send_complete(
    NANO_XRCE_SessionListener *const listener,
    NANO_XRCE_Session *const session,
    NANO_XRCE_ReliableStream *const stream,
    const NANO_XRCE_MessageHeader *const msg_hdr,
    const NANO_XRCE_SubmessageHeader *const submsg_hdr,
    NANO_MessageBuffer *const payload)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ProxyClient *self = (NANO_XRCE_ProxyClient*) listener->user_data;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    UNUSED_ARG(payload);
    
    NANO_LOG_TRACE("SEND complete",
        NANO_LOG_KEY("client", *NANO_XRCE_Session_key(session))
        NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(session))
        NANO_LOG_STREAMID("stream",
            NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream))
        NANO_LOG_MSGHDR("msg", *msg_hdr)
        NANO_LOG_SUBMSGHDR("submsg",*submsg_hdr))
    
    if (submsg_hdr->id == NANO_XRCE_SUBMESSAGEID_DATA ||
        submsg_hdr->id == NANO_XRCE_SUBMESSAGEID_FRAGMENT ||
        submsg_hdr->id == NANO_XRCE_SUBMESSAGEID_SERVICE_REPLY)
    {
        /* Lookup forward data request and return loan to D2S2_Agent */
        NANO_CHECK_RC(
            NANO_XRCE_ProxyClient_dismiss_forward_request(
                self,
                NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream),
                &msg_hdr->sn),
            NANO_LOG_DEBUG("FAILED to dismiss forward request on send complete",
                NANO_LOG_KEY("client", *NANO_XRCE_Session_key(session))
                NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(session))
                NANO_LOG_STREAMID("stream",
                    NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream))
                NANO_LOG_MSGHDR("msg", *msg_hdr)
                NANO_LOG_SUBMSGHDR("submsg",*submsg_hdr)));

    }
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

NANO_PRIVATE
void
NANO_XRCE_ProxyClient_on_msg_lost(
    NANO_XRCE_SessionListener *const listener,
    NANO_XRCE_Session *const session,
    NANO_XRCE_ReliableStream *const stream,
    const NANO_XRCE_SeqNum lost,
    const NANO_XRCE_ReliableMessageLostReason reason)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ProxyClient *self = (NANO_XRCE_ProxyClient*) listener->user_data;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    switch (reason)
    {
    case NANO_XRCE_RELIABLEMESSAGELOSTREASON_LOST_BY_REMOTE_WRITER:
    {
        NANO_LOG_WARNING("message LOST by remote writer",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(session))
            NANO_LOG_STREAMID("stream", NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream))
            NANO_LOG_SN("sn", lost))
        break;
    }
    case NANO_XRCE_RELIABLEMESSAGELOSTREASON_LOST_BY_LOCAL_WRITER:
    {
        NANO_LOG_WARNING("message LOST by local writer",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(session))
            NANO_LOG_STREAMID("stream", NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream))
            NANO_LOG_SN("sn", lost))
        
        /* try to find forward request and dismiss it if present */
        NANO_CHECK_RC(
            NANO_XRCE_ProxyClient_dismiss_forward_request(
                self,
                NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream),
                &lost),
            NANO_LOG_ERROR("FAILED to dismiss forward request on sample lost",
                NANO_LOG_KEY("client", *NANO_XRCE_Session_key(session))
                NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(session))
                NANO_LOG_STREAMID("stream",
                    NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream))
                NANO_LOG_SN("sn", lost)));

        break;
    }
    default:
    {
        NANO_LOG_WARNING("message LOST for UNKNOWN REASONS",
            NANO_LOG_KEY("client", *NANO_XRCE_Session_key(session))
            NANO_LOG_SESSIONID("session", *NANO_XRCE_Session_id(session))
            NANO_LOG_STREAMID("stream", NANO_XRCE_Stream_id((NANO_XRCE_Stream*)stream))
            NANO_LOG_SN("sn", lost)
            NANO_LOG_H32("reason", reason))
        break;
    }
    }
    
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

NANO_RetCode
NANO_XRCE_ProxyClient_initialize(
    NANO_XRCE_ProxyClient *const self,
    const NANO_XRCE_ClientRepresentation *const client_repr,
    NANO_XRCE_Agent *const agent)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SessionProperties session_props =
        NANO_XRCE_SESSIONPROPERTIES_INITIALIZER;
    NANO_XRCE_ProxyClientTransportProperties transport_props =
        NANO_XRCE_PROXYCLIENTTRANSPORTPROPERTIES_INITIALIZER;
    NANO_XRCE_StreamStorageRecord *storage_be = NULL,
                                  *storage_rel = NULL;
    struct REDAFastBufferPoolProperty pool_props =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    const NANO_XRCE_ProxyClient def_self = NANO_XRCE_PROXYCLIENT_INITIALIZER;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(client_repr != NULL)
    NANO_PCOND(NANO_XRCE_ClientKey_is_valid(&client_repr->client_key))
    
    *self = def_self;

    self->agent = agent;

    self->transport.agent = agent;
    self->transport.client = self;

    self->storage =
        (NANO_XRCE_SessionStorageRecord*)
            REDAFastBufferPool_getBuffer(
                self->agent->pool_storage_session);
    if (self->storage == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate reliable storage")
        goto done;
    }
    NANO_XRCE_DefaultAgentSessionStorage_initialize(&self->storage->storage);

    NANO_CHECK_RC(
        NANO_XRCE_ProxyClient_allocate_stream_storage(self, &storage_be),
        NANO_LOG_ERROR_MSG("FAILED to allocate best-effort stream storage"));
    
    NANO_CHECK_RC(
        NANO_XRCE_ProxyClient_allocate_stream_storage(self, &storage_rel),
        NANO_LOG_ERROR_MSG("FAILED to allocate reliable stream storage"));

    self->requests_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_ProxyClientRequest, &pool_props);
    if (self->requests_pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate requests pool")
        goto done;
    }

    self->forwards_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_ForwardDataRequest, &pool_props);
    if (self->forwards_pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate forwards pool")
        goto done;
    }
    
    self->writes_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_WriteRequest, &pool_props);
    if (self->writes_pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate writes pool")
        goto done;
    }

    session_props.id = client_repr->session_id;
    session_props.key = client_repr->client_key;
    session_props.listener.on_submsg = NANO_XRCE_ProxyClient_on_submsg;
    session_props.listener.on_send_complete =
        NANO_XRCE_ProxyClient_on_send_complete;
    session_props.listener.on_msg_lost = NANO_XRCE_ProxyClient_on_msg_lost;
    session_props.listener.user_data = (NANO_u8*) self;
    session_props.storage = &self->storage->storage.base;
    session_props.transport = &self->transport.base;
    session_props.transport_properties = &transport_props.base;

    session_props.heartbeat_period = agent->props.heartbeat_period;

    transport_props.key = client_repr->client_key;
    transport_props.id = client_repr->session_id;
#if NANO_FEAT_MTU_IN_CLIENT_REPR
    transport_props.base.mtu = client_repr->mtu;
    NANO_LOG_DEBUG("client MTU",
        NANO_LOG_U16("mtu", transport_props.base.mtu))
#endif /* NANO_FEAT_MTU_IN_CLIENT_REPR */


    NANO_CHECK_RC(
        NANO_XRCE_Session_initialize(&self->session, &session_props),
        NANO_LOG_ERROR_MSG("FAILED to initialize proxy-client's session"));
    
    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        /* TODO finalize client */
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_XRCE_ProxyClient_finalize(NANO_XRCE_ProxyClient *const self)
{
    NANO_XRCE_StreamStorageRecord *storage = NULL;
    struct REDAInlineListNode *node = NULL;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)

    NANO_XRCE_Session_finalize(&self->session);
    NANO_XRCE_Session_finalize_storage(&self->session);

    node = REDAInlineList_getFirst(&self->stream_storage);
    while (node != NULL)
    {
        storage = (NANO_XRCE_StreamStorageRecord*) node;
        node = REDAInlineListNode_getNext(&storage->node);

        NANO_XRCE_ProxyClient_release_stream_storage(self, storage);
    }
    
    REDAFastBufferPool_delete(self->requests_pool);
    self->requests_pool = NULL;

    REDAFastBufferPool_delete(self->writes_pool);
    self->writes_pool = NULL;

    REDAFastBufferPool_delete(self->forwards_pool);
    self->forwards_pool = NULL;
    
    NANO_LOG_FN_EXIT
}
