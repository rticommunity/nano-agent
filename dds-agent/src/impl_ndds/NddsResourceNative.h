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

#ifndef NddsResourceNative_h
#define NddsResourceNative_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsInfrastructure.h"
#include "NddsExternalService.h"

typedef struct NDDSA_ResourceI NDDSA_Resource;

typedef struct NDDSA_GenericResourceI
{
    D2S2_ResourceId id;
} NDDSA_GenericResource;

#define NDDSA_GENERICRESOURCE_INITIALIZER \
{\
    D2S2_RESOURCEID_INITIALIZER /* guid */\
}

RTIBool
NDDSA_GenericResource_initialize(
    NDDSA_GenericResource *self,
    const D2S2_ResourceId *const id);

void
NDDSA_GenericResource_finalize(NDDSA_GenericResource *self);

RTIBool
NDDSA_GenericResource_copy(
    NDDSA_GenericResource *const dst,
    const NDDSA_GenericResource *const src);

typedef union NDDSA_EntityResourceI
{
    DDS_DomainParticipant *participant;
    DDS_Topic *topic;
    DDS_Publisher *publisher;
    DDS_Subscriber *subscriber;
    DDS_DataWriter *writer;
    DDS_DataReader *reader;
    NDDSA_ExternalService *service;
    NDDSA_ExternalServiceResource *service_resource;
} NDDSA_EntityResource;

#define NDDSA_ENTITYRESOURCE_INITIALIZER \
{\
    NULL /* participant */\
}

typedef union NDDSA_ResourceNativeValueI
{
    NDDSA_GenericResource generic;
    NDDSA_EntityResource entity;
} NDDSA_ResourceNativeValue;

#define NDDSA_RESOURCENATIVEVALUE_INITIALIZER \
{\
    NDDSA_GENERICRESOURCE_INITIALIZER /* generic */\
}

typedef enum NDDSA_ResourceNativeKindI
{
    NDDSA_RESOURCENATIVEKIND_UNKNOWN,\
    NDDSA_RESOURCENATIVEKIND_ENTITY,\
    NDDSA_RESOURCENATIVEKIND_GENERIC\
} NDDSA_ResourceNativeKind;

typedef struct NDDSA_ResourceNativeI
{
    NDDSA_ResourceNativeKind kind;
    NDDSA_ResourceNativeValue value;
    D2S2_ResourceRepresentation create_repr;
} NDDSA_ResourceNative;

#define NDDSA_RESOURCENATIVE_INITIALIZER \
{\
    NDDSA_RESOURCENATIVEKIND_UNKNOWN, /* kind */\
    NDDSA_RESOURCENATIVEVALUE_INITIALIZER, /* value */\
    D2S2_RESOURCEREPRESENTATION_INITIALIZER /* repr */\
}

RTIBool
NDDSA_ResourceNative_initialize_entity(
    NDDSA_ResourceNative *const self,
    const NDDSA_EntityResource *const entity,
    const D2S2_ResourceRepresentation *const create_repr);

RTIBool
NDDSA_ResourceNative_initialize_generic(
    NDDSA_ResourceNative *const self,
    const NDDSA_GenericResource *const generic,
    const D2S2_ResourceRepresentation *const create_repr);

void
NDDSA_ResourceNative_finalize(NDDSA_ResourceNative *const self);

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsResourceNative_h */