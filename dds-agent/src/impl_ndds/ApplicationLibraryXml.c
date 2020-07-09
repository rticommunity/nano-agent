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

#include "ApplicationLibraryXml.h"
#include "NddsInfrastructure.h"


typedef struct NDDSA_ApplicationLibraryXmlI
{
    struct DDS_XMLObject base;
    char *name;
    struct DDS_StringSeq applications;
    struct DDS_StringSeq participants;
} NDDSA_ApplicationLibraryXml;

RTI_PRIVATE
struct DDS_XMLObject*
NDDSA_ApplicationLibraryXml_on_new(
    const struct DDS_XMLExtensionClass *extension_class,
    const struct DDS_XMLObject *parent_object,
    const char **attr,
    struct DDS_XMLContext *context);

RTI_PRIVATE
void
NDDSA_ApplicationLibraryXml_on_delete(struct DDS_XMLObject *self);

RTI_PRIVATE
void
NDDSA_ApplicationLibraryXml_on_start_tag(
    struct DDS_XMLObject *self,
    const char *tag_name,
    const char **attr,
    struct DDS_XMLContext *context);

RTI_PRIVATE
void
NDDSA_ApplicationLibraryXml_on_end_tag(
    struct DDS_XMLObject *self,
    const char *tag_name,
    const char *element_text,
    struct DDS_XMLContext *context);

/******************************************************************************
 * Function implementations
 ******************************************************************************/

struct DDS_XMLObject*
NDDSA_ApplicationLibraryXml_on_new(
    const struct DDS_XMLExtensionClass *extension_class,
    const struct DDS_XMLObject *parent_object,
    const char **attr,
    struct DDS_XMLContext *context)
{
    NDDSA_ApplicationLibraryXml *config = NULL;
    const char *name = NULL;

    UNUSED_ARG(context);

    RTIOsapiHeap_allocateStructure(&config, NDDSA_ApplicationLibraryXml);
    if (config == NULL)
    {
        return NULL;
    }
    RTIOsapiMemory_zero(config, sizeof(NDDSA_ApplicationLibraryXml));

    name = DDS_XMLHelper_get_attribute_value(attr, "name");
    if (!DDS_XMLObject_initialize(
            &config->base,
            extension_class,
            parent_object,
            name,
            NULL))
    {
        RTIOsapiHeap_freeStructure(config);
        return NULL;
    }

    return &config->base;
}

void
NDDSA_ApplicationLibraryXml_on_delete(struct DDS_XMLObject *self)
{
    NDDSA_ApplicationLibraryXml *config = (NDDSA_ApplicationLibraryXml*) self;
    if (config == NULL) {
        return;
    }

    DDS_XMLObject_finalize(&config->base);
    DDS_StringSeq_finalize(&config->participants);
    DDS_StringSeq_finalize(&config->applications);
    RTIOsapiHeap_freeStructure(config);
}

void
NDDSA_ApplicationLibraryXml_on_start_tag(
    struct DDS_XMLObject *self,
    const char *tag_name,
    const char **attr,
    struct DDS_XMLContext *context)
{
    UNUSED_ARG(context);
    UNUSED_ARG(tag_name);
    UNUSED_ARG(attr);
    UNUSED_ARG(self);
    /* NOOP */
}

void
NDDSA_ApplicationLibraryXml_on_end_tag(
    struct DDS_XMLObject *xml_obj,
    const char *tag_name,
    const char *element_text,
    struct DDS_XMLContext *context)
{
    RTIBool res = RTI_FALSE;
    NDDSA_ApplicationLibraryXml *self = (NDDSA_ApplicationLibraryXml*) xml_obj;
    struct DDS_XMLObject *application = NULL,
                  *participant = NULL;
    const char *app_lib_name = NULL,
               *app_name = NULL,
               *dp_name = NULL;
    char *app_qname = NULL,
         *dp_qname = NULL;
    DDS_UnsignedLong apps_count = 0,
                     participants_count = 0;
    
    UNUSED_ARG(context);
    UNUSED_ARG(element_text);
    UNUSED_ARG(tag_name);

    if (self == NULL) {
        return;
    }

    app_lib_name = DDS_XMLObject_get_name(&self->base);

    application =
            DDS_XMLObject_get_first_child_with_tag(
                &self->base, "application");
    
    while (application != NULL) 
    {
        app_name = DDS_XMLObject_get_name(application);

        if (!NDDSA_StringUtil_append(app_lib_name, "::", app_name, &app_qname))
        {
            goto done;
        }

        if (DDS_StringSeq_ensure_length(
                &self->applications, apps_count + 1, apps_count + 1))
        {
            goto done;
        }
        apps_count += 1;
        *DDS_StringSeq_get_reference(
            &self->applications, apps_count) = app_qname;
        app_qname = NULL;

        participant = DDS_XMLObject_get_first_child_with_tag(
                            application, "domain_participant");

        while (participant != NULL)
        {
            dp_name = DDS_XMLObject_get_name(participant);
            if (!NDDSA_StringUtil_append(
                    app_name, "::", dp_name, &dp_qname))
            {
                goto done;
            }
            if (DDS_StringSeq_ensure_length(
                    &self->participants,
                    participants_count + 1,
                    participants_count + 1))
            {
                goto done;
            }
            
            participants_count += 1;
            *DDS_StringSeq_get_reference(
                &self->participants, participants_count) = dp_qname;
            dp_qname = NULL;

            participant = DDS_XMLObject_get_next_sibling_with_tag(
                                participant, "domain_participant");
        }

        application = DDS_XMLObject_get_next_sibling_with_tag(
                            application, "application");
    }

    res = RTI_TRUE;
done:
    if (!res)
    {
        if (app_qname != NULL)
        {
            RTIOsapiHeap_free(app_qname);
        }
        if (dp_qname != NULL)
        {
            RTIOsapiHeap_free(dp_qname);
        }
        if (apps_count > 0)
        {
            DDS_StringSeq_ensure_length(
                &self->applications, 0, apps_count);
        }
        if (participants_count > 0)
        {
            DDS_StringSeq_ensure_length(
                &self->participants, 0, participants_count);
        }
    }
}

RTIBool
NDDSA_ApplicationLibraryXml_register_extension(
    struct DDS_XMLParser *const xml_parser)
{
    RTIBool retcode = RTI_FALSE;
    struct DDS_XMLExtensionClass *extension_class = NULL;

    extension_class = DDS_XMLExtensionClass_new(
                            "application_library",
                            NULL,
                            RTI_FALSE,
                            RTI_FALSE,
                            NDDSA_ApplicationLibraryXml_on_start_tag,
                            NDDSA_ApplicationLibraryXml_on_end_tag,
                            NDDSA_ApplicationLibraryXml_on_new,
                            NDDSA_ApplicationLibraryXml_on_delete,
                            NULL);  // save function
    if (extension_class == NULL)
    {
        goto done;
    }

    if (!DDS_XMLParser_register_extension_class(xml_parser, extension_class))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    return retcode;
}


RTIBool
NDDSA_ApplicationLibraryXml_unregister_extension(
    struct DDS_XMLParser *const xml_parser)
{
    RTIBool retcode = RTI_FALSE;
    struct DDS_XMLExtensionClass *extension_class = NULL;

    extension_class =
            DDS_XMLParser_unregister_extension_class(
                xml_parser, "application_library");
    if (extension_class == NULL) 
    {
        goto done;
    }
    DDS_XMLExtensionClass_delete(extension_class);
    
    retcode = RTI_TRUE;
    
done:

    return retcode;
}