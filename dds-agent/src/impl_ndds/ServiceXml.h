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

#ifndef ServiceXml_h
#define ServiceXml_h

#include "dds_agent/dds_agent.h"

#include "NddsResourceNative.h"

#define NDDSA_SERVICE_XML_TAG             "external_service"
#define NDDSA_SERVICE_DESCRIPTOR_XML_TAG  "service_descriptor"
#define NDDSA_SERVICE_RESOURCE_XML_TAG    "service_resource"

typedef struct NDDSA_ServiceDescriptorXmlI
{
    struct DDS_XMLObject base;
    char * data;
} NDDSA_ServiceDescriptorXml;


typedef struct NDDSA_ServiceXmlI
{
    struct DDS_XMLObject base;
    NDDSA_ExternalService entity;
} NDDSA_ServiceXml;

typedef struct NDDSA_ServiceResourceXmlI
{
    struct DDS_XMLObject base;
    NDDSA_ExternalServiceResource entity;
} NDDSA_ServiceResourceXml;

RTIBool
NDDSA_ServiceXml_register_extension(
    struct DDS_XMLParser *const xml_parser);

RTIBool
NDDSA_ServiceXml_unregister_extension(
    struct DDS_XMLParser *const xml_parser);

#endif /* NddsAgentXml_h */