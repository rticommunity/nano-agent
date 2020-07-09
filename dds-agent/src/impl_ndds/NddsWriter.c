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

#include "NddsWriter.h"
#include "NddsAgent.h"

RTI_PRIVATE
void
NDDSA_Writer_on_condition_triggered(
    void * handler_data,
    DDS_Condition * condition)
{
    NDDSA_Writer *const self = (NDDSA_Writer*)handler_data;
    UNUSED_ARG(self);
    UNUSED_ARG(condition);
    
}


RTIBool
NDDSA_Writer_initialize(
    NDDSA_Writer *const self,
    NDDSA_Resource *const resource)
{
    D2S2Log_METHOD_NAME(NDDSA_Writer_initialize)
    RTIBool retcode = RTI_FALSE;
    struct REDAFastBufferPoolProperty pool_props =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    DDS_DataWriter *dw = NULL;
    struct DDS_DynamicDataTypeProperty_t support_props = 
                        DDS_DynamicDataTypeProperty_t_INITIALIZER;
    DDS_Topic *topic = NULL;
    DDS_TopicDescription *topic_desc = NULL;
    DDS_DomainParticipant *participant = NULL;
    const char *type_name = NULL;
    const DDS_TypeCode *type_code = NULL;

    D2S2Log_fn_entry()
    
    if (resource->base.kind != D2S2_RESOURCEKIND_DATAWRITER ||
        resource->native.kind != NDDSA_RESOURCENATIVEKIND_ENTITY ||
        resource->native.value.entity.writer == NULL)
    {
        goto done;
    }

    self->res = resource;

    self->data_buffer = NULL;
    self->data_buffer_len = 0;
    self->data_buffer_max = 0;

    self->pool_samples = 
        REDAFastBufferPool_newForStructure(NDDSA_WriterSample, &pool_props);
    if (self->pool_samples == NULL)
    {
        goto done;
    }

    dw = resource->native.value.entity.writer;

    self->cond_status = 
        DDS_Entity_get_statuscondition(DDS_DataWriter_as_entity(dw));
    

    self->cond_handler.handler_data =  self;
    self->cond_handler.on_condition_triggered =
        NDDSA_Writer_on_condition_triggered;
    
    if (DDS_RETCODE_OK !=
            DDS_Condition_set_handler(
                DDS_StatusCondition_as_condition(self->cond_status),
                &self->cond_handler))
    {
        goto done;
    }

    topic = DDS_DataWriter_get_topic(dw);
    if (topic == NULL)
    {
        goto done;
    }
    topic_desc = DDS_Topic_as_topicdescription(topic);

    type_name = DDS_TopicDescription_get_type_name(topic_desc);
    if (type_name == NULL)
    {
        goto done;
    }

    participant = DDS_TopicDescription_get_participant(topic_desc);
    if (participant == NULL)
    {
        goto done;
    }

    type_code = DDS_DomainParticipant_get_typecode(participant, type_name);
    if (type_code == NULL)
    {
        goto done;
    }

    self->dyn_writer = DDS_DynamicDataWriter_narrow(dw);
    if (self->dyn_writer == NULL)
    {
        printf("FAILED to narrow DataWriter %p\n",dw);
        goto done;
    }

    self->dyn_support = 
        DDS_DynamicDataTypeSupport_new(type_code, &support_props);
    if (self->dyn_support == NULL) {
        goto done;
    }

    self->dyn_sample =
        DDS_DynamicDataTypeSupport_create_data(self->dyn_support);
    if (self->dyn_sample == NULL)
    {
        goto done;
    }

    // {
    //     struct DDS_DynamicDataProperty_t sample_property = DDS_DynamicDataProperty_t_INITIALIZER;
    //     DDS_DynamicData_get_estimated_max_buffer_size(
    //         self->dyn_sample, &sample_property.buffer_initial_size);
    //     sample_property.buffer_max_size = sample_property.buffer_initial_size;
    //     DDS_DynamicDataTypeSupport_delete_data(self->dyn_support, self->dyn_sample);
    //     self->dyn_sample =
    //         DDS_DynamicDataTypeSupport_create_data(self->dyn_support);
    //     if (self->dyn_sample == NULL)
    //     {
    //         goto done;
    //     }
    // }

#if 0
    enabled_statuses = 
        DDS_StatusCondition_get_enabled_statuses(self->cond_status);

    enabled_statuses |= DDS_DATA_AVAILABLE_STATUS;
    
    if (DDS_RETCODE_OK !=
            DDS_StatusCondition_set_enabled_statuses(
                self->cond_status, enabled_statuses))
    {
        goto done;
    }
#endif

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

void
NDDSA_Writer_finalize(NDDSA_Writer *const self)
{
    if (self->data_buffer != NULL)
    {
        RTIOsapiHeap_freeBufferAligned(self->data_buffer);
        self->data_buffer = NULL;
        self->data_buffer_len = 0;
        self->data_buffer_max = 0;
    }
    DDS_DynamicDataTypeSupport_delete_data(self->dyn_support, self->dyn_sample);
    DDS_DynamicDataTypeSupport_delete(self->dyn_support);
    REDAFastBufferPool_delete(self->pool_samples);
}