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

#include "NddsReader.h"
#include "NddsAgent.h"
#include "NddsAgentDb.h"

RTI_PRIVATE
RTIBool
NDDSA_Reader_notify_data_available(NDDSA_Reader *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_Reader_notify_data_available)
    RTIBool retcode = RTI_FALSE;
    NDDSA_ReaderRequest *req = NULL;
    struct DDS_DynamicDataSeq samples_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq infos_seq = DDS_SEQUENCE_INITIALIZER;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    const struct RTINtpTime ts_zero = RTI_NTP_TIME_ZERO;

    D2S2Log_fn_entry()

    RTIOsapiSemaphore_take(self->sem_requests, RTI_NTP_TIME_INFINITE);

    // if (!NDDSA_AgentDb_lookup_resource(
    //         &self->agent->db, &self->resource_id, &resource_rec))
    // {
    //     goto done;
    // }
    // if (resource_rec == NULL)
    // {
    //     goto done;
    // }

    /* call READ to clear the DATA_AVAILABLE status */
    rc = DDS_DynamicDataReader_read(
                        self->dyn_reader,
                        &samples_seq,
                        &infos_seq,
                        1,
                        DDS_ANY_SAMPLE_STATE,
                        DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
    if (rc == DDS_RETCODE_OK)
    {
        if (DDS_RETCODE_OK !=
                DDS_DynamicDataReader_return_loan(
                    self->dyn_reader, &samples_seq, &infos_seq))
        {
            /* TODO log */
            goto done;
        }
    }
    else if (rc != DDS_RETCODE_NO_DATA)
    {
        goto done;
    }
    
    req = (NDDSA_ReaderRequest *)REDAInlineList_getFirst(&self->requests);

    while (req != NULL)
    {
        /* only fire event if the read is not already marked as "delayed",
           in which case, it's already scheduled to fire again from a
           previous notification */
        if (!NDDSA_Agent_post_event(
                self->agent,
                &req->read->event_listener,
                &req->read->event_listener_storage,
                sizeof(NDDSA_Read*),
                &ts_zero))
        {
            goto done;
        }
        
        req = (NDDSA_ReaderRequest*)
                REDAInlineListNode_getNext(&req->node);
    }
    
    retcode = RTI_TRUE;
    
done:

    RTIOsapiSemaphore_give(self->sem_requests);
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
void
NDDSA_Reader_on_data_available(
    void *const handler,
    DDS_DataReader *const reader)
{
    NDDSA_Reader *const self = (NDDSA_Reader*)handler;
    
    UNUSED_ARG(reader);

    if (self->agent == NULL)
    {
        return;
    }

    NDDSA_Reader_notify_data_available(self);
}

RTIBool
NDDSA_Reader_initialize(
    NDDSA_Reader *const self,
    NDDSA_Resource *const resource)
{
    D2S2Log_METHOD_NAME(NDDSA_Reader_initialize)
    RTIBool retcode = RTI_FALSE;
    struct REDAFastBufferPoolProperty pool_props =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    const NDDSA_Reader def_self = NDDSA_READER_INITIALIZER;
    
    D2S2Log_fn_entry()

    if (resource->base.kind != D2S2_RESOURCEKIND_DATAREADER ||
        resource->native.kind != NDDSA_RESOURCENATIVEKIND_ENTITY ||
        resource->native.value.entity.reader == NULL)
    {
        goto done;
    }

    *self = def_self;

    if (!D2S2_ResourceId_copy(&self->resource_id, &resource->base.id))
    {
        goto done;
    }

    self->dyn_reader =
        DDS_DynamicDataReader_narrow(resource->native.value.entity.reader);
    if (self->dyn_reader == NULL)
    {
        goto done;
    }

    self->pool_requests = 
        REDAFastBufferPool_newForStructure(NDDSA_ReaderRequest, &pool_props);
    if (self->pool_requests == NULL)
    {
        goto done;
    }

    // self->cond_status = 
    //     DDS_Entity_get_statuscondition(
    //         DDS_DataReader_as_entity(resource->native.value.entity.reader));
    // if (self->cond_status == NULL)
    // {
    //     goto done;
    // }

    // /* We will enable the status condition once a subscription is created */
    // if (DDS_RETCODE_OK !=
    //         DDS_StatusCondition_set_enabled_statuses(
    //             self->cond_status, DDS_STATUS_MASK_NONE))
    // {
    //     goto done;
    // }

    self->sem_requests =
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_MUTEX, NULL);
    if (self->sem_requests == NULL)
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_Reader_finalize(
    NDDSA_Reader *const self)
{
    RTIBool retcode = RTI_FALSE;
    struct DDS_DataReaderListener listener = DDS_DataReaderListener_INITIALIZER;
    DDS_DataReader *reader = NULL;

    if (NULL != REDAInlineList_getFirst(&self->requests))
    {
        goto done;
    }

    reader = DDS_DynamicDataReader_as_datareader(self->dyn_reader);

    if (DDS_RETCODE_OK !=
            DDS_DataReader_set_listener(
                reader, &listener, DDS_STATUS_MASK_NONE))
    {
        goto done;
    }

    RTIOsapiSemaphore_delete(self->sem_requests);

    if (self->pool_requests != NULL)
    {
        REDAFastBufferPool_delete(self->pool_requests);
        self->pool_requests = NULL;
    }

    D2S2_ResourceId_finalize(&self->resource_id);

    retcode = RTI_TRUE;
done:

    return retcode;
}

void
NDDSA_Reader_find_request_by_read(
    NDDSA_Reader *const self,
    NDDSA_Read *const read,
    NDDSA_ReaderRequest **const req_out)
{
    NDDSA_ReaderRequest *req = NULL;

    *req_out = NULL;
    req = (NDDSA_ReaderRequest *)REDAInlineList_getFirst(&self->requests);
    while (req != NULL && *req_out == NULL)
    {
        if (req->read == read)
        {
            *req_out = req;
        }
        req = (NDDSA_ReaderRequest*)REDAInlineListNode_getNext(&req->node);
    }
}

RTIBool
NDDSA_Reader_new_request(
    NDDSA_Reader *const self,
    NDDSA_Read *const read)
{
    D2S2Log_METHOD_NAME(NDDSA_Reader_new_request)
    RTIBool retcode = RTI_FALSE,
            first_request = RTI_FALSE;
    NDDSA_ReaderRequest *req = NULL;
    const NDDSA_ReaderRequest def_req = NDDSA_READERREQUEST_INITIALIZER;
    struct DDS_StringSeq query_params = DDS_SEQUENCE_INITIALIZER;
    DDS_DataReader *dds_reader = NULL;
    struct DDS_DataReaderListener listener =
        DDS_DataReaderListener_INITIALIZER;

    D2S2Log_fn_entry()


    dds_reader = DDS_DynamicDataReader_as_datareader(self->dyn_reader);
    if (dds_reader == NULL)
    {
        goto done;
    }

    RTIOsapiSemaphore_take(self->sem_requests, RTI_NTP_TIME_INFINITE);
    NDDSA_Reader_find_request_by_read(self, read, &req);
    if (req != NULL)
    {
        RTIOsapiSemaphore_give(self->sem_requests);
        goto done;
    }
    RTIOsapiSemaphore_give(self->sem_requests);

    req = (NDDSA_ReaderRequest*)
                    REDAFastBufferPool_getBuffer(self->pool_requests);
    if (req == NULL)
    {
        goto done;
    }
    *req = def_req;
    req->read = read;
    if (req->read->content_filter_expr != NULL)
    {
        req->query_condition =
            DDS_DataReader_create_querycondition(
                dds_reader,
                DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ANY_INSTANCE_STATE,
                req->read->content_filter_expr,
                &query_params);
        if (req->query_condition == NULL)
        {
            goto done;
        }
    }

    RTIOsapiSemaphore_take(self->sem_requests, RTI_NTP_TIME_INFINITE);
    first_request = (REDAInlineList_getFirst(&self->requests) == NULL);
    REDAInlineList_addNodeToBackEA(&self->requests, &req->node);
    RTIOsapiSemaphore_give(self->sem_requests);
    if (first_request)
    {
        listener.as_listener.listener_data = self;
        listener.on_data_available = NDDSA_Reader_on_data_available;

        if (DDS_RETCODE_OK !=
                DDS_DataReader_set_listener(
                    dds_reader,
                    &listener,
                    DDS_DATA_AVAILABLE_STATUS))
        {
            goto done;
        }
    }

    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (req != NULL)
        {
            if (req->query_condition != NULL)
            {
                if (DDS_RETCODE_OK !=
                        DDS_DataReader_delete_readcondition(
                            dds_reader,
                            DDS_QueryCondition_as_readcondition(
                                req->query_condition)))
                {
                    goto done;
                }
            }
            REDAFastBufferPool_returnBuffer(self->pool_requests, req);
        }
        if (first_request)
        {
            if (DDS_RETCODE_OK !=
                    DDS_DataReader_set_listener(
                        dds_reader,
                        &listener,
                        DDS_DATA_AVAILABLE_STATUS))
            {
                /* TODO log */
            }
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_Reader_request_complete(
    NDDSA_Reader *const self,
    NDDSA_Read *const read)
{
    RTIBool retcode = RTI_FALSE,
            has_requests = RTI_FALSE;
    NDDSA_ReaderRequest *req = NULL;
    DDS_DataReader *dds_reader = NULL;
    struct DDS_DataReaderListener listener = DDS_DataReaderListener_INITIALIZER;

    dds_reader = DDS_DynamicDataReader_as_datareader(self->dyn_reader);
    if (dds_reader == NULL)
    {
        goto done;
    }

    RTIOsapiSemaphore_take(self->sem_requests, RTI_NTP_TIME_INFINITE);
    NDDSA_Reader_find_request_by_read(self, read, &req);
    NDDSA_Reader_find_request_by_read(self, read, &req);
    if (req == NULL)
    {
        RTIOsapiSemaphore_give(self->sem_requests);
        goto done;
    }
    REDAInlineList_removeNodeEA(&self->requests, &req->node);
    has_requests = (REDAInlineList_getFirst(&self->requests) != NULL);
    RTIOsapiSemaphore_give(self->sem_requests);

    if (req->query_condition != NULL)
    {
        if (DDS_RETCODE_OK !=
                DDS_DataReader_delete_readcondition(
                    dds_reader,
                    DDS_QueryCondition_as_readcondition(
                        req->query_condition)))
        {
            goto done;
        }
    }

    REDAFastBufferPool_returnBuffer(self->pool_requests, req);

    if (!has_requests)
    {
        if (DDS_RETCODE_OK !=
                DDS_DataReader_set_listener(
                    dds_reader, &listener, DDS_STATUS_MASK_NONE))
        {
            goto done;
        }
    }
    
    retcode = RTI_TRUE;
    
done:

    return retcode;
}

RTIBool
NDDSA_Reader_requests_updated(
    NDDSA_Reader *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    NDDSA_ResourceRecord *const resource_rec)
{
    D2S2Log_METHOD_NAME(NDDSA_Reader_requests_updated)
    RTIBool retcode = RTI_FALSE,
            first = RTI_TRUE,
            dismissed_all = RTI_FALSE;
    struct DDS_SequenceNumber_t dismiss_sn = DDS_SEQUENCENUMBER_DEFAULT;
    NDDSA_ReaderRequest *req = NULL;
    struct DDS_DynamicDataSeq samples_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq infos_seq = DDS_SEQUENCE_INITIALIZER;

    UNUSED_ARG(session_rec);
    UNUSED_ARG(attached);
    UNUSED_ARG(resource_rec);
    
    req = (NDDSA_ReaderRequest*) REDAInlineList_getFirst(&self->requests);
    while (req != NULL)
    {
        if (first || 
            DDS_SequenceNumber_compare(
                &dismiss_sn,
                &req->read->last_sample) >= 0)
        {
            dismiss_sn = req->read->last_sample;
            first = RTI_FALSE;
        }
        req = (NDDSA_ReaderRequest*) REDAInlineListNode_getNext(&req->node);
    }


    if (DDS_SequenceNumber_compare(
                &dismiss_sn, &self->dismiss_sn) <= 0)
    {
        /* we have already dismissed up to this SN */
        retcode = RTI_TRUE;
        goto done;
    }

    while (!dismissed_all)
    {
        DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
        struct DDS_SequenceNumber_t sn_next = DDS_SEQUENCENUMBER_DEFAULT;

        /* check if the next sample should be dismissed */
        rc = DDS_DynamicDataReader_read(
                        self->dyn_reader,
                        &samples_seq,
                        &infos_seq,
                        1,
                        DDS_ANY_SAMPLE_STATE,
                        DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
        if (DDS_RETCODE_NO_DATA == rc)
        {
            /* there are no more samples in the reader's cache */
            dismissed_all = RTI_TRUE;
        }
        else if (DDS_RETCODE_OK != rc)
        {
            /* TODO log */
            goto done;
        }
        else
        {
            struct DDS_SampleInfo *info =
                DDS_SampleInfoSeq_get_reference(&infos_seq, 0);
            
            sn_next = info->reception_sequence_number;

            if (info->valid_data)
            {
                if (DDS_SequenceNumber_compare(
                        &info->reception_sequence_number, &dismiss_sn) > 0)
                {
                    dismissed_all = RTI_TRUE;
                }
            }

            if (DDS_RETCODE_OK !=
                    DDS_DynamicDataReader_return_loan(
                        self->dyn_reader, &samples_seq, &infos_seq))
            {
                /* TODO log */
                goto done;
            }
        }

        if (dismissed_all)
        {
            continue;
        }

        rc = DDS_DynamicDataReader_take(
                    self->dyn_reader,
                    &samples_seq,
                    &infos_seq,
                    1,
                    DDS_ANY_SAMPLE_STATE,
                    DDS_ANY_VIEW_STATE,
                    DDS_ANY_INSTANCE_STATE);
        if (DDS_RETCODE_OK != rc)
        {
            /* TODO log */
            /* We assume that the sample should have been there */
            goto done;
        }

        {
            struct DDS_SampleInfo *info =
                DDS_SampleInfoSeq_get_reference(&infos_seq, 0);
            
            self->dismiss_sn = info->reception_sequence_number;

            if (DDS_SequenceNumber_compare(
                    &sn_next, &info->reception_sequence_number) != 0)
            {
                D2S2Log_warn(
                    method_name,\
                    &RTI_LOG_ANY_s,\
                    "DISMISSED WRONG SN on data reader");\
            }
        }

        if (DDS_RETCODE_OK !=
                DDS_DynamicDataReader_return_loan(
                    self->dyn_reader, &samples_seq, &infos_seq))
        {
            /* TODO log */
            goto done;
        }

        dismissed_all =
            DDS_SequenceNumber_compare(&self->dismiss_sn, &dismiss_sn) >= 0;
    }
    

    retcode = RTI_TRUE;
    
done:

    return retcode;
}