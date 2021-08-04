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

#ifndef NddsInfrastructure_h
#define NddsInfrastructure_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#define NDDSA_OSAPI_MEMBER_OFFSET(TYPE, ELEMENT) \
    ((size_t)&(((TYPE *)0)->ELEMENT))

typedef struct NDDSA_AgentI NDDSA_Agent;
typedef struct NDDSA_ClientSessionI NDDSA_ClientSession;

/******************************************************************************
 *                               D2S2_Buffer
 ******************************************************************************/
RTIBool
D2S2_Buffer_initialize(
    D2S2_Buffer *const self,
    unsigned char *const data,
    const DDS_UnsignedLong data_len);

void
D2S2_Buffer_finalize(D2S2_Buffer *const self);

RTIBool
D2S2_Buffer_copy(
    D2S2_Buffer *const dst,
    const D2S2_Buffer *const src);

int
D2S2_Buffer_compare(
    D2S2_Buffer *const left, 
    const D2S2_Buffer *const right);

/******************************************************************************
 *                               D2S2_XcdrData
 ******************************************************************************/
RTIBool
D2S2_XcdrData_initialize(
    D2S2_XcdrData *const self,
    const D2S2_XcdrEncodingKind encoding,
    const D2S2_Buffer *const buffer);

void
D2S2_XcdrData_finalize(D2S2_XcdrData *const self);

RTIBool
D2S2_XcdrData_copy(
    D2S2_XcdrData *const dst,
    const D2S2_XcdrData *const src);

int
D2S2_XcdrData_compare(
    D2S2_XcdrData *const left,
    const D2S2_XcdrData *const right);

/******************************************************************************
 *                          D2S2_ResourceRepresentation
 ******************************************************************************/


/******************************************************************************
 *                          D2S2_ResourceId
 ******************************************************************************/

RTIBool
D2S2_ResourceId_initialize_ref(
    D2S2_ResourceId *const self,
    const char *const ref);

RTIBool
D2S2_ResourceId_initialize_guid(
    D2S2_ResourceId *const self,
    const DDS_GUID_t *const guid);

void
D2S2_ResourceId_finalize(D2S2_ResourceId *const self);

RTIBool
D2S2_ResourceId_copy(
    D2S2_ResourceId *const dst,
    const D2S2_ResourceId *const src);

int
D2S2_ResourceId_compare(
    const D2S2_ResourceId *const left,
    const D2S2_ResourceId *const right);

RTIBool
D2S2_ResourceId_as_ref(
    const D2S2_ResourceId *const id,
    const char **const ref_out);

/******************************************************************************
 *                          D2S2_Resource
 ******************************************************************************/

RTIBool
D2S2_Resource_initialize(
    D2S2_Resource *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceId *const id);

void
D2S2_Resource_finalize(D2S2_Resource *const self);

RTIBool
D2S2_Resource_copy(
    D2S2_Resource *const dst,
    const D2S2_Resource *const src);

int
D2S2_Resource_compare(
    const D2S2_Resource *const left,
    const D2S2_Resource *const right);

/******************************************************************************
 * String utils
 ******************************************************************************/

RTIBool
NDDSA_StringUtil_append(
    const char *const s1,
    const char *const sep,
    const char *const s2,
    char **const str_out);

#define NDDSA_StringUtil_create_ref(s1_,s2_,str_out_) \
    NDDSA_StringUtil_append((s1_),"::",(s2_),(str_out_))


#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsInfrastructure_h */