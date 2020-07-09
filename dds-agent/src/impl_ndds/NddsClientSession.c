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

#include "NddsClientSession.h"
#include "NddsAgentDb.h"
#include "NddsAgent.h"
#include "NddsReader.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

RTI_PRIVATE
RTIBool
NDDSA_Read_convert_sampleEA(
    NDDSA_Agent *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    DDS_DynamicData *const sample,
    struct DDS_SampleInfo *const sample_info,
    const DDS_UnsignedLong serialized_size,
    NDDSA_ReceivedData **const rcvd_data_out)
{
    D2S2Log_METHOD_NAME(NDDSA_Read_deliver_sampleEA)
    RTIBool retcode = RTI_FALSE;
    NDDSA_ReceivedData *rcvd_data = NULL;
    char *rti_ptr = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(sample_info);


    if (REDAInlineList_getFirst(&session_rec->session.rcvd_data_pool) != NULL)
    {
        rcvd_data =
            NDDSA_ReceivedData_from_node(
                REDAInlineList_getFirst(&session_rec->session.rcvd_data_pool));
        
        REDAInlineList_removeNodeEA(
            &session_rec->session.rcvd_data_pool, &rcvd_data->node);
        
        if (rcvd_data->data.value.xcdr.buffer.data != NULL)
        {
            if (rcvd_data->data.value.xcdr.buffer.data_len < serialized_size)
            {
                RTIOsapiHeap_freeBufferAligned(rcvd_data->data.value.xcdr.buffer.data);
                rcvd_data->data.value.xcdr.buffer.data = NULL;
                rcvd_data->data.value.xcdr.buffer.data_len = 0;
            }
        }
    }
    else
    {

        rcvd_data = 
            (NDDSA_ReceivedData*)
                REDAFastBufferPool_getBuffer(self->pool_samples);
        if (rcvd_data == NULL)
        {
            goto done;
        }
        rcvd_data->data.value.xcdr.buffer.data = NULL;
    }

    rcvd_data->session = &session_rec->session;
    rcvd_data->read = attached->read_req;

    rcvd_data->data.fmt = D2S2_DATAREPRESENTATIONFORMAT_XCDR;
    rcvd_data->data.value.xcdr.buffer.data_len = serialized_size;

    if (rcvd_data->data.value.xcdr.buffer.data == NULL)
    {
        RTIOsapiHeap_allocateBufferAligned(
            &rti_ptr,
            rcvd_data->data.value.xcdr.buffer.data_len,
            RTI_OSAPI_ALIGNMENT_DEFAULT);
        (rcvd_data->data.value.xcdr.buffer.data) = (unsigned char *)rti_ptr;
    }

    if (rcvd_data->data.value.xcdr.buffer.data == NULL)
    {
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_DynamicData_to_cdr_buffer(
                sample,
                (char*)rcvd_data->data.value.xcdr.buffer.data,
                &rcvd_data->data.value.xcdr.buffer.data_len))
    {
        goto done;
    }

    if (rcvd_data->data.value.xcdr.buffer.data[0] & 0x01)
    {
        /* PL */
        switch (rcvd_data->data.value.xcdr.buffer.data[1])
        {
        case 0x00:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_1_PL;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_FALSE;
            break;
        }
        case 0x01:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_1_PL;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_TRUE;
            break;
        }
        case 0x10:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_2_PL;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_FALSE;
            break;
        }
        case 0x11:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_2_PL;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_TRUE;
            break;
        }
        default:
            goto done;
        }
    }
    else
    {
        /* non-PL */
        switch (rcvd_data->data.value.xcdr.buffer.data[1])
        {
        case 0x00:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_1;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_FALSE;
            break;
        }
        case 0x01:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_1;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_TRUE;
            break;
        }
        case 0x10:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_2;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_FALSE;
            break;
        }
        case 0x11:
        {
            rcvd_data->data.value.xcdr.encoding = D2S2_XCDRENCODINGKIND_2;
            rcvd_data->data.value.xcdr.little_endian = DDS_BOOLEAN_TRUE;
            break;
        }
        default:
            goto done;
        }
    }

    rcvd_data->base.data = &rcvd_data->data;
    rcvd_data->base.session_key = session_rec->session.base.key;
    rcvd_data->base.reader_id = attached->base.id;


#if 0
    printf("****************************************\n");
    printf("FORWARD SAMPLE:\n");    
    printf("recvd(%u)(enc=%X, le=%d): { ",
        rcvd_data->data.value.xcdr.buffer.data_len,
        rcvd_data->data.value.xcdr.encoding,
        rcvd_data->data.value.xcdr.little_endian);
    for (size_t i = 0; i < rcvd_data->data.value.xcdr.buffer.data_len; i++)
    {
        printf("%02X ",rcvd_data->data.value.xcdr.buffer.data[i]);
    }
    printf("}\n");
    // printf("sample: ");
    // DDS_DynamicData_print(sample, stdout, 0);
    // printf("\n");
    printf("****************************************\n");
#endif

    *rcvd_data_out = rcvd_data;
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (rcvd_data != NULL)
        {
            REDAInlineList_addNodeToBackEA(
                &session_rec->session.rcvd_data_pool,
                &rcvd_data->node);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}



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
    struct RTINtpTime *const min_delay)
{
    D2S2Log_METHOD_NAME(NDDSA_Read_process_sampleEA)
    RTIBool retcode = RTI_FALSE,
            has_min_pace = RTI_FALSE,
            has_max_bytes = RTI_FALSE;
    struct RTINtpTime ts_diff = RTI_NTP_TIME_ZERO;
    struct RTINtpTime ts_delay_try = RTI_NTP_TIME_ZERO;
    DDS_UnsignedLong serialized_size = 0;
    DDS_Boolean retained = DDS_BOOLEAN_FALSE,
                deliver_again = DDS_BOOLEAN_FALSE;
    NDDSA_ReceivedData *rcvd_data = NULL;

