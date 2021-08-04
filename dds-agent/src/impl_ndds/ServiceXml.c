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

#include "ServiceXml.h"
#include "NddsInfrastructure.h"

RTI_PRIVATE
struct DDS_XMLObject*
NDDSA_ServiceXml_on_new(
    const struct DDS_XMLExtensionClass *extension_class,
    const struct DDS_XMLObject *parent_object,
    const char **attr,
    struct DDS_XMLContext *context);

RTI_PRIVATE
void
NDDSA_ServiceXml_on_delete(struct DDS_XMLObject *self);

RTI_PRIVATE
void
NDDSA_ServiceXml_on_start_tag(
    struct DDS_XMLObject *self,
    const char *tag_name,
    const char **attr,
    struct DDS_XMLContext *context);

RTI_PRIVATE
void
NDDSA_ServiceXml_on_end_tag(
    struct DDS_XMLObject *self,
    const char *tag_name,
    const char *element_text,
    struct DDS_XMLContext *context);

RTI_PRIVATE
struct DDS_XMLObject*
NDDSA_ServiceResourceXml_on_new(
    const struct DDS_XMLExtensionClass *extension_class,
    const struct DDS_XMLObject *parent_object,
    const char **attr,
    struct DDS_XMLContext *context);

RTI_PRIVATE
void
NDDSA_ServiceResourceXml_on_delete(struct DDS_XMLObject *self);

RTI_PRIVATE
void
NDDSA_ServiceResourceXml_on_start_tag(
    struct DDS_XMLObject *self,
    const char *tag_name,
    const char **attr,
    struct DDS_XMLContext *context);

RTI_PRIVATE
void
NDDSA_ServiceResourceXml_on_end_tag(
    struct DDS_XMLObject *self,
    const char *tag_name,
    const char *element_text,
    struct DDS_XMLContext *context);

/******************************************************************************
 * Function implementations
 ******************************************************************************/

struct DDS_XMLObject*
NDDSA_ServiceXml_on_new(
    const struct DDS_XMLExtensionClass *extension_class,
    const struct DDS_XMLObject *parent_object,
    const char **attr,
    struct DDS_XMLContext *context)
{
    struct DDS_XMLObject *result = NULL;
    NDDSA_ServiceXml *config = NULL;
    const char *val = NULL;
    char *fqname = NULL;
    const NDDSA_ExternalService def_entity = NDDSA_EXTERNALSERVICE_INITIALIZER;
    RTIBool obj_initd = RTI_FALSE;
  
    UNUSED_ARG(context);

    RTIOsapiHeap_allocateStructure(&config, NDDSA_ServiceXml);
    if (config == NULL)
    {
        goto done;
    }
    RTIOsapiMemory_zero(config, sizeof(NDDSA_ServiceXml));
    config->entity = def_entity;

    val = DDS_XMLHelper_get_attribute_value(attr, "name");
    if (!DDS_XMLObject_initialize(
            &config->base,
            extension_class,
            parent_object,
            val,
            NULL))
    {
        goto done;
    }
    obj_initd = RTI_TRUE;

    fqname = DDS_String_dup(
        DDS_XMLObject_get_fully_qualified_name(&config->base) + 2);
    if (NULL == fqname)
    {
        goto done;
    }
    D2S2_ResourceId_set_ref(&config->entity.id, fqname);

    val = DDS_XMLHelper_get_attribute_value(attr, "plugin");
    if (NULL == val || val[0] == '\0')
    {
        D2S2Log_freeForm(RTI_LOG_BIT_EXCEPTION)(
            "missing required attribute for <%s/>: %s\n", 
            NDDSA_SERVICE_XML_TAG, "plugin");
        goto done;
    }
    config->entity.plugin_name = DDS_String_dup(val);
    if (NULL == config->entity.plugin_name)
    {
        goto done;
    }

    val = DDS_XMLHelper_get_attribute_value(attr, "url");
    if (NULL != val)
    {
        config->entity.url = DDS_String_dup(val);
        if (NULL == config->entity.url)
        {
            goto done;
        }
    }

    result = &config->base;
done:
    if (NULL == result)
    {
        if (NULL != config)
        {
            if (NULL != config->entity.id.value.ref)
            {
                DDS_String_free(config->entity.id.value.ref);
            }
            if (NULL != config->entity.plugin_name)
            {
                DDS_String_free(config->entity.plugin_name);
            }
            if (obj_initd)
            {
                DDS_XMLObject_finalize(&config->base);
            }
            RTIOsapiHeap_freeStructure(config);
        }
    }
    return result;
}

