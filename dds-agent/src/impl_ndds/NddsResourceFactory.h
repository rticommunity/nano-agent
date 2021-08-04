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

#ifndef NddsResourceFactory_h
#define NddsResourceFactory_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsInfrastructure.h"
#include "NddsResourceNative.h"

typedef struct NDDSA_CreatedResourceLogI
{
    D2S2_ResourceId id;
    D2S2_ResourceKind kind;
    void * data;
} NDDSA_CreatedResourceLog;

#define NDDSA_CREATEDRESOURCELOG_INITIALIZER \
{\
    D2S2_RESOURCEID_INITIALIZER, /* id */\
    D2S2_RESOURCEKIND_UNKNOWN, /* kind */\
    NULL /* data */\
}

DDS_SEQUENCE(NDDSA_CreatedResourceLogSeq, NDDSA_CreatedResourceLog);


typedef struct NDDSA_ResourceFactoryI
{
    struct DDS_XMLParser *xml_parser;
    struct RTIOsapiSemaphore *mutex;
} NDDSA_ResourceFactory;


#define NDDSA_RESOURCEFACTORY_INITIALIZER \
{\
    NULL, /* xml_parser */\
    NULL /* mutex */\
}

void
NDDSA_ResourceFactory_enter_ea(
    NDDSA_ResourceFactory *const self);

#define NDDSA_ResourceFactory_enter_ea(s_) {}

void
NDDSA_ResourceFactory_leve_ea(
    NDDSA_ResourceFactory *const self);

#define NDDSA_ResourceFactory_leave_ea(s_) {}


RTIBool
NDDSA_ResourceFactory_initialize(
    NDDSA_ResourceFactory *const self);

void
NDDSA_ResourceFactory_finalize(
    NDDSA_ResourceFactory *const self);

RTIBool
NDDSA_ResourceFactory_create_resource_native(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceRepresentation *const repr,
    const D2S2_ResourceId *const parent,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const created_out);

// RTIBool
// NDDSA_ResourceFactory_delete_entity_resource(
//     NDDSA_ResourceFactory *const self,
//     const D2S2_ResourceKind kind,
//     const D2S2_ResourceId *const id);

RTIBool
NDDSA_ResourceFactory_lookup_entity_resource(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out,
    NDDSA_EntityResource *const entity_out);

void
NDDSA_ResourceFactory_unload_resource_xml(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id);


#include "NddsXmlVisitor.h"

typedef struct NDDSA_XmlResourceVisitorI
{
    NDDSA_XmlVisitor base;
    struct NDDSA_CreatedResourceLogSeq *result_seq;
    RTIBool error;
    DDS_DomainParticipantFactory *factory;
    D2S2_ResourceProperties res_properties;
} NDDSA_XmlResourceVisitor;

#define NDDSA_XMLRESOURCEVISITOR_INITIALIZER \
{\
    NDDSA_XMLVISITOR_INITIALIZER, /* base */\
    NULL, /* result_seq */\
    RTI_FALSE, /* error */\
    NULL, /* factory */\
    D2S2_RESOURCEPROPERTIES_INITIALIZER /* res_properties */\
}

void
NDDSA_XmlResourceVisitor_initialize(
    NDDSA_XmlResourceVisitor *const self,
    const D2S2_ResourceProperties *const res_properties,
    struct NDDSA_CreatedResourceLogSeq *const created_resources);

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsResourceFactory_h */