    D2S2Log_fn_entry()

    *old_sample = RTI_FALSE;
    *try_again = RTI_FALSE;

    *old_sample =
        (DDS_SequenceNumber_compare(
            &attached->read_req->last_sample,
            &sample_info->reception_sequence_number) >= 0)?
                RTI_TRUE : RTI_FALSE;

    if (*old_sample)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    has_min_pace = RTINtpTime_compareToZero(&attached->read_req->sample_pace) != 0;
    has_max_bytes = attached->read_req->max_bytes > 0;


    if (has_min_pace)
    {
        RTINtpTime_subtract(ts_diff, *ts_now, attached->read_req->last_sample_ts);

        if (RTINtpTime_compare(ts_diff, attached->read_req->sample_pace) < 0)
        {
            RTINtpTime_subtract(
                *min_delay, attached->read_req->sample_pace, ts_diff);
            *try_again = RTI_TRUE;
            retcode = RTI_TRUE;
            goto done;
        }
    }

    if (DDS_RETCODE_OK !=
            DDS_DynamicData_to_cdr_buffer(sample, NULL, &serialized_size))
    {
        goto done;
    }

    if (has_max_bytes)
    {
        /* TODO check if writing would exceed max_bytes_per_second */
    }

    if (!NDDSA_Read_convert_sampleEA(
            self,
            session_rec,
            attached,
            sample,
            sample_info,
            serialized_size,
            &rcvd_data))
    {
        goto done;
    }

#if NANO_FEAT_ASSERT_LIVELINESS_ON_DATA
    if (DDS_RETCODE_OK !=
            NDDSA_Agent_on_session_activityI(self, session_rec))
    {
        goto done;
    }
#endif /* NANO_FEAT_ASSERT_LIVELINESS_ON_DATA */

    if (DDS_RETCODE_OK !=
            D2S2_AgentServerInterface_on_data_available(
                session_rec->session.intf,
                &self->base,
                &session_rec->session.base,
                session_rec->session.user_data,
                attached->base.id,
                attached->user_data,
                &rcvd_data->base,
                &retained,
                &deliver_again))
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "D2S2_AgentServerInterface_on_data_available");
        goto done;
    }

    if (!deliver_again)
    {
        if (retained)
        {
            REDAInlineList_addNodeToBackEA(
                &attached->read_req->samples, &rcvd_data->node);
        }
        else
        {
            attached->last_sample_returned =
                sample_info->reception_sequence_number;
            attached->read_req->samples_rcvd += 1;
        }
        attached->read_req->last_sample =
            sample_info->reception_sequence_number;
        attached->last_sample_forwarded =
            sample_info->reception_sequence_number;
        attached->read_req->last_sample_ts = *ts_now;

        rcvd_data = NULL;
    }
    else
    {
        RTINtpTime_packFromMillisec(ts_delay_try, 0, 1);
        *try_again = RTI_TRUE;
        *min_delay = ts_delay_try;
    }


    retcode = RTI_TRUE;
    
done:

    if (rcvd_data != NULL)
    {
        REDAInlineList_addNodeToBackEA(
            &session_rec->session.rcvd_data_pool, &rcvd_data->node);
    }
    D2S2Log_fn_exit()
    return retcode;
}

void
NDDSA_ClientSession_find_attached_resource(
    NDDSA_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    NDDSA_AttachedResource **const attached_res_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ClientSession_find_attached_resource)
    NDDSA_AttachedResource *res = NULL;
    struct REDAInlineListNode *node = NULL;

    D2S2Log_fn_entry()

    *attached_res_out = NULL;

    node = REDAInlineList_getFirst(&session->resources);
    while (node != NULL && *attached_res_out == NULL)
    {
        res = NDDSA_AttachedResource_from_node(node);

        if (res->base.id == resource_id)
        {
            *attached_res_out = res;
        }
        else
        {
            node = REDAInlineListNode_getNext(&res->node);
        }
    }

    D2S2Log_fn_exit()
}

void
NDDSA_Read_is_complete(
    NDDSA_Read *const self,
    struct RTINtpTime *const ts_now,
    RTIBool *const finite_request,
    RTIBool *const complete)
{
    RTIBool has_max_time = RTI_FALSE,
            has_max_samples = RTI_FALSE;
    struct RTINtpTime ts_diff = RTI_NTP_TIME_ZERO;
    
    *finite_request = RTI_FALSE;
    *complete = RTI_FALSE;

    has_max_time = 
        (RTINtpTime_compareToZero(&self->max_elapsed_ts) != 0);
    has_max_samples = self->samples_max > 0;
    
    if (!has_max_time && !has_max_samples)
    {
        return;
    }

    *finite_request = RTI_TRUE;

    if (has_max_time)
    {
        RTINtpTime_subtract(ts_diff, *ts_now, self->start_ts);
        if (RTINtpTime_compare(ts_diff, self->max_elapsed_ts) < 0)
        {
            /* not enough time has passed yet */
            return;
        }
    }
    if (has_max_samples)
    {
        if (self->samples_rcvd < self->samples_max)
        {
            /* not enough samples have been received */
            return;
        }
    }

    *complete = RTI_TRUE;
    return;
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */