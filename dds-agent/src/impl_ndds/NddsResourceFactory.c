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

#include "NddsResourceFactory.h"
#include "ApplicationLibraryXml.h"
#include "ServiceXml.h"
#include "NddsAgentDtd.h"

#include "NddsXmlResource.h"
#include "NddsRefResource.h"
#include "NddsDcpsVisitor.h"
#include "NddsAgent.h"
#include "NddsAgentDb.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT


RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_lookup_entity_resourceEA(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out,
    NDDSA_EntityResource *const entity_out);

RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_initialize_xml_parser(
    NDDSA_ResourceFactory *const self);


/******************************************************************************
 *                          NDDSA_CreatedResourceLog
 ******************************************************************************/


RTI_PRIVATE
RTIBool
NDDSA_CreatedResourceLog_initialize_w_params(
    NDDSA_CreatedResourceLog *self,
    const struct DDS_TypeAllocationParams_t * allocParams)
{
    NDDSA_CreatedResourceLog def_self = NDDSA_CREATEDRESOURCELOG_INITIALIZER;
    UNUSED_ARG(allocParams);
    *self = def_self;
    return RTI_TRUE;
}

RTI_PRIVATE
RTIBool
NDDSA_CreatedResourceLog_finalize_w_params(
    NDDSA_CreatedResourceLog *self,
    const struct DDS_TypeDeallocationParams_t * deallocParams)
{
    UNUSED_ARG(deallocParams);
    D2S2_ResourceId_finalize(&self->id);
    return RTI_TRUE;
}

RTI_PRIVATE
RTIBool
NDDSA_CreatedResourceLog_copy(
    NDDSA_CreatedResourceLog *const dst,
    const NDDSA_CreatedResourceLog *const src)
{
    D2S2Log_METHOD_NAME(NDDSA_CreatedResourceLog)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    dst->kind = src->kind;
    retcode = D2S2_ResourceId_copy(&dst->id, &src->id);

    D2S2Log_fn_exit()
    return retcode;
}

#define T                       NDDSA_CreatedResourceLog
#define TSeq                    NDDSA_CreatedResourceLogSeq
#define T_initialize_w_params   NDDSA_CreatedResourceLog_initialize_w_params
#define T_finalize_w_params     NDDSA_CreatedResourceLog_finalize_w_params
#define T_copy                  NDDSA_CreatedResourceLog_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"

/******************************************************************************
 * Function implementations
 ******************************************************************************/

RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_initialize_xml_parser(
    NDDSA_ResourceFactory *const self)
{
    RTIBool retcode = RTI_FALSE;
    struct DDS_DomainParticipantFactoryQos factory_qos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;
    DDS_DomainParticipantFactory *factory = NULL;
    DDS_QosProvider *qos_provider = NULL;
    
    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_get_qos(factory, &factory_qos))
    {
        goto done;
    }
    // printf("---- AGENT DTD ----\n");
    // {
    //     size_t i = 0;
    //     for (i = 0; i < NDDSA_AGENT_DTD_SIZE; i++)
    //     {
    //       printf("%s", NDDSA_AGENT_DTD[i]);
    //     }
        
    // }
    // printf("---- //AGENT DTD ----\n");
    DDS_StringSeq_from_array(
            &factory_qos.profile.string_profile_dtd,
            NDDSA_AGENT_DTD,
            NDDSA_AGENT_DTD_SIZE);
    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_set_qos(factory, &factory_qos))
    {
        goto done;
    }

    qos_provider =
            DDS_DomainParticipantFactory_get_qos_providerI(factory);
    if (qos_provider == NULL)
    {
        goto done;
    }

    /* Get pointer to the internal DDS XML Parser */
    self->xml_parser = DDS_QosProvider_get_xml_parser(qos_provider);
    if (self->xml_parser == NULL)
    {
        goto done;
    }

    if (!NDDSA_ApplicationLibraryXml_register_extension(self->xml_parser))
    {
        goto done;
    }

    if (!NDDSA_ServiceXml_register_extension(self->xml_parser))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactoryQos_finalize(&factory_qos))
    {
        /* TODO log */
    }
    return retcode;
}


