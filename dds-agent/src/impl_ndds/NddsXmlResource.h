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

#ifndef NddsXmlResource_h
#define NddsXmlResource_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsResourceFactory.h"

RTIBool
NDDSA_ResourceFactory_create_application_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const app_lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const created_entities_out,
    struct NDDSA_CreatedResourceLogSeq *const participants_out,
    char **const app_lib_name_out);

RTIBool
NDDSA_ResourceFactory_create_participant_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const dp_lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const created_entities_out,
    char **const dp_lib_name_out);

RTIBool
NDDSA_ResourceFactory_create_topic_xml(
    NDDSA_ResourceFactory *const self,
    const char *const topic_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const participant,
    NDDSA_CreatedResourceLog *const topic_out);

RTIBool
NDDSA_ResourceFactory_create_publisher_xml(
    NDDSA_ResourceFactory *const self,
    const char *const pub_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const participant,
    NDDSA_CreatedResourceLog *const pub_out);

RTIBool
NDDSA_ResourceFactory_create_subscriber_xml(
    NDDSA_ResourceFactory *const self,
    const char *const sub_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const participant,
    NDDSA_CreatedResourceLog *const sub_out);

RTIBool
NDDSA_ResourceFactory_create_datawriter_xml(
    NDDSA_ResourceFactory *const self,
    const char *const writer_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const publisher,
    NDDSA_CreatedResourceLog *const writer_out);

RTIBool
NDDSA_ResourceFactory_create_datareader_xml(
    NDDSA_ResourceFactory *const self,
    const char *const reader_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const subscriber,
    NDDSA_CreatedResourceLog *const reader_out);

RTIBool
NDDSA_ResourceFactory_create_types_xml(
    NDDSA_ResourceFactory *const self,
    const char *const types_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const types_out);

RTIBool
NDDSA_ResourceFactory_create_domain_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const domain_lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const domains_out);

RTIBool
NDDSA_ResourceFactory_create_qos_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const qos_lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const qos_profiles_out);

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsXmlResource_h */