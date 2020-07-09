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

#ifndef NddsReader_h
#define NddsReader_h

#include "NddsClientSession.h"

typedef struct NDDSA_ReaderRequestI
{
    struct REDAInlineListNode node;
    NDDSA_Read *read;
    DDS_QueryCondition *query_condition;
} NDDSA_ReaderRequest;

#define NDDSA_READERREQUEST_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* read */\
    NULL /* query_condition */\
}

typedef struct NDDSA_ReaderI
{
    D2S2_ResourceId resource_id;
    NDDSA_Agent *agent;
    DDS_DynamicDataReader *dyn_reader;
    struct RTIOsapiSemaphore *sem_requests;
    struct REDAInlineList requests;
    struct REDAFastBufferPool *pool_requests;
    DDS_StatusCondition *cond_status;
    struct DDS_ConditionHandler cond_handler;
    DDS_Boolean dismiss_samples;
    struct DDS_SequenceNumber_t dismiss_sn;
    RTIBool deleted;
} NDDSA_Reader;

#define NDDSA_READER_INITIALIZER \
{\
    D2S2_RESOURCEID_INITIALIZER, /* resource_id */\
    NULL, /* agent */\
    NULL, /* dyn_agent */\
    NULL, /* sem_requests */\
    REDA_INLINE_LIST_EMPTY, /* requests */\
    NULL, /* pool_requests */\
    NULL, /* cond_status */\
    DDS_ConditionHandler_INITIALIZER, /* cond_handler */\
    DDS_BOOLEAN_FALSE, /* dismiss_samples */\
    DDS_SEQUENCENUMBER_DEFAULT, /* dismiss_sn */\
    RTI_FALSE /* deleted */\
}

RTIBool
NDDSA_Reader_initialize(
    NDDSA_Reader *const self,
    NDDSA_Resource *const resource);

RTIBool
NDDSA_Reader_finalize(
    NDDSA_Reader *const self);

RTIBool
NDDSA_Reader_new_request(
    NDDSA_Reader *const self,
    NDDSA_Read *const read);

RTIBool
NDDSA_Reader_request_complete(
    NDDSA_Reader *const self,
    NDDSA_Read *const read);

void
NDDSA_Reader_find_request_by_read(
    NDDSA_Reader *const self,
    NDDSA_Read *const read,
    NDDSA_ReaderRequest **const req_out);

RTIBool
NDDSA_Reader_requests_updated(
    NDDSA_Reader *const self,
    NDDSA_ClientSessionRecord *const session_rec,
    NDDSA_AttachedResource *const attached,
    NDDSA_ResourceRecord *const resource_rec);

#endif /* NddsReader_h */