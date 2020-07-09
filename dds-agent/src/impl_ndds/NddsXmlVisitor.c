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

#include "NddsXmlVisitor.h"

typedef RTIBool
(*NDDSA_Agent_VisitXmlFn)(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const obj);

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_children(
    NDDSA_Agent *const agent,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const parent,
    const char *tag_name,
    NDDSA_Agent_VisitXmlFn recur_visit)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_children)
    RTIBool retcode = RTI_FALSE;
    struct DDS_XMLObject *child = NULL;

    D2S2Log_fn_entry()

    child = DDS_XMLObject_get_first_child_with_tag(parent, tag_name);

    while (child != NULL)
    {
        if (!recur_visit(agent, visitor, child))
        {
            goto done;
        }

        child = DDS_XMLObject_get_next_sibling_with_tag(child, tag_name);
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}


RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_topic(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const topic)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_topic)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()
    
    if (!NDDSA_XmlVisitor_on_topic(visitor, self, topic))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_datareader(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const reader)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_datareader)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()
    
    if (!NDDSA_XmlVisitor_on_datareader(visitor, self, reader))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_datawriter(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const writer)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_datawriter)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    if (!NDDSA_XmlVisitor_on_datawriter(visitor, self, writer))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_subscriber(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const subscriber)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_subscriber)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    if (!NDDSA_XmlVisitor_on_subscriber(visitor, self, subscriber))
    {
        goto done;
    }
    if (!NDDSA_Agent_visit_xml_children(
            self, visitor, subscriber, "data_reader", NDDSA_Agent_visit_xml_datareader))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_publisher(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const publisher)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_publisher)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    if (!NDDSA_XmlVisitor_on_publisher(visitor, self, publisher))
    {
        goto done;
    }

    if (!NDDSA_Agent_visit_xml_children(
            self, visitor, publisher, "data_writer", NDDSA_Agent_visit_xml_datawriter))
    {
        goto done;
    }


    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_domain_participant(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const participant)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_domain_participant)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    if (!NDDSA_XmlVisitor_on_participant(visitor, self, participant))
    {
        goto done;
    }

    if (!NDDSA_Agent_visit_xml_children(
            self, visitor, participant, "publisher", NDDSA_Agent_visit_xml_publisher))
    {
        goto done;
    }

    if (!NDDSA_Agent_visit_xml_children(
            self, visitor, participant, "subscriber", NDDSA_Agent_visit_xml_subscriber))
    {
        goto done;
    }

    if (!NDDSA_Agent_visit_xml_children(
            self, visitor, participant, "topic", NDDSA_Agent_visit_xml_topic))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_xml_domain_participant_library(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor,
    struct DDS_XMLObject *const lib)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml_domain_participant_library)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()
    
    if (!NDDSA_Agent_visit_xml_children(
            self, visitor, lib, "domain_participant", NDDSA_Agent_visit_xml_domain_participant))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_Agent_visit_xml(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor)
{
    D2S2Log_METHOD_NAME(NDDSA_Agent_visit_xml)
    RTIBool retcode = RTI_FALSE;
    DDS_DomainParticipantFactory *factory = NULL;
    struct DDS_XMLObject *xml_root = NULL;

    D2S2Log_fn_entry()

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    xml_root = DDS_DomainParticipantFactory_get_xml_rootI(factory);
    if (xml_root == NULL)
    {
        goto done;
    }

    if (!NDDSA_Agent_visit_xml_children(
            self, visitor, xml_root, 
            "domain_participant_library",
            NDDSA_Agent_visit_xml_domain_participant_library))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}