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
#include "NddsRefResource.h"
#include "ServiceXml.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

RTI_PRIVATE
void
NDDSA_XmlResource_find_first_xml_element(
    const char *const xml_str,
    const char *const xml_el_open_tag,
    const char *const xml_el_close_tag,
    const char **const xml_el_start_out,
    const char **const xml_el_end_out);

/******************************************************************************
 * Private Functions
 ******************************************************************************/
RTI_PRIVATE
void
NDDSA_XmlResource_find_first_xml_element(
    const char *const xml_str,
    const char *const xml_el_open_tag,
    const char *const xml_el_close_tag,
    const char **const xml_el_start_out,
    const char **const xml_el_end_out)
{
    const char *xml_el_start = NULL,
               *xml_el_end = NULL;
    DDS_UnsignedLong next_ch = 0;

    *xml_el_start_out = NULL;
    *xml_el_end_out = NULL;

    xml_el_start = strstr(xml_str, xml_el_open_tag);

    if (xml_el_start == NULL)
    {
        return;
    }
    /* check that the following char is either a space or ">" */
    next_ch = strlen(xml_el_open_tag);
    if (xml_el_start[next_ch] != ' ' && xml_el_start[next_ch] != '>')
    {
        return;
    }
    
    xml_el_end = strstr(xml_el_start, xml_el_close_tag);
    /* if we found an opening element but didn't find a closing
        * one refuse to process the string further, and return NULL */
    if (xml_el_end == NULL)
    {
        return;
    }
    /* we found the closing tag, return pointer to end of closing tag */
    xml_el_end += strlen(xml_el_close_tag);
    
    *xml_el_start_out = xml_el_start;
    *xml_el_end_out = xml_el_end;
}