void
NDDSA_ServiceXml_on_delete(struct DDS_XMLObject *self)
{
    NDDSA_ServiceXml *config = (NDDSA_ServiceXml*) self;
    if (config == NULL) {
        return;
    }

    DDS_XMLObject_finalize(&config->base);

    D2S2_ResourceId_finalize(&config->entity.id);

    if (NULL != config->entity.plugin_name)
    {
        DDS_String_free(config->entity.plugin_name);
    }
    if (NULL != config->entity.url)
    {
        DDS_String_free(config->entity.url);
    }
    if (config->entity.descriptor)
    {
        DDS_String_free(config->entity.descriptor);
    }
    RTIOsapiHeap_freeStructure(config);
}

void
NDDSA_ServiceXml_on_start_tag(
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
NDDSA_ServiceXml_on_end_tag(
    struct DDS_XMLObject *xml_obj,
    const char *tag_name,
    const char *element_text,
    struct DDS_XMLContext *context)
{
    RTIBool res = RTI_FALSE;
    NDDSA_ServiceXml *self = (NDDSA_ServiceXml*) xml_obj;
    NDDSA_ServiceDescriptorXml * descriptor = NULL;
    NDDSA_ServiceResourceXml * resource = NULL;

    UNUSED_ARG(context);
    UNUSED_ARG(element_text);
    UNUSED_ARG(tag_name);

    if (self == NULL) {
        return;
    }

    if (strcmp(NDDSA_SERVICE_DESCRIPTOR_XML_TAG, tag_name) == 0)
    {
        if (NULL != element_text)
        {
            self->entity.descriptor = DDS_String_dup(element_text);
            if (NULL == self->entity.descriptor)
            {
                goto done;
            }
        }

        res = RTI_TRUE;
        goto done;
    }

    resource = (NDDSA_ServiceResourceXml*)
        DDS_XMLObject_get_first_child_with_tag(&self->base, NDDSA_SERVICE_RESOURCE_XML_TAG);
    while (NULL != resource)
    {
        REDAInlineList_addNodeToBackEA(
          &self->entity.resources, &resource->entity.node);
        resource->entity.service = &self->entity;
        resource = (NDDSA_ServiceResourceXml*)
            DDS_XMLObject_get_next_sibling_with_tag(&resource->base, NDDSA_SERVICE_RESOURCE_XML_TAG);
    }

    res = RTI_TRUE;
done:
    if (!res)
    {

    }
}

struct DDS_XMLObject*
NDDSA_ServiceResourceXml_on_new(
    const struct DDS_XMLExtensionClass *extension_class,
    const struct DDS_XMLObject *parent_object,
    const char **attr,
    struct DDS_XMLContext *context)
{
    struct DDS_XMLObject *result = NULL;
    NDDSA_ServiceResourceXml *config = NULL;
    const char *val = NULL;
    char *fqname = NULL;
    const NDDSA_ExternalServiceResource def_entity = NDDSA_EXTERNALSERVICERESOURCE_INITIALIZER;
    RTIBool obj_initd = RTI_FALSE;

    UNUSED_ARG(context);

    RTIOsapiHeap_allocateStructure(&config, NDDSA_ServiceResourceXml);
    if (config == NULL)
    {
        goto done;
    }
    RTIOsapiMemory_zero(config, sizeof(NDDSA_ServiceResourceXml));
    config->entity = def_entity;

    val = DDS_XMLHelper_get_attribute_value(attr, "name");
    if (!DDS_XMLObject_initialize(
            &config->base,
            extension_class,
            parent_object,
            val,
            NULL))
    {
        goto done;
    }

    fqname = DDS_String_dup(
        DDS_XMLObject_get_fully_qualified_name(&config->base) + 2);
    if (NULL == fqname)
    {
        goto done;
    }
    D2S2_ResourceId_set_ref(&config->entity.id, fqname);

    val = DDS_XMLHelper_get_attribute_value(attr, "path");
    if (NULL != val)
    {
        config->entity.path = DDS_String_dup(val);
        if (NULL == config->entity.path)
        {
            goto done;
        }
    }

    result = &config->base;
done:
    if (NULL == result)
    {
        if (NULL != config->entity.id.value.ref)
        {
            DDS_String_free(config->entity.id.value.ref);
        }
        if (obj_initd)
        {
            DDS_XMLObject_finalize(&config->base);
        }
        RTIOsapiHeap_freeStructure(config);
    }
    return result;
}

void
NDDSA_ServiceResourceXml_on_delete(struct DDS_XMLObject *self)
{
    NDDSA_ServiceResourceXml *config = (NDDSA_ServiceResourceXml*) self;
    if (config == NULL) {
        return;
    }

    DDS_XMLObject_finalize(&config->base);
    D2S2_ResourceId_finalize(&config->entity.id);
    if (NULL != config->entity.path)
    {
        DDS_String_free(config->entity.path);
    }
    if (NULL != config->entity.descriptor)
    {
        DDS_String_free(config->entity.descriptor);
    }
    RTIOsapiHeap_freeStructure(config);
}

void
NDDSA_ServiceResourceXml_on_start_tag(
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
NDDSA_ServiceResourceXml_on_end_tag(
    struct DDS_XMLObject *xml_obj,
    const char *tag_name,
    const char *element_text,
    struct DDS_XMLContext *context)
{
    RTIBool res = RTI_FALSE;
    NDDSA_ServiceResourceXml *self = (NDDSA_ServiceResourceXml*) xml_obj;

    UNUSED_ARG(context);
    UNUSED_ARG(element_text);
    UNUSED_ARG(tag_name);

    if (self == NULL) {
        goto done;
    }

    if (NULL != element_text)
    {
        self->entity.descriptor = DDS_String_dup(element_text);
        if (NULL == self->entity.descriptor)
        {
            goto done;
        }
    }

    res = RTI_TRUE;
done:
    if (!res)
    {

    }
}


RTIBool
NDDSA_ServiceXml_register_extension(
    struct DDS_XMLParser *const xml_parser)
{
    RTIBool retcode = RTI_FALSE,
            server_registered = RTI_FALSE;
    struct DDS_XMLExtensionClass *server_extension_class = NULL,
                                 *res_extension_class = NULL;

    server_extension_class = DDS_XMLExtensionClass_new(
        NDDSA_SERVICE_XML_TAG,
        NULL,
        RTI_FALSE,
        RTI_FALSE,
        NDDSA_ServiceXml_on_start_tag,
        NDDSA_ServiceXml_on_end_tag,
        NDDSA_ServiceXml_on_new,
        NDDSA_ServiceXml_on_delete,
        NULL);
    if (server_extension_class == NULL)
    {
        goto done;
    }

    if (!DDS_XMLParser_register_extension_class(xml_parser, server_extension_class))
    {
        goto done;
    }
    server_registered = RTI_TRUE;

    res_extension_class = DDS_XMLExtensionClass_new(
        NDDSA_SERVICE_RESOURCE_XML_TAG,
        NULL,
        RTI_FALSE,
        RTI_FALSE,
        NDDSA_ServiceResourceXml_on_start_tag,
        NDDSA_ServiceResourceXml_on_end_tag,
        NDDSA_ServiceResourceXml_on_new,
        NDDSA_ServiceResourceXml_on_delete,
        NULL);
    if (res_extension_class == NULL)
    {
        goto done;
    }

    if (!DDS_XMLParser_register_extension_class(xml_parser, res_extension_class))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (server_registered)
        {
            server_extension_class =
                DDS_XMLParser_unregister_extension_class(xml_parser, NDDSA_SERVICE_XML_TAG);
        }
        if (NULL != server_extension_class)
        {
            DDS_XMLExtensionClass_delete(server_extension_class);
        }
        if (NULL != res_extension_class)
        {
            DDS_XMLExtensionClass_delete(res_extension_class);
        }
    }

    return retcode;
}


RTIBool
NDDSA_ServiceXml_unregister_extension(
    struct DDS_XMLParser *const xml_parser)
{
    RTIBool retcode = RTI_FALSE;
    struct DDS_XMLExtensionClass *extension_class = NULL;

    extension_class =
        DDS_XMLParser_unregister_extension_class(xml_parser, NDDSA_SERVICE_XML_TAG);
    if (extension_class == NULL) 
    {
        goto done;
    }
    DDS_XMLExtensionClass_delete(extension_class);

    extension_class =
        DDS_XMLParser_unregister_extension_class(xml_parser, NDDSA_SERVICE_DESCRIPTOR_XML_TAG);
    if (extension_class == NULL) 
    {
        goto done;
    }
    DDS_XMLExtensionClass_delete(extension_class);
  
    retcode = RTI_TRUE;
    
done:

    return retcode;
}