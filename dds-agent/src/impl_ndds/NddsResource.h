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

#ifndef NddsResource_h
#define NddsResource_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsResourceNative.h"

struct NDDSA_ResourceI
{
    D2S2_Resource base;
    NDDSA_ResourceNative native;
    DDS_UnsignedLong attached_count;
    NDDSA_Agent *agent;
    void *user_data;
    struct REDAInlineListNode node;
    RTIBool loaded;
    RTIBool native_deleted;
    RTIBool pending_delete;
};

#define NDDSA_RESOURCE_INITIALIZER \
{\
    D2S2_RESOURCE_INITIALIZER, /* base */\
    NDDSA_RESOURCENATIVE_INITIALIZER, /* native */\
    0, /* attached_count */\
    NULL, /* agent */\
    NULL, /* user_data */\
    REDAInlineListNode_INITIALIZER, /* node */\
    RTI_FALSE, /* loaded */\
    RTI_FALSE, /* native_deleted */\
    RTI_FALSE /* pending_delete */\
}


typedef struct NDDSA_ResourceRecordI
{
    struct REDAInlineListNode node;
    struct REDAWeakReference ref;
    struct REDAExclusiveArea *ea;
    NDDSA_Resource resource;
} NDDSA_ResourceRecord;

#define NDDSA_RESOURCERECORD_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    REDA_WEAK_REFERENCE_INVALID, /* ref */ \
    NULL, /* ea */\
    NDDSA_RESOURCE_INITIALIZER /* resource */\
}

NDDSA_ResourceRecord*
NDDSA_ResourceRecord_from_resource(NDDSA_Resource *const self);

#define NDDSA_ResourceRecord_from_resource(s_) \
((NDDSA_ResourceRecord*) \
    (((unsigned char*)(s_)) - \
        NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_ResourceRecord, resource)))


NDDSA_Resource*
NDDSA_Resource_from_node(struct REDAInlineListNode *const self);

#define NDDSA_Resource_from_node(s_) \
((NDDSA_Resource*) \
    (((unsigned char*)(s_)) - \
        NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_Resource, node)))


RTIBool
NDDSA_Resource_initialize(
    NDDSA_Resource *self,
    const D2S2_ResourceId *const id,
    const D2S2_ResourceKind kind,
    NDDSA_ResourceNative *const native);

void
NDDSA_Resource_finalize(NDDSA_Resource *self);

RTIBool
NDDSA_Resource_can_delete(NDDSA_Resource *self);

NDDSA_Resource*
NDDSA_Resource_from_event_delete(NDDSA_Resource *const self);

#define NDDSA_Resource_from_event_delete(s_) \
((NDDSA_Resource*) \
    (((unsigned char*)(s_)) - \
        NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_Resource, event_delete)))

RTIBool
NDDSA_Resource_convert_representation(
    NDDSA_Resource *const self,
    const D2S2_ResourceRepresentation *const from,
    const D2S2_ResourceRepresentationFormat out_fmt,
    D2S2_ResourceRepresentation *const to);

NDDSA_ResourceNative*
NDDSA_Resource_get_native(NDDSA_Resource *const self);

#define NDDSA_Resource_get_native(s_) \
    (((s_)->native_deleted)? &(s_)->native : NULL)

#define NDDSA_Resource_get_native_entity(s_,e_,t_,tfield_) \
{\
    NDDSA_ResourceNative *native_res_ = NULL;\
    if ((s_)->base.kind == (t_))\
    {\
        native_res_ = NDDSA_ResourceNative_get_native((s_));\
        if (native_res_ != NULL && \
            native_res_->kind == NDDSA_RESOURCENATIVEKIND_ENTITY)\
        {\
            (dp_) = native_res_->value.entity.tfield_;\
        }\
    }\
}

void
NDDSA_Resource_get_native_domain_participant(
    NDDSA_Resource *const self,
    DDS_DomainParticipant **const entity_out);

#define NDDSA_Resource_get_native_domain_participant(s_,e_) \
    NDDSA_Resource_get_native_entity(\
        (s_),(e_),D2S2_RESOURCEKIND_DOMAINPARTICIPANT,participant)

void
NDDSA_Resource_get_native_topic(
    NDDSA_Resource *const self,
    DDS_Topic **const entity_out);

#define NDDSA_Resource_get_native_topic(s_,e_) \
    NDDSA_Resource_get_native_entity((s_),(e_),D2S2_RESOURCEKIND_TOPIC,topic)

void
NDDSA_Resource_get_native_publisher(
    NDDSA_Resource *const self,
    DDS_Publisher **const entity_out);

#define NDDSA_Resource_get_native_publisher(s_,e_) \
    NDDSA_Resource_get_native_entity(\
        (s_),(e_),D2S2_RESOURCEKIND_PUBLISHER,publisher)

void
NDDSA_Resource_get_native_subscriber(
    NDDSA_Resource *const self,
    DDS_Subscriber **const entity_out);

#define NDDSA_Resource_get_native_subscriber(s_,e_) \
    NDDSA_Resource_get_native_entity(\
        (s_),(e_),D2S2_RESOURCEKIND_SUBSCRIBER,subscriber)

void
NDDSA_Resource_get_native_datawriter(
    NDDSA_Resource *const self,
    DDS_DataWriter **const entity_out);

#define NDDSA_Resource_get_native_datawriter(s_,e_) \
    NDDSA_Resource_get_native_entity(\
        (s_),(e_),D2S2_RESOURCEKIND_DATAWRITER,writer)

void
NDDSA_Resource_get_native_datareader(
    NDDSA_Resource *const self,
    DDS_DataReader **const entity_out);

#define NDDSA_Resource_get_native_datareader(s_,e_) \
    NDDSA_Resource_get_native_entity(\
        (s_),(e_),D2S2_RESOURCEKIND_DATAREADER,reader)

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsResource_h */