RTIBool
NDDSA_XmlResource_parse_xml_element_name(
    const char *const el_xml,
    const char *const el_open_tag,
    const char **const el_content_start_out,
    char **const el_name_out)
{
    RTIBool retcode = RTI_FALSE;
    const char *el_start = NULL,
               *el_end = NULL,
               *el_name_pfx = "name=\"",
               *el_content_start = NULL;
    char *el_name = NULL;
    DDS_UnsignedLong el_len = 0;

    *el_name_out = NULL;
    *el_content_start_out = NULL;

    el_start = strstr(el_xml, el_open_tag);
    if (el_start == NULL)
    {
        goto done;
    }
    el_start += strlen(el_open_tag);

    el_start = strstr(el_start, el_name_pfx);
    if (el_start == NULL)
    {
        goto done;
    }

    el_start += strlen(el_name_pfx);
    el_end = strstr(el_start, "\"");
    if (el_end == NULL)
    {
        goto done;
    }

    el_len = el_end - el_start;

    RTIOsapiHeap_allocateString(&el_name, el_len);
    if (el_name == NULL)
    {
        goto done;
    }

    RTIOsapiMemory_copy(el_name, el_start, el_len);
    el_name[el_len] = '\0';

    el_content_start = strstr(el_start, ">");
    if (el_content_start == NULL)
    {
        goto done;
    }
    el_content_start += 1;

    *el_name_out = el_name;
    *el_content_start_out = el_content_start;
    
    retcode = RTI_TRUE;
    
done:
    if (!retcode && el_name != NULL)
    {
        RTIOsapiHeap_free(el_name);
    }
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_create_participants_xml(
    NDDSA_ResourceFactory *const self,
    DDS_DomainParticipantFactory *const factory,
    const char *const lib_name,
    const char *const dp_lib_xml_content_start,
    const char *const dp_lib_xml_content_end,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const participants_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_participants_xml)
    RTIBool retcode = RTI_FALSE;
    const char *dp_xml_start = NULL,
               *dp_xml_end = NULL,
               *dp_lib_xml_head = NULL,
               *dp_xml_content_start = NULL;
    char *dp_xml_name = NULL,
         *dp_full_name = NULL;
    DDS_UnsignedLong dp_i = 0,
                     dp_i_start = 0;
    NDDSA_CreatedResourceLog *dp_id = NULL;
    DDS_DomainParticipant *dp = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    dp_i = NDDSA_CreatedResourceLogSeq_get_length(participants_out);
    dp_i_start = dp_i;

    dp_lib_xml_head = dp_lib_xml_content_start;

    while (dp_lib_xml_head != NULL && dp_lib_xml_head[0] != '\0')
    {
        NDDSA_XmlResource_find_first_xml_element(
                dp_lib_xml_head,
                "<domain_participant",
                "</domain_participant>",
                &dp_xml_start,
                &dp_xml_end);
        
        if (dp_xml_start != NULL && dp_xml_end != NULL)
        {
            /* Check that the next element was not found beyond the valid
               boundaries of the input string */
            if (dp_xml_start >= dp_lib_xml_content_end)
            {
                dp_lib_xml_head = NULL;
                continue;
            }

            if (dp_xml_name != NULL)
            {
                RTIOsapiHeap_free(dp_xml_name);
                dp_xml_name = NULL;
            }
            if (!NDDSA_XmlResource_parse_xml_element_name(
                    dp_xml_start,
                    "<domain_participant",
                    &dp_xml_content_start,
                    &dp_xml_name))
            {
                goto done;
            }
            if (dp_full_name != NULL)
            {
                RTIOsapiHeap_free(dp_full_name);
                dp_full_name = NULL;
            }
            if (!NDDSA_StringUtil_append(
                    lib_name, "::", dp_xml_name, &dp_full_name))
            {
                goto done;
            }

            if (!NDDSA_CreatedResourceLogSeq_ensure_length(
                    participants_out,
                    dp_i + 1,
                    dp_i + 1))
            {
                /* TODO log */
                goto done;
            }

            // create_params.participant_name = (char*) dp_full_name;

#if 0
            if (properties->domain_id != 0)
            {
                struct DDS_DomainParticipantConfigParams_t create_params =
                    DDS_DomainParticipantConfigParams_t_INITIALIZER;
                create_params.domain_id = properties->domain_id;

                /* This method cannot be used because it requires us to fill in
                   also the qos_profile information, otherwise creation will
                   fail. This is not an easy thing to do and most importantly it
                   is not compatible with a participant with "inline qos" in the
                   XML definition, which is not defined in a separate profile. */
                dp = DDS_DomainParticipantFactory_create_participant_from_config_w_params(
                        factory, dp_full_name, &create_params);
            }
            else
#endif
            {
                dp = DDS_DomainParticipantFactory_create_participant_from_config(
                        factory, dp_full_name);
            }
            if (dp == NULL)
            {
                goto done;
            }


            dp_id = NDDSA_CreatedResourceLogSeq_get_reference(
                        participants_out, dp_i);
            dp_id->kind = D2S2_RESOURCEKIND_DOMAINPARTICIPANT;
            if (!D2S2_ResourceId_initialize_ref(&dp_id->id, dp_full_name))
            {
                DDS_DomainParticipantFactory_delete_participant(factory, dp);
                /* TODO log */
                goto done;
            }

            dp_i += 1;
        }

        dp_lib_xml_head = dp_xml_end;
    }
    
    
    retcode = RTI_TRUE;
    
done:
    if (dp_xml_name != NULL)
    {
        RTIOsapiHeap_free(dp_xml_name);
    }
    if (dp_full_name != NULL)
    {
        RTIOsapiHeap_free(dp_full_name);
    }

    if (!retcode)
    {
        if (dp_i > dp_i_start)
        {
            DDS_UnsignedLong i = 0;
            for (i = dp_i_start; i < dp_i; i++)
            {
                dp_id = NDDSA_CreatedResourceLogSeq_get_reference(
                            participants_out, i);
                
                if (!NDDSA_RefResource_lookup_participant(&dp_id->id, &dp))
                {
                    continue;
                }
                if (dp != NULL)
                {
                    DDS_DomainParticipantFactory_delete_participant(
                        factory, dp);
                }
            }
        }
        NDDSA_CreatedResourceLogSeq_set_length(participants_out, 0);
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_ResourceFactory_create_qosprofile_xml(
    NDDSA_ResourceFactory *const self,
    DDS_DomainParticipantFactory *const factory,
    const char *const lib_name,
    const char *const lib_xml_content_start,
    const char *const lib_xml_content_end,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const profiles_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_qosprofile_xml)
    RTIBool retcode = RTI_FALSE;
    const char *el_xml_start = NULL,
               *el_xml_end = NULL,
               *lib_xml_head = NULL,
               *el_xml_content_start = NULL;
    char *el_xml_name = NULL,
         *el_full_name = NULL;
    DDS_UnsignedLong el_i = 0;
    NDDSA_CreatedResourceLog *el_id = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(factory);
    UNUSED_ARG(properties);
    
    el_i = NDDSA_CreatedResourceLogSeq_get_length(profiles_out);

    lib_xml_head = lib_xml_content_start;

    while (lib_xml_head != NULL && lib_xml_head[0] != '\0')
    {
        NDDSA_XmlResource_find_first_xml_element(
                lib_xml_head,
                "<qos_profile",
                "</qos_profile>",
                &el_xml_start,
                &el_xml_end);
        
        if (el_xml_start != NULL && el_xml_end != NULL)
        {
            /* Check that the next element was not found beyond the valid
               boundaries of the input string */
            if (el_xml_start >= lib_xml_content_end)
            {
                lib_xml_head = NULL;
                continue;
            }

            if (el_xml_name != NULL)
            {
                RTIOsapiHeap_free(el_xml_name);
                el_xml_name = NULL;
            }
            if (!NDDSA_XmlResource_parse_xml_element_name(
                    el_xml_start,
                    "<qos_profile",
                    &el_xml_content_start,
                    &el_xml_name))
            {
                goto done;
            }
            if (el_full_name != NULL)
            {
                RTIOsapiHeap_free(el_full_name);
                el_full_name = NULL;
            }
            if (!NDDSA_StringUtil_append(
                    lib_name, "::", el_xml_name, &el_full_name))
            {
                goto done;
            }

            if (!NDDSA_CreatedResourceLogSeq_ensure_length(
                    profiles_out,
                    el_i + 1,
                    el_i + 1))
            {
                /* TODO log */
                goto done;
            }

            el_id = NDDSA_CreatedResourceLogSeq_get_reference(profiles_out, el_i);
            el_id->kind = D2S2_RESOURCEKIND_QOSPROFILE;
            if (!D2S2_ResourceId_initialize_ref(&el_id->id, el_full_name))
            {
                /* TODO log */
                goto done;
            }

            el_i += 1;
        }

        lib_xml_head = el_xml_end;
    }
    
    
    retcode = RTI_TRUE;
    
done:
    if (el_xml_name != NULL)
    {
        RTIOsapiHeap_free(el_xml_name);
    }
    if (el_full_name != NULL)
    {
        RTIOsapiHeap_free(el_full_name);
    }

    if (!retcode)
    {
        NDDSA_CreatedResourceLogSeq_set_length(profiles_out, 0);
    }
    D2S2Log_fn_exit()
    return retcode;
}


/******************************************************************************
 * Public Helpers
 ******************************************************************************/

RTIBool
NDDSA_ResourceFactory_create_application_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const app_lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const applications_out,
    struct NDDSA_CreatedResourceLogSeq *const participants_out,
    char **const app_lib_name_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_application_library_xml)
    RTIBool retcode = RTI_FALSE,
            initd_lib = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *app_xml_start = NULL,
               *app_xml_end = NULL,
               *app_xml_content_start = NULL,
               *app_lib_xml_head = NULL,
               *app_lib_full_name = NULL,
               *app_lib_xml_start = NULL,
               *app_lib_xml_end = NULL;
    char *app_xml_name = NULL,
         *app_full_name = NULL, 
         *app_lib_name = NULL;
    NDDSA_CreatedResourceLog *app_id = NULL;
    DDS_UnsignedLong app_created_count = 0;

    D2S2Log_fn_entry()

    if (!NDDSA_CreatedResourceLogSeq_set_length(applications_out, 0))
    {
        /* TODO log */
        goto done;
    }

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
            app_lib_xml,
            "<application_library",
            "</application_library>",
            &app_lib_xml_start,
            &app_lib_xml_end);
    
    if (app_lib_xml_start == NULL ||
        app_lib_xml_end == NULL)
    {
        /* TODO log */
        goto done;
    }

    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &app_lib_full_name, "", app_lib_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        /* TODO log */
        goto done;
    }
    initd_lib = RTI_TRUE;

    if (!NDDSA_XmlResource_parse_xml_element_name(
            app_lib_xml,
            "<application_library",
            &app_lib_xml_head,
            &app_lib_name))
    {
        /* TODO log */
        goto done;
    }

    while (app_lib_xml_head != NULL && app_lib_xml_head[0] != '\0')
    {
        NDDSA_XmlResource_find_first_xml_element(
                app_lib_xml_head,
                "<application",
                "</application>",
                &app_xml_start,
                &app_xml_end);

        if (app_xml_start != NULL)
        {
            if (app_xml_name != NULL)
            {
                RTIOsapiHeap_free(app_xml_name);
                app_xml_name = NULL;
            }
            if (!NDDSA_XmlResource_parse_xml_element_name(
                    app_xml_start,
                    "<application",
                    &app_xml_content_start,
                    &app_xml_name))
            {
                goto done;
            }
            if (app_full_name != NULL)
            {
                RTIOsapiHeap_free(app_full_name);
                app_full_name = NULL;
            }
            if (!NDDSA_StringUtil_append(
                    app_lib_full_name, "::", app_xml_name, &app_full_name))
            {
                goto done;
            }

            if (!NDDSA_CreatedResourceLogSeq_ensure_length(
                    applications_out,
                    app_created_count + 1,
                    app_created_count + 1))
            {
                /* TODO log */
                goto done;
            }

            app_id = NDDSA_CreatedResourceLogSeq_get_reference(
                        applications_out, app_created_count);
            app_id->kind = D2S2_RESOURCEKIND_APPLICATION;
            if (!D2S2_ResourceId_initialize_ref(&app_id->id, app_full_name))
            {
                /* TODO log */
                goto done;
            }

            app_created_count += 1;

            if (!NDDSA_ResourceFactory_create_participants_xml(
                    self,
                    factory,
                    app_full_name,
                    app_xml_content_start,
                    app_xml_end - strlen("</application>"),
                    properties,
                    participants_out))
            {
                /* TODO log */
                goto done;
            }
        }

        app_lib_xml_head = app_xml_end;
    }
    
    
    *app_lib_name_out = app_lib_name;
    retcode = RTI_TRUE;
    