RTIBool
NDDSA_ResourceFactory_initialize(
    NDDSA_ResourceFactory *const self)
{
    RTIBool retcode = RTI_FALSE;

    /* Initialize configuration parser */
    if (!NDDSA_ResourceFactory_initialize_xml_parser(self))
    {
        goto done;
    }
    
    self->mutex = RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_MUTEX, NULL);
    if (self->mutex == NULL)
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    return retcode;
}

void
NDDSA_ResourceFactory_finalize(
    NDDSA_ResourceFactory *const self)
{
    
    NDDSA_ApplicationLibraryXml_unregister_extension(self->xml_parser);
    RTIOsapiSemaphore_delete(self->mutex);
}

RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_create_resource_native_xml(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const char *const xml_repr,
    const D2S2_ResourceId *const parent,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const created_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_resource_native_xml)
    RTIBool retcode = RTI_FALSE;
    char *lib_name = NULL;
    NDDSA_CreatedResourceLog *created_id = NULL;
    struct NDDSA_CreatedResourceLogSeq inner_entities = DDS_SEQUENCE_INITIALIZER;

    D2S2Log_fn_entry()

    switch (kind)
    {
    case D2S2_RESOURCEKIND_APPLICATION:
    {
        if (parent != NULL)
        {
            /* we don't expect a parent for this type of entity */
            goto done;
        }
        retcode = 
            NDDSA_ResourceFactory_create_application_library_xml(
                self,
                xml_repr,
                properties,
                created_out,
                &inner_entities,
                &lib_name);
        break;
    }
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        if (parent != NULL)
        {
            /* we don't expect a parent for this type of entity */
            goto done;
        }
        retcode =
            NDDSA_ResourceFactory_create_participant_library_xml(
                self,
                xml_repr,
                properties,
                created_out,
                &lib_name);
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        if (parent == NULL)
        {
            /* we need a parent for this type of entity */
            goto done;
        }
        if (!NDDSA_CreatedResourceLogSeq_ensure_length(created_out, 1, 1))
        {
            goto done;
        }

        created_id = NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0);
        if (created_id == NULL)
        {
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_topic_xml(
                        self, xml_repr, properties, parent, created_id);
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        if (parent == NULL)
        {
            /* we need a parent for this type of entity */
            goto done;
        }
        if (!NDDSA_CreatedResourceLogSeq_ensure_length(created_out, 1, 1))
        {
            goto done;
        }

        created_id = NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0);
        if (created_id == NULL)
        {
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_publisher_xml(
                        self, xml_repr, properties, parent, created_id);
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        if (parent == NULL)
        {
            /* we need a parent for this type of entity */
            goto done;
        }
        if (!NDDSA_CreatedResourceLogSeq_ensure_length(created_out, 1, 1))
        {
            goto done;
        }

        created_id = NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0);
        if (created_id == NULL)
        {
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_subscriber_xml(
                        self, xml_repr, properties, parent, created_id);
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        if (parent == NULL)
        {
            /* we need a parent for this type of entity */
            goto done;
        }
        if (!NDDSA_CreatedResourceLogSeq_ensure_length(created_out, 1, 1))
        {
            goto done;
        }

        created_id = NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0);
        if (created_id == NULL)
        {
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_datawriter_xml(
                        self, xml_repr, properties, parent, created_id);
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        if (parent == NULL)
        {
            /* we need a parent for this type of entity */
            goto done;
        }
        if (!NDDSA_CreatedResourceLogSeq_ensure_length(created_out, 1, 1))
        {
            goto done;
        }

        created_id = NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0);
        if (created_id == NULL)
        {
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_datareader_xml(
                        self, xml_repr, properties, parent, created_id);
        break;
    }
    case D2S2_RESOURCEKIND_DOMAIN:
    {
        if (parent != NULL)
        {
            /* we don't expect a parent for this type of entity */
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_domain_library_xml(
                        self, xml_repr, properties, created_out);
        break;
    }
    case D2S2_RESOURCEKIND_QOSPROFILE:
    {
        if (parent != NULL)
        {
            /* we don't expect a parent for this type of entity */
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_qos_library_xml(
                    self, xml_repr, properties, created_out);
        break;
    }
    case D2S2_RESOURCEKIND_TYPE:
    {
        if (parent != NULL)
        {
            /* we don't expect a parent for this type of entity */
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_types_xml(
                    self, xml_repr, properties, created_out);
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE:
    {
        if (parent != NULL)
        {
            /* we don't expect a parent for this type of entity */
            goto done;
        }
        retcode = NDDSA_ResourceFactory_create_service_xml(
            self, xml_repr, properties, created_out, &inner_entities);
        break;
    }
    default:
    {
        goto done;
    }
    }

    if (!retcode)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_s,
            "FAILED to create native resource");
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (lib_name != NULL)
    {
        RTIOsapiHeap_free(lib_name);
    }

    /* We currently don't do anything with the list of references of
       "inner entities" created as side-effect when creating another entity */
    NDDSA_CreatedResourceLogSeq_finalize(&inner_entities);

    if (!retcode)
    {
        if (!NDDSA_CreatedResourceLogSeq_set_length(created_out, 0))
        {
            goto done;
        }
    }
    else
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "CREATED new resource by XML: ",
            NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0)->id.value.ref);
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_create_resource_native_ref(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const char *const ref_repr,
    const D2S2_ResourceId *const parent,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const created_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_resource_native_ref)
    RTIBool retcode = RTI_FALSE,
            resource_exists = RTI_FALSE;
    NDDSA_CreatedResourceLog *created_id = NULL;
    D2S2_ResourceId resource_id = D2S2_RESOURCEID_INITIALIZER;
    struct NDDSA_CreatedResourceLogSeq inner_entities = DDS_SEQUENCE_INITIALIZER;
    char *full_ref = NULL;
    void *resource_data =  NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    if (parent == NULL)
    {
        resource_id.kind = D2S2_RESOURCEIDKIND_REF;
        resource_id.value.ref = (char*) ref_repr;
    }
    else
    {
        if (!NDDSA_StringUtil_create_ref(
                parent->value.ref,
                ref_repr,
                &full_ref))
        {
            goto done;
        }
        resource_id.kind = D2S2_RESOURCEIDKIND_REF;
        resource_id.value.ref = full_ref;
    }


    switch (kind)
    {
    case D2S2_RESOURCEKIND_APPLICATION:
    {
        if (!NDDSA_RefResource_lookup_application(
                        &resource_id, &resource_exists))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        DDS_DomainParticipant *dp = NULL;
        if (!NDDSA_RefResource_lookup_participant(&resource_id, &dp))
        {
            goto done;
        }
        resource_exists = (dp != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        DDS_Topic *topic = NULL;
        if (!NDDSA_RefResource_lookup_topic(&resource_id, &topic))
        {
            goto done;
        }
        resource_exists = (topic != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        DDS_Publisher *pub = NULL;
        if (!NDDSA_RefResource_lookup_publisher(&resource_id, &pub))
        {
            goto done;
        }
        resource_exists = (pub != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        DDS_Subscriber *sub = NULL;
        if (!NDDSA_RefResource_lookup_subscriber(&resource_id, &sub))
        {
            goto done;
        }
        resource_exists = (sub != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        DDS_DataWriter *dw = NULL;
        if (!NDDSA_RefResource_lookup_datawriter(&resource_id, &dw))
        {
            goto done;
        }
        resource_exists = (dw != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        DDS_DataReader *dr = NULL;
        if (!NDDSA_RefResource_lookup_datareader(&resource_id, &dr))
        {
            goto done;
        }
        resource_exists = (dr != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_DOMAIN:
    {
        if (!NDDSA_RefResource_lookup_domain(
                        &resource_id, &resource_exists))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_QOSPROFILE:
    {
        if (!NDDSA_RefResource_lookup_qosprofile(
                        &resource_id, &resource_exists))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_TYPE:
    {
        if (!NDDSA_RefResource_lookup_type(
                        &resource_id, &resource_exists))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE:
    {
        if (!NDDSA_RefResource_lookup_service(&resource_id, &resource_data))
        {
            goto done;
        }
        resource_exists = (resource_data != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
    {
        if (!NDDSA_RefResource_lookup_service_resource(&resource_id, &resource_data))
        {
            goto done;
        }
        resource_exists = (resource_data != NULL);
        break;
    }
    default:
    {
        goto done;
    }
    }

    if (resource_exists)
    {
        if (!NDDSA_CreatedResourceLogSeq_ensure_length(created_out, 1, 1))
        {
            goto done;
        }

        created_id = NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0);
        if (created_id == NULL)
        {
            goto done;
        }
        created_id->kind = kind;
        created_id->data = resource_data;
        if (!D2S2_ResourceId_initialize_ref(
                &created_id->id, resource_id.value.ref))
        {
            goto done;
        }
    }
    else
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    
    /* We currently don't do anything with the list of references of
       "inner entities" created as side-effect when creating another entity */
    NDDSA_CreatedResourceLogSeq_finalize(&inner_entities);

    if (!retcode)
    {
        if (!NDDSA_CreatedResourceLogSeq_set_length(created_out, 0))
        {
            goto done;
        }
    }
    else
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "CREATED new resource by REF: ",
            NDDSA_CreatedResourceLogSeq_get_reference(created_out, 0)->id.value.ref);
    }
    if (full_ref != NULL)
    {
        RTIOsapiHeap_freeString(full_ref);
    }
    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_ResourceFactory_create_resource_native(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceRepresentation *const repr,
    const D2S2_ResourceId *const parent,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const created_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_resource_native)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    NDDSA_ResourceFactory_enter_ea(self);

    // printf("*** CREATE NATIVE RESOURCE: kind=%s, repr=%s, repr_fmt=%02X parent=%s\n",
    //     D2S2_ResourceKind_to_str(kind),
    //     (repr->fmt == D2S2_RESOURCEREPRESENTATIONFORMAT_REF)?
    //         repr->value.ref : repr->value.xml,
    //     repr->fmt,
    //     (parent != NULL)? parent->value.ref : NULL);

    switch (repr->fmt)
    {
    case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
    {
        retcode =
            NDDSA_ResourceFactory_create_resource_native_xml(
                self, kind, repr->value.xml, parent, properties, created_out);
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
    {
        retcode =
            NDDSA_ResourceFactory_create_resource_native_ref(
                self, kind, repr->value.ref, parent, properties, created_out);
        break;
    }
    default:
    {
        /* unsupported representation format */
        goto done;
    }
    }

    if (!retcode)
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:

    NDDSA_ResourceFactory_leave_ea(self);
    D2S2Log_fn_exit()
    return retcode;
}

// RTIBool
// NDDSA_ResourceFactory_delete_entity_resource(
//     NDDSA_ResourceFactory *const self,
//     const D2S2_ResourceKind kind,
//     const D2S2_ResourceId *const id)
// {
//     RTIBool retcode = RTI_FALSE,
//             exists = RTI_FALSE,
//             parent_exists = RTI_FALSE,
//             initd_resource_name = RTI_FALSE;
//     NDDSA_EntityResource entity_res = NDDSA_ENTITYRESOURCE_INITIALIZER,
//                          parent_res = NDDSA_ENTITYRESOURCE_INITIALIZER;
//     DDS_DomainParticipantFactory *factory = NULL;
//     D2S2_EntityName resource_name = D2S2_ENTITYNAME_INITIALIZER;
//     char *participant_name = NULL,
//          *publisher_name = NULL,
//          *subscriber_name = NULL;
//     D2S2_ResourceId parent_id = D2S2_RESOURCEID_INITIALIZER;
//     D2S2_ResourceKind parent_kind = D2S2_RESOURCEKIND_UNKNOWN;

//     NDDSA_ResourceFactory_enter_ea(self);

//     if (!D2S2_EntityName_from_id(&resource_name, id))
//     {
//         goto done;
//     }
//     initd_resource_name = RTI_TRUE;

//     factory = DDS_DomainParticipantFactory_get_instance();
//     if (factory == NULL)
//     {
//         goto done;
//     }

//     if (!NDDSA_ResourceFactory_lookup_entity_resourceEA(
//             self,
//             kind,
//             id,
//             &exists,
//             &entity_res))
//     {
//         goto done;
//     }

//     if (!exists)
//     {
//         /* Resource doesn't exists, so there's nothing left to do.
//            It might have been already deleted by some other means. */
//         retcode = RTI_TRUE;
//         goto done;
//     }

//     switch (kind)
//     {
//     case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
//     {
//         /* no parent */
//         break;
//     }
//     case D2S2_RESOURCEKIND_TOPIC:
//     case D2S2_RESOURCEKIND_PUBLISHER:
//     case D2S2_RESOURCEKIND_SUBSCRIBER:
//     {
//         parent_kind = D2S2_RESOURCEKIND_DOMAINPARTICIPANT;
//         if (!D2S2_EntityName_to_ref(
//                 &resource_name,
//                 D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
//                 &participant_name))
//         {
//             goto done;
//         }
//         parent_id.kind = D2S2_RESOURCEIDKIND_REF;
//         parent_id.value.ref = participant_name;
//         break;
//     }
//     case D2S2_RESOURCEKIND_DATAWRITER:
//     {
//         parent_kind = D2S2_RESOURCEKIND_PUBLISHER;
//         if (!D2S2_EntityName_to_ref(
//                 &resource_name,
//                 D2S2_ENTITYNAME_DEPTH_PUBLISHER,
//                 &publisher_name))
//         {
//             goto done;
//         }
//         parent_id.kind = D2S2_RESOURCEIDKIND_REF;
//         parent_id.value.ref = (char*) publisher_name;
//         break;
//     }
//     case D2S2_RESOURCEKIND_DATAREADER:
//     {
//         parent_kind = D2S2_RESOURCEKIND_SUBSCRIBER;
//         if (!D2S2_EntityName_to_ref(
//                 &resource_name,
//                 D2S2_ENTITYNAME_DEPTH_SUBSCRIBER,
//                 &subscriber_name))
//         {
//             goto done;
//         }
//         parent_id.kind = D2S2_RESOURCEIDKIND_REF;
//         parent_id.value.ref = (char*) subscriber_name;
//         break;
//     }
//     default:
//     {
//         /* invalid resource kind */
//         goto done;
//     }
//     }

//     if (parent_kind != D2S2_RESOURCEKIND_UNKNOWN)
//     {
//         if (!NDDSA_ResourceFactory_lookup_entity_resourceEA(
//                 self, parent_kind, &parent_id, &parent_exists, &parent_res))
//         {
//             goto done;
//         }
//         if (!parent_exists)
//         {
//             /* This should NOT happen */
//             goto done;
//         }
//     }

//     switch (kind)
//     {
//     case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
//     {
//         if (DDS_RETCODE_OK !=
//                 DDS_DomainParticipant_delete_contained_entities(
//                     entity_res.participant))
//         {
//             goto done;
//         }
        
//         if (DDS_RETCODE_OK !=
//                 DDS_DomainParticipantFactory_delete_participant(
//                     factory, entity_res.participant))
//         {
//             goto done;
//         }
//         break;
//     }
//     case D2S2_RESOURCEKIND_TOPIC:
//     {
//         if (DDS_RETCODE_OK !=
//                 DDS_DomainParticipant_delete_topic(
//                     parent_res.participant, entity_res.topic))
//         {
//             goto done;
//         }
//         break;
//     }
//     case D2S2_RESOURCEKIND_PUBLISHER:
//     {
//         if (DDS_RETCODE_OK !=
//                 DDS_Publisher_delete_contained_entities(entity_res.publisher))
//         {
//             goto done;
//         }

//         if (DDS_RETCODE_OK !=
//                 DDS_DomainParticipant_delete_publisher(
//                     parent_res.participant, entity_res.publisher))
//         {
//             goto done;
//         }
//         break;
//     }
//     case D2S2_RESOURCEKIND_SUBSCRIBER:
//     {
//         if (DDS_RETCODE_OK !=
//                 DDS_Subscriber_delete_contained_entities(
//                     entity_res.subscriber))
//         {
//             goto done;
//         }

//         if (DDS_RETCODE_OK !=
//                 DDS_DomainParticipant_delete_subscriber(
//                     parent_res.participant, entity_res.subscriber))
//         {
//             goto done;
//         }
//         break;
//     }
//     case D2S2_RESOURCEKIND_DATAWRITER:
//     {
//         if (DDS_RETCODE_OK !=
//                 DDS_Publisher_delete_datawriter(
//                     parent_res.publisher, entity_res.writer))
//         {
//             goto done;
//         }
//         break;
//     }
//     case D2S2_RESOURCEKIND_DATAREADER:
//     {
//         if (DDS_RETCODE_OK !=
//                 DDS_Subscriber_delete_datareader(
//                     parent_res.subscriber, entity_res.reader))
//         {
//             goto done;
//         }
//         break;
//     }
//     default:
//     {
//         /* Should never get here */
//         goto done;
//     }
//     }

//     retcode = RTI_TRUE;
// done:
//     if (initd_resource_name)
//     {
//         D2S2_EntityName_finalize(&resource_name);
//     }
//     if (participant_name != NULL)
//     {
//         RTIOsapiHeap_freeString(participant_name);
//     }
//     if (publisher_name != NULL)
//     {
//         RTIOsapiHeap_freeString(publisher_name);
//     }
//     if (subscriber_name != NULL)
//     {
//         RTIOsapiHeap_freeString(subscriber_name);
//     }
//     NDDSA_ResourceFactory_leave_ea(self);
//     return retcode;
// }

RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_lookup_entity_resourceEA(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out,
    NDDSA_EntityResource *const entity_out)
{
    RTIBool retcode = RTI_FALSE;

    *exists_out = RTI_FALSE;

    UNUSED_ARG(self);
    
    switch (kind)
    {
    case D2S2_RESOURCEKIND_APPLICATION:
    {
        if (!NDDSA_RefResource_lookup_application(id, exists_out))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        if (!NDDSA_RefResource_lookup_participant(
                id, &entity_out->participant))
        {
            goto done;
        }
        *exists_out = (entity_out->participant != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        if (!NDDSA_RefResource_lookup_topic(id, &entity_out->topic))
        {
            goto done;
        }
        *exists_out = (entity_out->topic != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        if (!NDDSA_RefResource_lookup_publisher(id, &entity_out->publisher))
        {
            goto done;
        }
        *exists_out = (entity_out->publisher != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        if (!NDDSA_RefResource_lookup_subscriber(id, &entity_out->subscriber))
        {
            goto done;
        }
        *exists_out = (entity_out->subscriber != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        if (!NDDSA_RefResource_lookup_datawriter(id, &entity_out->writer))
        {
            goto done;
        }
        *exists_out = (entity_out->writer != NULL);
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        if (!NDDSA_RefResource_lookup_datareader(id, &entity_out->reader))
        {
            goto done;
        }
        *exists_out = (entity_out->reader != NULL);
        
        break;
    }
    case D2S2_RESOURCEKIND_DOMAIN:
    {
        if (!NDDSA_RefResource_lookup_domain(id, exists_out))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_QOSPROFILE:
    {
        if (!NDDSA_RefResource_lookup_qosprofile(id, exists_out))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_TYPE:
    {
        if (!NDDSA_RefResource_lookup_type(id, exists_out))
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE:
    {
        void *resource_data = NULL;
        if (!NDDSA_RefResource_lookup_service(id, &resource_data))
        {
            goto done;
        }
        entity_out->service = (NDDSA_ExternalService*)resource_data;
        *exists_out = NULL != entity_out->service;
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
    {
        void *resource_data = NULL;
        if (!NDDSA_RefResource_lookup_service_resource(id, &resource_data))
        {
            goto done;
        }
        entity_out->service_resource = (NDDSA_ExternalServiceResource*)resource_data;
        *exists_out = NULL != entity_out->service_resource;
        break;
    }
    default:
    {
        goto done;
    }
    }
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}


RTIBool
NDDSA_ResourceFactory_lookup_entity_resource(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceId *const id,
    RTIBool *const exists_out,
    NDDSA_EntityResource *const entity_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_lookup_entity_resource)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    NDDSA_ResourceFactory_enter_ea(self);

    retcode = NDDSA_ResourceFactory_lookup_entity_resourceEA(
                    self, kind, id, exists_out, entity_out);

    NDDSA_ResourceFactory_leave_ea(self);

    D2S2Log_fn_exit()
    return retcode;
}


RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_append_created_resource(
    NDDSA_XmlResourceVisitor *const self,
    const char *const ref,
    const D2S2_ResourceKind ref_kind,
    void * const ref_data)
{
    RTIBool retcode = RTI_FALSE;
    NDDSA_CreatedResourceLog *id_ref = NULL;
    DDS_UnsignedLong seq_len = 0;

    seq_len = NDDSA_CreatedResourceLogSeq_get_length(self->result_seq);

    if (!NDDSA_CreatedResourceLogSeq_ensure_length(
            self->result_seq, seq_len + 1, seq_len + 1))
    {
        goto done;
    }

    id_ref = NDDSA_CreatedResourceLogSeq_get_reference(self->result_seq, seq_len);
    if (id_ref == NULL)
    {
        goto done;
    }

    id_ref->kind = ref_kind;
    if (!D2S2_ResourceId_initialize_ref(&id_ref->id, ref))
    {
        goto done;
    }
    id_ref->data = ref_data;

    
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        NDDSA_CreatedResourceLogSeq_set_length(self->result_seq, seq_len);
    }
    return retcode;
}


/******************************************************************************
 * XML Visitor Implementation
 ******************************************************************************/
RTI_PRIVATE
const char *
NDDSA_XmlResourceVisitor_get_resource_id(
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_resource(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj,
    const D2S2_ResourceKind res_kind);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_participant(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_publisher(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_subscriber(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_datawriter(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_datareader(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_topic(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_service(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_service_resource(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj);

RTI_PRIVATE
const char *
NDDSA_XmlResourceVisitor_get_resource_id(
    struct DDS_XMLObject *xml_obj)
{
    const char *el_fqn = NULL,
               *sub_fqn = NULL,
               *const pfx = "::",
               *res = NULL;
    DDS_UnsignedLong fqn_len = 0;
    const DDS_UnsignedLong pfx_len = 2;

    el_fqn = DDS_XMLObject_get_fully_qualified_name(xml_obj);
    if (el_fqn == NULL)
    {
        return NULL;
    }
    fqn_len = strlen(el_fqn);
    if (fqn_len == 0)
    {
        return NULL;
    }

    sub_fqn = strstr(el_fqn,pfx);
    if (sub_fqn == NULL || sub_fqn != el_fqn)
    {
        res = el_fqn;
    }
    else if (sub_fqn == el_fqn)
    {
        res = el_fqn + pfx_len;
    }

    return res;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_resource(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj,
    const D2S2_ResourceKind res_kind)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_resource)
    RTIBool retcode = RTI_FALSE;
    NDDSA_XmlResourceVisitor *const self =
        (NDDSA_XmlResourceVisitor*)visitor;
    const char *el_fqn = NULL;
    D2S2_ResourceId id = D2S2_RESOURCEID_INITIALIZER,
                    parent_id = D2S2_RESOURCEID_INITIALIZER;
    NDDSA_ResourceRecord *resource_rec = NULL;
    D2S2_ResourceRepresentation resource_repr =
        D2S2_RESOURCEREPRESENTATION_INITIALIZER;
    D2S2_EntityName res_name = D2S2_ENTITYNAME_INITIALIZER;
    DDS_DomainParticipant *dp = NULL;
    void * res_data = NULL;

    D2S2Log_fn_entry()

    el_fqn = NDDSA_XmlResourceVisitor_get_resource_id(xml_obj);
    if (el_fqn == NULL)
    {
        goto done;
    }
    
    id.kind = D2S2_RESOURCEIDKIND_REF;
    id.value.ref = (char*) el_fqn;

    if (!NDDSA_AgentDb_lookup_resourceEA(&agent->db, &id, &resource_rec))
    {
        goto done;
    }

    if (resource_rec == NULL)
    {
        if (!D2S2_EntityName_from_id(&res_name,&id))
        {
            goto done;
        }

        parent_id.kind = D2S2_RESOURCEIDKIND_REF;
        resource_repr.fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_REF;
        

        switch (res_kind)
        {
        case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
        {
            resource_repr.value.ref = (char*) el_fqn;
            dp = DDS_DomainParticipantFactory_create_participant_from_config(
                    self->factory, el_fqn);
            if (dp == NULL)
            {
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_PUBLISHER:
        {
            resource_repr.value.ref = D2S2_EntityName_leaf(&res_name);
            if (!D2S2_EntityName_to_ref(
                    &res_name,
                    D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
                    &parent_id.value.ref))
            {
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_SUBSCRIBER:
        {
            resource_repr.value.ref = D2S2_EntityName_leaf(&res_name);
            if (!D2S2_EntityName_to_ref(
                    &res_name,
                    D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
                    &parent_id.value.ref))
            {
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_TOPIC:
        {
            resource_repr.value.ref = D2S2_EntityName_leaf(&res_name);
            if (!D2S2_EntityName_to_ref(
                    &res_name,
                    D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
                    &parent_id.value.ref))
            {
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_DATAREADER:
        {
            resource_repr.value.ref = D2S2_EntityName_leaf(&res_name);
            if (!D2S2_EntityName_to_ref(
                    &res_name,
                    D2S2_ENTITYNAME_DEPTH_SUBSCRIBER,
                    &parent_id.value.ref))
            {
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_DATAWRITER:
        {
            resource_repr.value.ref = D2S2_EntityName_leaf(&res_name);
            if (!D2S2_EntityName_to_ref(
                    &res_name,
                    D2S2_ENTITYNAME_DEPTH_PUBLISHER,
                    &parent_id.value.ref))
            {
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEKIND_SERVICE:
        {
            resource_repr.value.ref = (char*) el_fqn;
            res_data = xml_obj;
            break;
        }
        case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
        {
            resource_repr.value.ref = D2S2_EntityName_leaf(&res_name);
            if (!D2S2_EntityName_to_ref(
                    &res_name,
                    D2S2_ENTITYNAME_DEPTH_SERVICE,
                    &parent_id.value.ref))
            {
                goto done;
            }
            break;
        }
        default:
            goto done;
        }

        if (DDS_RETCODE_OK !=
                NDDSA_Agent_insert_resource_recordEA(
                    agent,
                    res_kind,
                    &resource_repr,
                    ((res_kind != D2S2_RESOURCEKIND_DOMAINPARTICIPANT &&
                      res_kind != D2S2_RESOURCEKIND_SERVICE)?
                        &parent_id : NULL),
                    &self->res_properties,
                    &resource_rec))
        {
            goto done;
        }

#if 0
        resource_rec->resource.loaded = RTI_TRUE;
#endif
    }

    if (!NDDSA_XmlResourceVisitor_append_created_resource(
            self, id.value.ref, res_kind, res_data))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (resource_rec != NULL)
    {
        if (!NDDSA_AgentDb_release_resourceEA(&agent->db, resource_rec))
        {
            D2S2Log_exception(
                method_name,
                &RTI_LOG_ANY_FAILURE_s,
                D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED);
            retcode = RTI_FALSE;
        }
    }
    D2S2_EntityName_finalize(&res_name);
    D2S2_ResourceId_finalize(&parent_id);

    D2S2Log_fn_exit()
    return retcode;
}


RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_participant(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_participant)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_DOMAINPARTICIPANT);
    
    D2S2Log_fn_exit();
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_publisher(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_publisher)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_PUBLISHER);
    
    D2S2Log_fn_exit();
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_subscriber(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_subscriber)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_SUBSCRIBER);
    
    D2S2Log_fn_exit();
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_datawriter(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_datawriter)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_DATAWRITER);
    
    D2S2Log_fn_exit();
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_datareader(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_datareader)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_DATAREADER);
    
    D2S2Log_fn_exit();
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_topic(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_topic)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_TOPIC);

    D2S2Log_fn_exit();
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_service(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_service)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_SERVICE);

    D2S2Log_fn_exit();
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_XmlResourceVisitor_on_service_resource(
    void *const visitor,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *xml_obj)
{
    D2S2Log_METHOD_NAME(NDDSA_XmlResourceVisitor_on_service_resource)
    RTIBool retcode = RTI_FALSE;
    D2S2Log_fn_entry()

    retcode = NDDSA_XmlResourceVisitor_on_resource(
                visitor, agent, xml_obj,
                D2S2_RESOURCEKIND_SERVICE_RESOURCE);

    D2S2Log_fn_exit();
    return retcode;
}

void
NDDSA_XmlResourceVisitor_initialize(
    NDDSA_XmlResourceVisitor *const self,
    const D2S2_ResourceProperties *const res_properties,
    struct NDDSA_CreatedResourceLogSeq *const created_resources)
{
    self->base.on_participant = NDDSA_XmlResourceVisitor_on_participant;
    self->base.on_topic = NDDSA_XmlResourceVisitor_on_topic;
    self->base.on_publisher = NDDSA_XmlResourceVisitor_on_publisher;
    self->base.on_subscriber = NDDSA_XmlResourceVisitor_on_subscriber;
    self->base.on_datawriter = NDDSA_XmlResourceVisitor_on_datawriter;
    self->base.on_datareader = NDDSA_XmlResourceVisitor_on_datareader;
    self->base.on_service = NDDSA_XmlResourceVisitor_on_service;
    self->base.on_service_resource = NDDSA_XmlResourceVisitor_on_service_resource;
    self->base.user_data = self;
    self->res_properties = *res_properties;
    
    self->factory = DDS_DomainParticipantFactory_get_instance();

    self->result_seq = created_resources;
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */
