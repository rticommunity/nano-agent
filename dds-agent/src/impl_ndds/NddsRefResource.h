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

#ifndef NddsRefResource_h
#define NddsRefResource_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsInfrastructure.h"


RTIBool
NDDSA_RefResource_lookup_application(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out);

RTIBool
NDDSA_RefResource_lookup_participant(
    const D2S2_ResourceId *const id,
    DDS_DomainParticipant **const participant_out);

RTIBool
NDDSA_RefResource_lookup_topic(
    const D2S2_ResourceId *const id,
    DDS_Topic **const topic_out);

RTIBool
NDDSA_RefResource_lookup_publisher(
    const D2S2_ResourceId *const id,
    DDS_Publisher **const publisher_out);

RTIBool
NDDSA_RefResource_lookup_subscriber(
    const D2S2_ResourceId *const id,
    DDS_Subscriber **const subscriber_out);

RTIBool
NDDSA_RefResource_lookup_datawriter(
    const D2S2_ResourceId *const id,
    DDS_DataWriter **const datawriter_out);

RTIBool
NDDSA_RefResource_lookup_datareader(
    const D2S2_ResourceId *const id,
    DDS_DataReader **const datareader_out);

RTIBool
NDDSA_RefResource_lookup_domain(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out);

RTIBool
NDDSA_RefResource_lookup_qosprofile(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out);

RTIBool
NDDSA_RefResource_lookup_type(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out);

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsRefResource_h */