done:
    if (app_lib_name != NULL)
    {
        RTIOsapiHeap_free(app_lib_name);
    }
    if (app_full_name != NULL)
    {
        RTIOsapiHeap_free(app_full_name);
    }

    if (!retcode)
    {
        if (initd_lib)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                    factory, app_lib_full_name);
        }
        NDDSA_CreatedResourceLogSeq_set_length(applications_out, 0);
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_participant_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const dp_lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const participants_out,
    char **const dp_lib_name_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_participant_library_xml)
    RTIBool retcode = RTI_FALSE,
            initd_lib = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *dp_lib_xml_head = NULL,
               *dp_lib_xml_start = NULL,
               *dp_lib_xml_end = NULL,
               *dp_lib_full_name = NULL;
    char *dp_lib_name = NULL;

    D2S2Log_fn_entry()

    if (!NDDSA_CreatedResourceLogSeq_set_length(participants_out, 0))
    {
        /* TODO log */
        goto done;
    }
    
    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
            dp_lib_xml,
            "<domain_participant_library",
            "</domain_participant_library>",
            &dp_lib_xml_start,
            &dp_lib_xml_end);
    
    if (dp_lib_xml_start == NULL ||
        dp_lib_xml_end == NULL)
    {
        /* TODO log */
        goto done;
    }
    
    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &dp_lib_full_name, "", dp_lib_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        /* TODO log */
        goto done;
    }

    if (!NDDSA_XmlResource_parse_xml_element_name(
            dp_lib_xml,
            "<domain_participant_library",
            &dp_lib_xml_head,
            &dp_lib_name))
    {
        /* TODO log */
        goto done;
    }

    if (!NDDSA_ResourceFactory_create_participants_xml(
            self,
            factory,
            dp_lib_name,
            dp_lib_xml_head,
            dp_lib_xml_end - strlen("</domain_participant_library>"),
            properties,
            participants_out))
    {
        /* TODO log */
        goto done;
    }
    
    *dp_lib_name_out = dp_lib_name;
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (dp_lib_name != NULL)
        {
            RTIOsapiHeap_free(dp_lib_name);
        }
        if (initd_lib)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                factory, dp_lib_full_name);
        }
        NDDSA_CreatedResourceLogSeq_set_length(participants_out, 0);
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_topic_xml(
    NDDSA_ResourceFactory *const self,
    const char *const topic_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const participant_id,
    NDDSA_CreatedResourceLog *const topic_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_topic_xml)
    RTIBool retcode = RTI_FALSE,
            loaded_xml = RTI_FALSE,
            created_dds = RTI_FALSE;
            // initd_name = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    DDS_DomainParticipant *participant = NULL;
    DDS_Topic *topic = NULL;
    const char *participant_ref = NULL,
               *topic_xml_start = NULL,
               *topic_xml_end = NULL,
               *topic_full_name = NULL;
    // D2S2_EntityName participant_name = D2S2_ENTITYNAME_INITIALIZER;
    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
                topic_xml,
                "<topic",
                "</topic>",
                &topic_xml_start,
                &topic_xml_end);
    if (topic_xml_start == NULL ||
        topic_xml_end == NULL)
    {
        goto done;
    }

    // if (!D2S2_EntityName_from_id(&participant_name, participant_id))
    // {
    //     goto done;
    // }
    // if (!D2S2_EntityName_component(
    //         &participant_name,
    //         D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
    //         &participant_ref))
    // {
    //     goto done;
    // }

    if (!D2S2_ResourceId_as_ref(participant_id, &participant_ref))
    {
        goto done;
    }
    
    if (!NDDSA_RefResource_lookup_participant(participant_id, &participant))
    {
        goto done;
    }

    if (participant == NULL)
    {
        /* unknown participant */
        goto done;
    }

    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &topic_full_name, participant_ref, topic_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        /* TODO log */
        goto done;
    }
    loaded_xml = RTI_TRUE;

    topic = DDS_DomainParticipant_create_topic_from_config(
            participant, topic_full_name);
    if (topic == NULL)
    {
        goto done;
    }
    created_dds = RTI_TRUE;

    topic_out->kind = D2S2_RESOURCEKIND_TOPIC;
    if (!D2S2_ResourceId_initialize_ref(&topic_out->id, topic_full_name))
    {
        /* TODO log */
        goto done;
    }


    
    retcode = RTI_TRUE;
    
done:
    // D2S2_EntityName_finalize(&participant_name);

    if (!retcode)
    {
        if (created_dds)
        {
            /* TODO delete topic */
        }
        if (loaded_xml)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                    factory, topic_full_name);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_publisher_xml(
    NDDSA_ResourceFactory *const self,
    const char *const pub_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const participant,
    NDDSA_CreatedResourceLog *const pub_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_publisher_xml)
    RTIBool retcode = RTI_FALSE,
            loaded_xml = RTI_FALSE,
            created_dds = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *pub_xml_start = NULL,
               *pub_xml_end = NULL,
               *pub_full_name = NULL,
               *dp_full_name = NULL;
    DDS_DomainParticipant *dp = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    if (!NDDSA_RefResource_lookup_participant(participant, &dp))
    {
        /* some error occurred while looking up participant */
        goto done;
    }
    if (dp == NULL)
    {
        /* Unknown participant */
        goto done;
    }
    if (!D2S2_ResourceId_as_ref(participant, &dp_full_name))
    {
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
                pub_xml,
                "<publisher",
                "</publisher>",
                &pub_xml_start,
                &pub_xml_end);
    if (pub_xml_start == NULL ||
        pub_xml_end == NULL)
    {
        goto done;
    }

    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &pub_full_name, dp_full_name, pub_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        /* TODO log */
        goto done;
    }
    loaded_xml = RTI_TRUE;

    rc = DDS_DomainParticipant_create_publishers_from_config(
            dp, NULL, pub_full_name);
    if (DDS_RETCODE_OK != rc)
    {
        goto done;
    }
    created_dds = RTI_TRUE;

    pub_out->kind = D2S2_RESOURCEKIND_PUBLISHER;
    if (!D2S2_ResourceId_initialize_ref(&pub_out->id, pub_full_name))
    {
        /* TODO log */
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    // if (pub_full_name != NULL)
    // {
    //     RTIOsapiHeap_free(pub_full_name);
    // }

    if (!retcode)
    {
        if (created_dds)
        {
            /*TODO delete publisher */
        }
        if (loaded_xml)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                factory, pub_full_name);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_subscriber_xml(
    NDDSA_ResourceFactory *const self,
    const char *const sub_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const participant,
    NDDSA_CreatedResourceLog *const sub_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_subscriber_xml)
    RTIBool retcode = RTI_FALSE,
            loaded_xml = RTI_FALSE,
            created_dds = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *sub_xml_start = NULL,
               *sub_xml_end = NULL,
               *sub_full_name = NULL,
               *dp_full_name = NULL;
    DDS_DomainParticipant *dp = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    if (!NDDSA_RefResource_lookup_participant(participant, &dp))
    {
        /* some error occurred while looking up participant */
        goto done;
    }
    if (dp == NULL)
    {
        /* Unknown participant */
        goto done;
    }

    if (!D2S2_ResourceId_as_ref(participant, &dp_full_name))
    {
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
                sub_xml,
                "<subscriber",
                "</subscriber>",
                &sub_xml_start,
                &sub_xml_end);
    if (sub_xml_start == NULL ||
        sub_xml_end == NULL)
    {
        goto done;
    }

    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &sub_full_name, dp_full_name, sub_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        /* TODO log */
        goto done;
    }

    rc = DDS_DomainParticipant_create_subscribers_from_config(
            dp, NULL, sub_full_name);
    if (DDS_RETCODE_OK != rc)
    {
        goto done;
    }

    sub_out->kind = D2S2_RESOURCEKIND_SUBSCRIBER;
    if (!D2S2_ResourceId_initialize_ref(&sub_out->id, sub_full_name))
    {
        /* TODO log */
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    // if (sub_full_name != NULL)
    // {
    //     RTIOsapiHeap_free(sub_full_name);
    // }
    if (!retcode)
    {
        if (created_dds)
        {
            /*TODO delete subscriber */
        }
        if (loaded_xml)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                factory, sub_full_name);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_datawriter_xml(
    NDDSA_ResourceFactory *const self,
    const char *const writer_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const publisher_id,
    NDDSA_CreatedResourceLog *const writer_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_datawriter_xml)
    RTIBool retcode = RTI_FALSE,
            initd_name = RTI_FALSE,
            xml_loaded = RTI_FALSE,
            dds_created = RTI_FALSE;
    DDS_DomainParticipantFactory *factory = NULL;
    DDS_DomainParticipant *participant = NULL;
    DDS_Publisher *publisher = NULL;
    char *participant_ref = NULL;
    D2S2_ResourceId participant_id = D2S2_RESOURCEID_INITIALIZER;
    const char *publisher_ref = NULL,
               *dw_xml_start = NULL,
               *dw_xml_end = NULL,
               *dw_full_name = NULL;
    D2S2_EntityName publisher_name = D2S2_ENTITYNAME_INITIALIZER;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    if (!D2S2_EntityName_from_id(&publisher_name, publisher_id))
    {
        goto done;
    }
    initd_name = RTI_TRUE;
    
    if ((D2S2_ENTITYNAME_DEPTH_PUBLISHER !=
            D2S2_EntityName_depth(&publisher_name)))
    {
        /* invalid id */
        goto done;
    }

    if (!D2S2_EntityName_to_ref(
            &publisher_name,
            D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
            &participant_ref))
    {
        goto done;
    }

    participant_id.kind = D2S2_RESOURCEIDKIND_REF;
    participant_id.value.ref = participant_ref;

    if (!NDDSA_RefResource_lookup_participant(&participant_id, &participant))
    {
        goto done;
    }
    if (participant == NULL)
    {
        /* unknown participant */
        goto done;
    }
    
    publisher = 
        DDS_DomainParticipant_lookup_publisher_by_name(
            participant, D2S2_EntityName_leaf(&publisher_name));
    if (publisher == NULL)
    {
        /* unknown publisher */
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
                writer_xml,
                "<data_writer",
                "</data_writer>",
                &dw_xml_start,
                &dw_xml_end);
    if (dw_xml_start == NULL ||
        dw_xml_end == NULL)
    {
        goto done;
    }

    if (!D2S2_ResourceId_as_ref(publisher_id, &publisher_ref))
    {
        goto done;
    }

    retcode = 
        DDS_DomainParticipantFactory_load_xml_element(
            factory, &dw_full_name, publisher_ref, dw_xml_start);
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }
    xml_loaded = RTI_TRUE;

    retcode = DDS_Publisher_create_datawriters_from_config(
                    publisher, NULL, dw_full_name);
    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }
    dds_created = RTI_TRUE;

    writer_out->kind = D2S2_RESOURCEKIND_DATAWRITER;
    if (!D2S2_ResourceId_initialize_ref(&writer_out->id, dw_full_name))
    {
        /* TODO log */
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (participant_ref != NULL)
    {
        RTIOsapiHeap_free(participant_ref);
    }
    if (initd_name)
    {
        D2S2_EntityName_finalize(&publisher_name);
    }

    if (!retcode)
    {
        if (dds_created)
        {
            /* TODO implement me: delete datawriter */
        }
        if (xml_loaded)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                    factory, dw_full_name);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_datareader_xml(
    NDDSA_ResourceFactory *const self,
    const char *const reader_xml,
    const D2S2_ResourceProperties *const properties,
    const D2S2_ResourceId *const subscriber_id,
    NDDSA_CreatedResourceLog *const reader_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_datareader_xml)
    RTIBool retcode = RTI_FALSE,
            initd_name = RTI_FALSE,
            xml_loaded = RTI_FALSE,
            dds_created = RTI_FALSE;
    DDS_DomainParticipantFactory *factory = NULL;
    DDS_DomainParticipant *participant = NULL;
    DDS_Subscriber *subscriber = NULL;
    char *participant_ref = NULL;
    D2S2_ResourceId participant_id = D2S2_RESOURCEID_INITIALIZER;
    const char *subscriber_ref = NULL,
               *dr_xml_start = NULL,
               *dr_xml_end = NULL,
               *dr_full_name = NULL;
    D2S2_EntityName subscriber_name = D2S2_ENTITYNAME_INITIALIZER;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    if (!D2S2_EntityName_from_id(&subscriber_name, subscriber_id))
    {
        goto done;
    }
    initd_name = RTI_TRUE;
    
    if ((D2S2_ENTITYNAME_DEPTH_SUBSCRIBER !=
            D2S2_EntityName_depth(&subscriber_name)))
    {
        /* invalid id */
        goto done;
    }

    if (!D2S2_EntityName_to_ref(
            &subscriber_name,
            D2S2_ENTITYNAME_DEPTH_PARTICIPANT,
            &participant_ref))
    {
        goto done;
    }
    participant_id.kind = D2S2_RESOURCEIDKIND_REF;
    participant_id.value.ref = participant_ref;

    if (!NDDSA_RefResource_lookup_participant(&participant_id, &participant))
    {
        goto done;
    }
    if (participant == NULL)
    {
        /* unknown participant */
        goto done;
    }
    
    subscriber = 
        DDS_DomainParticipant_lookup_subscriber_by_name(
            participant, D2S2_EntityName_leaf(&subscriber_name));
    if (subscriber == NULL)
    {
        /* unknown subscriber */
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
                reader_xml,
                "<data_reader",
                "</data_reader>",
                &dr_xml_start,
                &dr_xml_end);
    if (dr_xml_start == NULL ||
        dr_xml_end == NULL)
    {
        goto done;
    }

    if (!D2S2_ResourceId_as_ref(subscriber_id, &subscriber_ref))
    {
        goto done;
    }

    retcode = 
        DDS_DomainParticipantFactory_load_xml_element(
            factory, &dr_full_name, subscriber_ref, dr_xml_start);
    if (DDS_RETCODE_OK != retcode)
    {
        goto done;
    }
    xml_loaded = RTI_TRUE;

    retcode = DDS_Subscriber_create_datareaders_from_config(
                    subscriber, NULL, dr_full_name);
    if (retcode != DDS_RETCODE_OK)
    {
        goto done;
    }
    dds_created = RTI_TRUE;

    reader_out->kind = D2S2_RESOURCEKIND_DATAREADER;
    if (!D2S2_ResourceId_initialize_ref(&reader_out->id, dr_full_name))
    {
        /* TODO log */
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (participant_ref != NULL)
    {
        RTIOsapiHeap_free(participant_ref);
    }
    if (initd_name)
    {
        D2S2_EntityName_finalize(&subscriber_name);
    }

    if (!retcode)
    {
        if (dds_created)
        {
            /* TODO implement me: delete datawriter */
        }
        if (xml_loaded)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                    factory, dr_full_name);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_types_xml(
    NDDSA_ResourceFactory *const self,
    const char *const types_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const types_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_types_xml)
    RTIBool retcode = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *types_xml_start = NULL,
               *types_xml_end = NULL,
               *inserted_tag_name = NULL;
    DDS_UnsignedLong types_seq_len = 0;
    NDDSA_CreatedResourceLog *types_id = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
                types_xml,
                "<types",
                "</types>",
                &types_xml_start,
                &types_xml_end);
    if (types_xml_start == NULL ||
        types_xml_end == NULL)
    {
        goto done;
    }

    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &inserted_tag_name, "", types_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "DDS_DomainParticipantFactory_load_xml_element");
        goto done;
    }

    types_seq_len = NDDSA_CreatedResourceLogSeq_get_length(types_out);

    if (!NDDSA_CreatedResourceLogSeq_ensure_length(
            types_out,
            types_seq_len + 1,
            types_seq_len + 1))
    {
        /* TODO log */
        goto done;
    }

    types_id = NDDSA_CreatedResourceLogSeq_get_reference(types_out, types_seq_len);

    types_id->kind = D2S2_RESOURCEKIND_TYPE;
    if (!D2S2_ResourceId_initialize_ref(&types_id->id, inserted_tag_name))
    {
        /* TODO log */
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_domain_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const domains_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_domain_library_xml)
    RTIBool retcode = RTI_FALSE,
            initd_lib = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *xml_start = NULL,
               *xml_end = NULL,
               *xml_content_start = NULL,
               *lib_xml_head = NULL,
               *lib_full_name = NULL,
               *lib_xml_start = NULL,
               *lib_xml_end = NULL;
    char *xml_name = NULL,
         *full_name = NULL, 
         *lib_name = NULL;
    NDDSA_CreatedResourceLog *id = NULL;
    DDS_UnsignedLong created_count = 0;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);
    UNUSED_ARG(properties);

    if (!NDDSA_CreatedResourceLogSeq_set_length(domains_out, 0))
    {
        goto done;
    }

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
            lib_xml,
            "<domain_library",
            "</domain_library>",
            &lib_xml_start,
            &lib_xml_end);
    
    if (lib_xml_start == NULL ||
        lib_xml_end == NULL)
    {
        goto done;
    }
    
    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &lib_full_name, "", lib_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_FAILURE_s,
            "DDS_DomainParticipantFactory_load_xml_element");
        goto done;
    }
    initd_lib = RTI_TRUE;

    if (!NDDSA_XmlResource_parse_xml_element_name(
            lib_xml,
            "<domain_library",
            &lib_xml_head,
            &lib_name))
    {
        goto done;
    }

    while (lib_xml_head != NULL && lib_xml_head[0] != '\0')
    {
        NDDSA_XmlResource_find_first_xml_element(
                lib_xml_head,
                "<domain",
                "</domain>",
                &xml_start,
                &xml_end);

        if (xml_start != NULL)
        {
            if (xml_name != NULL)
            {
                RTIOsapiHeap_free(xml_name);
                xml_name = NULL;
            }
            if (!NDDSA_XmlResource_parse_xml_element_name(
                    xml_start,
                    "<domain",
                    &xml_content_start,
                    &xml_name))
            {
                goto done;
            }
            if (full_name != NULL)
            {
                RTIOsapiHeap_free(full_name);
                full_name = NULL;
            }
            if (!NDDSA_StringUtil_append(
                    lib_full_name, "::", xml_name, &full_name))
            {
                goto done;
            }
            RTIOsapiHeap_free(xml_name);
            xml_name = NULL;

            if (!NDDSA_CreatedResourceLogSeq_ensure_length(
                    domains_out,
                    created_count + 1,
                    created_count + 1))
            {
                goto done;
            }

            id = NDDSA_CreatedResourceLogSeq_get_reference(
                        domains_out, created_count);
            id->kind = D2S2_RESOURCEKIND_DOMAIN;
            if (!D2S2_ResourceId_initialize_ref(&id->id, full_name))
            {
                goto done;
            }

            created_count += 1;
        }

        lib_xml_head = xml_end;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (lib_name != NULL)
    {
        RTIOsapiHeap_free(lib_name);
    }
    if (full_name != NULL)
    {
        RTIOsapiHeap_free(full_name);
    }

    if (!retcode)
    {
        if (initd_lib)
        {
            DDS_DomainParticipantFactory_unload_xml_element(
                    factory, lib_full_name);
        }
        NDDSA_CreatedResourceLogSeq_set_length(domains_out, 0);
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceFactory_create_qos_library_xml(
    NDDSA_ResourceFactory *const self,
    const char *const qos_lib_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const qos_profiles_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_qos_library_xml)
    RTIBool retcode = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *el_xml_start = NULL,
               *el_xml_end = NULL,
               *inserted_tag_name = NULL,
               *xml_content_start = NULL;
    char *lib_name = NULL;

    D2S2Log_fn_entry()

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    NDDSA_XmlResource_find_first_xml_element(
                qos_lib_xml,
                "<qos_library",
                "</qos_library>",
                &el_xml_start,
                &el_xml_end);
    if (el_xml_start == NULL ||
        el_xml_end == NULL)
    {
        goto done;
    }

    rc = DDS_DomainParticipantFactory_load_xml_element(
                factory, &inserted_tag_name, "", el_xml_start);
    if (DDS_RETCODE_OK != rc)
    {
        /* TODO log */
        goto done;
    }

    if (!NDDSA_XmlResource_parse_xml_element_name(
            qos_lib_xml,
            "<qos_library",
            &xml_content_start,
            &lib_name))
    {
        goto done;
    }
    if (!NDDSA_ResourceFactory_create_qosprofile_xml(
            self,
            factory,
            lib_name,
            xml_content_start,
            el_xml_end - strlen("</qos_library>"),
            properties,
            qos_profiles_out))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (lib_name != NULL)
    {
        RTIOsapiHeap_freeString(lib_name);
    }
    
    D2S2Log_fn_exit()
    return retcode;
}

void
NDDSA_ResourceFactory_unload_resource_xml(
    NDDSA_ResourceFactory *const self,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_unload_resource_xml)
    RTIBool retcode = RTI_FALSE,
            initd_name = RTI_FALSE;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *resource_ref = NULL;
    char *resource_ref_own = NULL;
    D2S2_EntityName resource_name = D2S2_ENTITYNAME_INITIALIZER;

    D2S2Log_fn_entry()

    UNUSED_ARG(self);

    NDDSA_ResourceFactory_enter_ea(self);

    if (resource_id->kind != D2S2_RESOURCEIDKIND_REF)
    {
        goto done;
    }
    resource_ref = resource_id->value.ref;

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        goto done;
    }

    if (!D2S2_EntityName_from_id(&resource_name, resource_id))
    {
        goto done;
    }
    initd_name = RTI_TRUE;

    switch (resource_kind)
    {
    case D2S2_RESOURCEKIND_DOMAINPARTICIPANT:
    {
        /* Must delete <participant_library> */
        if (!D2S2_EntityName_to_ref(
                &resource_name,
                D2S2_ENTITYNAME_DEPTH_LIBRARY,
                &resource_ref_own))
        {
            goto done;
        }
        resource_ref = resource_ref_own;
        break;
    }
    case D2S2_RESOURCEKIND_TOPIC:
    {
        /* must delete <topic> */
        resource_ref = resource_id->value.ref;
        break;
    }
    case D2S2_RESOURCEKIND_PUBLISHER:
    {
        /* must delete <publisher> */
        resource_ref = resource_id->value.ref;
        break;
    }
    case D2S2_RESOURCEKIND_SUBSCRIBER:
    {
        /* Must delete <subscriber> */
        resource_ref = resource_id->value.ref;
        break;
    }
    case D2S2_RESOURCEKIND_DATAWRITER:
    {
        /* Must delete <data_writer> */
        resource_ref = resource_id->value.ref;
        break;
    }
    case D2S2_RESOURCEKIND_DATAREADER:
    {
        /* Must delete <data_reader */
        resource_ref = resource_id->value.ref;
        break;
    }
    case D2S2_RESOURCEKIND_DOMAIN:
    {
        /* Must delete <domain_library> */
        if (!D2S2_EntityName_to_ref(
                &resource_name,
                D2S2_ENTITYNAME_DEPTH_LIBRARY,
                &resource_ref_own))
        {
            goto done;
        }
        resource_ref = resource_ref_own;
        break;
    }
    case D2S2_RESOURCEKIND_TYPE:
    {
        /* Must delete <types> */
        resource_ref = resource_id->value.ref;
        break;
    }
    case D2S2_RESOURCEKIND_QOSPROFILE:
    {
        /* must delete <qos_profile_library> */
        if (!D2S2_EntityName_to_ref(
                &resource_name,
                D2S2_ENTITYNAME_DEPTH_LIBRARY,
                &resource_ref_own))
        {
            goto done;
        }
        resource_ref = resource_ref_own;
        break;
    }
    case D2S2_RESOURCEKIND_APPLICATION:
    {
        /* must delete <application_library> */
        if (!D2S2_EntityName_to_ref(
                &resource_name,
                D2S2_ENTITYNAME_DEPTH_LIBRARY,
                &resource_ref_own))
        {
            goto done;
        }
        resource_ref = resource_ref_own;
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE:
    {
        /* Must delete <service */
        resource_ref = resource_id->value.ref;
        break;
    }
    case D2S2_RESOURCEKIND_SERVICE_RESOURCE:
    {
        /* Must delete <resource */
        resource_ref = resource_id->value.ref;
        break;
    }
    default:
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_unload_xml_element(
                factory, resource_ref))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        D2S2Log_exception(
            method_name,
            &RTI_LOG_ANY_ss,
            "FAILED to unload XML for resource: ",
            resource_id->value.ref);
    }
    else
    {
        D2S2Log_local(
            method_name,
            &RTI_LOG_ANY_ss,
            "UNLOADED XML for resource: ",
            resource_id->value.ref);
    }
    NDDSA_ResourceFactory_leave_ea(self);
    if (resource_ref_own)
    {
        RTIOsapiHeap_freeString(resource_ref_own);
    }
    if (initd_name)
    {
        D2S2_EntityName_finalize(&resource_name);
    }
    D2S2Log_fn_exit()
    return;
}

RTIBool
NDDSA_ResourceFactory_create_service_xml(
    NDDSA_ResourceFactory *const self,
    const char *const svc_server_xml,
    const D2S2_ResourceProperties *const properties,
    struct NDDSA_CreatedResourceLogSeq *const svc_out,
    struct NDDSA_CreatedResourceLogSeq *const svc_resources_out)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceFactory_create_service_xml)
    RTIBool retcode = RTI_FALSE;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    DDS_DomainParticipantFactory *factory = NULL;
    const char *server_full_name = NULL,
               *el_full_name = NULL;
    NDDSA_CreatedResourceLog * server_id = NULL;
    DDS_UnsignedLong seq_len = 0,
                     seq_max = 0,
                     el_i = 0;
    NDDSA_CreatedResourceLog *el_id = NULL;
    struct DDS_XMLObject * xml_svc = NULL,
                         * xml_root = NULL;
    NDDSA_ServiceXml * root = NULL;
    NDDSA_ServiceResourceXml * child = NULL;

    D2S2Log_fn_entry()

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    rc = DDS_DomainParticipantFactory_load_xml_element(
        factory, &server_full_name, "", svc_server_xml);
    if (DDS_RETCODE_OK != rc)
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

    xml_svc = DDS_XMLObject_lookup(xml_root, server_full_name);
    if (NULL == xml_svc)
    {
        /* TODO log */
        goto done;
    }

    if (strcmp(NDDSA_SERVICE_XML_TAG, DDS_XMLObject_get_tag_name(xml_svc)) != 0)
    {
        /* TODO log */
        goto done;
    }

    root = (NDDSA_ServiceXml*)xml_svc;

    child = (NDDSA_ServiceResourceXml*)
        DDS_XMLObject_get_first_child_with_tag(&root->base, NDDSA_SERVICE_RESOURCE_XML_TAG);
    el_i = NDDSA_CreatedResourceLogSeq_get_length(svc_resources_out);
    while (NULL != child)
    {
        if (!NDDSA_CreatedResourceLogSeq_ensure_length(
                svc_resources_out, el_i + 1, el_i + 1))
        {
            /* TODO log */
            goto done;
        }

        el_id = NDDSA_CreatedResourceLogSeq_get_reference(svc_resources_out, el_i);
        el_id->kind = D2S2_RESOURCEKIND_SERVICE_RESOURCE;
        el_id->data = child;
        el_full_name = DDS_XMLObject_get_fully_qualified_name(&child->base);
        if (!D2S2_ResourceId_initialize_ref(&el_id->id, el_full_name))
        {
            /* TODO log */
            goto done;
        }

        el_i += 1;
        child = (NDDSA_ServiceResourceXml*) DDS_XMLObject_get_next_sibling(&child->base);
    }

    seq_len = NDDSA_CreatedResourceLogSeq_get_length(svc_out);
    seq_max = NDDSA_CreatedResourceLogSeq_get_maximum(svc_out);
    seq_max = (seq_max < seq_len + 1)? seq_len + 1 : seq_max;
    if (!NDDSA_CreatedResourceLogSeq_ensure_length(svc_out, seq_len + 1, seq_max))
    {
        /* TODO log */
        goto done;
    }
    server_id = 
      NDDSA_CreatedResourceLogSeq_get_reference(svc_out, seq_len + 1);
    server_id->kind = D2S2_RESOURCEKIND_SERVICE;
    server_id->data = root;
    if (!D2S2_ResourceId_initialize_ref(&server_id->id, server_full_name))
    {
        /* TODO log */
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */
