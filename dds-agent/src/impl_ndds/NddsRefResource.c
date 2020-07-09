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

#include "NddsXmlResource.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

RTIBool
NDDSA_RefResource_lookup_application(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out)
{
    RTIBool retcode = RTI_FALSE;
    DDS_DomainParticipantFactory *factory = NULL;
    struct DDS_XMLObject *xml_root = NULL,
                         *xml_app = NULL;
    
    if (id->kind != D2S2_RESOURCEIDKIND_REF)
    {
        /* unsupported id type */
        goto done;
    }

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    DDS_DomainParticipantFactory_lockI(factory);

    xml_root = DDS_DomainParticipantFactory_get_xml_rootI(factory);
    if (xml_root == NULL)
    {
        DDS_DomainParticipantFactory_unlockI(factory);
        goto done;
    }

    xml_app = DDS_XMLObject_lookup(xml_root, id->value.ref);

    DDS_DomainParticipantFactory_unlockI(factory);
    
    *exists_out = (xml_app != NULL)? RTI_TRUE : RTI_FALSE;

    retcode = RTI_TRUE;
    
done:

    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_participant(
    const D2S2_ResourceId *const id,
    DDS_DomainParticipant **const participant_out)
{
    RTIBool retcode = RTI_FALSE;
    DDS_DomainParticipantFactory *factory = NULL;
    D2S2_EntityName dp_name = D2S2_ENTITYNAME_INITIALIZER;
    const char *dp_ref = NULL;

    if (id->kind != D2S2_RESOURCEIDKIND_REF)
    {
        /* unsupported id */
        goto done;
    }

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    if (!D2S2_EntityName_from_id(&dp_name, id))
    {
        goto done;
    }

    if (!D2S2_EntityName_component(
            &dp_name,
            D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
            &dp_ref))
    {
        goto done;
    }

    *participant_out = 
        DDS_DomainParticipantFactory_lookup_participant_by_name(
                factory, dp_ref);

    
    retcode = RTI_TRUE;
    
done:
    D2S2_EntityName_finalize(&dp_name);

    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_topic(
    const D2S2_ResourceId *const id,
    DDS_Topic **const topic_out)
{
    RTIBool retcode = RTI_FALSE;
    DDS_DomainParticipant *dp = NULL;
    D2S2_EntityName topic_name = D2S2_ENTITYNAME_INITIALIZER;
    D2S2_ResourceId dp_id = D2S2_RESOURCEID_INITIALIZER;
    char *dp_ref = NULL;
    const char *topic_ref = NULL;
    DDS_TopicDescription *topic_desc = NULL;

    *topic_out = NULL;

    if (!D2S2_EntityName_from_id(&topic_name, id))
    {
        goto done;
    }

    if (!D2S2_EntityName_to_ref(
            &topic_name, D2S2_ENTITYNAME_DEPTH_PARTICIPANT, &dp_ref))
    {
        goto done;
    }

    dp_id.kind = D2S2_RESOURCEIDKIND_REF;
    dp_id.value.ref = dp_ref;

    if (!NDDSA_RefResource_lookup_participant(&dp_id, &dp))
    {
        goto done;
    }
    if (dp == NULL)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    if (!D2S2_EntityName_component(
            &topic_name, D2S2_ENTITYNAME_DEPTH_TOPIC, &topic_ref))
    {
        goto done;
    }

    topic_desc = DDS_DomainParticipant_lookup_topicdescription(dp, topic_ref);
    if (topic_desc == NULL)
    {
        *topic_out = NULL;
    }
    else
    {
        *topic_out = DDS_Topic_narrow(topic_desc);
    }
    
    retcode = RTI_TRUE;
    
done:
    if (dp_ref != NULL)
    {
        RTIOsapiHeap_free(dp_ref);
    }
    D2S2_EntityName_finalize(&topic_name);

    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_publisher(
    const D2S2_ResourceId *const id,
    DDS_Publisher **const publisher_out)
{
    RTIBool retcode = RTI_FALSE;
    DDS_DomainParticipant *dp = NULL;
    D2S2_EntityName pub_name = D2S2_ENTITYNAME_INITIALIZER;
    D2S2_ResourceId dp_id = D2S2_RESOURCEID_INITIALIZER;
    char *dp_ref = NULL;
    const char *pub_ref = NULL;

    *publisher_out = NULL;

    if (!D2S2_EntityName_from_id(&pub_name, id))
    {
        goto done;
    }

    if (!D2S2_EntityName_to_ref(
            &pub_name, D2S2_ENTITYNAME_DEPTH_PARTICIPANT, &dp_ref))
    {
        goto done;
    }

    dp_id.kind = D2S2_RESOURCEIDKIND_REF;
    dp_id.value.ref = dp_ref;

    if (!NDDSA_RefResource_lookup_participant(&dp_id, &dp))
    {
        goto done;
    }
    if (dp == NULL)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    if (!D2S2_EntityName_component(
            &pub_name, D2S2_ENTITYNAME_DEPTH_PUBLISHER, &pub_ref))
    {
        goto done;
    }

    *publisher_out = 
        DDS_DomainParticipant_lookup_publisher_by_name(dp, pub_ref);
    
    retcode = RTI_TRUE;
    
done:
    if (dp_ref != NULL)
    {
        RTIOsapiHeap_free(dp_ref);
    }
    D2S2_EntityName_finalize(&pub_name);
    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_subscriber(
    const D2S2_ResourceId *const id,
    DDS_Subscriber **const subscriber_out)
{
    RTIBool retcode = RTI_FALSE;
    DDS_DomainParticipant *dp = NULL;
    D2S2_EntityName sub_name = D2S2_ENTITYNAME_INITIALIZER;
    D2S2_ResourceId dp_id = D2S2_RESOURCEID_INITIALIZER;
    char *dp_ref = NULL;
    const char *sub_ref = NULL;

    *subscriber_out = NULL;

    if (!D2S2_EntityName_from_id(&sub_name, id))
    {
        goto done;
    }

    if (!D2S2_EntityName_to_ref(
            &sub_name, D2S2_ENTITYNAME_DEPTH_PARTICIPANT, &dp_ref))
    {
        goto done;
    }

    dp_id.kind = D2S2_RESOURCEIDKIND_REF;
    dp_id.value.ref = dp_ref;

    if (!NDDSA_RefResource_lookup_participant(&dp_id, &dp))
    {
        goto done;
    }
    if (dp == NULL)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    if (!D2S2_EntityName_component(
            &sub_name, D2S2_ENTITYNAME_DEPTH_PUBLISHER, &sub_ref))
    {
        goto done;
    }

    *subscriber_out = 
        DDS_DomainParticipant_lookup_subscriber_by_name(dp, sub_ref);
    
    retcode = RTI_TRUE;
    
done:
    if (dp_ref != NULL)
    {
        RTIOsapiHeap_free(dp_ref);
    }
    D2S2_EntityName_finalize(&sub_name);
    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_datawriter(
    const D2S2_ResourceId *const id,
    DDS_DataWriter **const datawriter_out)
{
    RTIBool retcode = RTI_FALSE;
    DDS_Publisher *pub = NULL;
    D2S2_EntityName dw_name = D2S2_ENTITYNAME_INITIALIZER;
    D2S2_ResourceId pub_id = D2S2_RESOURCEID_INITIALIZER;
    char *pub_ref = NULL;
    const char *dw_ref = NULL;

    *datawriter_out = NULL;

    if (!D2S2_EntityName_from_id(&dw_name, id))
    {
        goto done;
    }

    if (!D2S2_EntityName_to_ref(
            &dw_name, D2S2_ENTITYNAME_DEPTH_PUBLISHER, &pub_ref))
    {
        goto done;
    }

    pub_id.kind = D2S2_RESOURCEIDKIND_REF;
    pub_id.value.ref = pub_ref;

    if (!NDDSA_RefResource_lookup_publisher(&pub_id, &pub))
    {
        goto done;
    }
    if (pub == NULL)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    if (!D2S2_EntityName_component(
            &dw_name, D2S2_ENTITYNAME_DEPTH_DATAWRITER, &dw_ref))
    {
        goto done;
    }

    *datawriter_out = 
        DDS_Publisher_lookup_datawriter_by_name(pub, dw_ref);
    
    retcode = RTI_TRUE;
    
done:
    if (pub_ref != NULL)
    {
        RTIOsapiHeap_free(pub_ref);
    }
    D2S2_EntityName_finalize(&dw_name);
    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_datareader(
    const D2S2_ResourceId *const id,
    DDS_DataReader **const datareader_out)
{
    RTIBool retcode = RTI_FALSE;
    DDS_Subscriber *sub = NULL;
    D2S2_EntityName dr_name = D2S2_ENTITYNAME_INITIALIZER;
    D2S2_ResourceId sub_id = D2S2_RESOURCEID_INITIALIZER;
    char *sub_ref = NULL;
    const char *dr_ref = NULL;

    *datareader_out =  NULL;

    if (!D2S2_EntityName_from_id(&dr_name, id))
    {
        goto done;
    }

    if (!D2S2_EntityName_to_ref(
            &dr_name, D2S2_ENTITYNAME_DEPTH_SUBSCRIBER, &sub_ref))
    {
        goto done;
    }

    sub_id.kind = D2S2_RESOURCEIDKIND_REF;
    sub_id.value.ref = sub_ref;

    if (!NDDSA_RefResource_lookup_subscriber(&sub_id, &sub))
    {
        goto done;
    }
    if (sub == NULL)
    {
        retcode = RTI_TRUE;
        goto done;
    }

    if (!D2S2_EntityName_component(
            &dr_name, D2S2_ENTITYNAME_DEPTH_DATAREADER, &dr_ref))
    {
        goto done;
    }
    
    *datareader_out = 
        DDS_Subscriber_lookup_datareader_by_name(sub, dr_ref);
    
    retcode = RTI_TRUE;
    
done:
    if (sub_ref != NULL)
    {
        RTIOsapiHeap_free(sub_ref);
    }
    D2S2_EntityName_finalize(&dr_name);
    return retcode;
}


RTIBool
NDDSA_RefResource_lookup_domain(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out)
{
    RTIBool retcode = RTI_FALSE;
    UNUSED_ARG(id);
    UNUSED_ARG(exists_out);
    
    retcode = RTI_TRUE;
    
    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_qosprofile(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out)
{
    RTIBool retcode = RTI_FALSE;
    UNUSED_ARG(id);
    UNUSED_ARG(exists_out);
    retcode = RTI_TRUE;
    return retcode;
}

RTIBool
NDDSA_RefResource_lookup_type(
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out)
{
    RTIBool retcode = RTI_FALSE;
    UNUSED_ARG(id);
    UNUSED_ARG(exists_out);
    retcode = RTI_TRUE;
    return retcode;
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

