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

#ifndef NddsWriter_h
#define NddsWriter_h

#include "NddsClientSession.h"

typedef struct NDDSA_WriterSampleI
{
    struct REDAInlineListNode node;
    const D2S2_DataRepresentation *data;
    DDS_ReturnCode_t write_rc;
} NDDSA_WriterSample;

#define NDDSA_WriterSample_is_success(s_) \
    (((s_)->write_rc) == DDS_RETCODE_OK)

typedef struct NDDSA_WriterI
{
    NDDSA_Resource *res;
    NDDSA_Agent *agent;
    DDS_DynamicDataWriter *dyn_writer;
    DDS_DynamicData *dyn_sample;
    struct DDS_DynamicDataTypeSupport *dyn_support;
    struct REDAInlineList requests;
    struct REDAInlineList samples;
    struct REDAFastBufferPool *pool_samples;
    void *user_data;
    DDS_StatusCondition *cond_status;
    struct DDS_ConditionHandler cond_handler;
    char *data_buffer;
    DDS_UnsignedLong data_buffer_len;
    DDS_UnsignedLong data_buffer_max;
} NDDSA_Writer;

RTIBool
NDDSA_Writer_initialize(
    NDDSA_Writer *const self,
    NDDSA_Resource *const resource);

void
NDDSA_Writer_finalize(NDDSA_Writer *const self);


#endif /* NddsWriter_h */