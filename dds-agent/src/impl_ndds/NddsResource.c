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

#include "NddsResource.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT


/******************************************************************************
 *                                Resource
 ******************************************************************************/

RTIBool
NDDSA_Resource_initialize(
    NDDSA_Resource *self,
    const D2S2_ResourceId *const id,
    const D2S2_ResourceKind kind,
    NDDSA_ResourceNative *const native)
{
    D2S2Log_METHOD_NAME(NDDSA_Resource_initialize)
    RTIBool res = RTI_FALSE;
    NDDSA_Resource def_self = NDDSA_RESOURCE_INITIALIZER;
    *self = def_self;

    D2S2Log_fn_entry()

    if (!D2S2_Resource_initialize(&self->base, kind, id))
    {
        goto done;
    }
    self->native = *native;

    res = RTI_TRUE;

done:

    D2S2Log_fn_exit()
    return res;
}

RTIBool
NDDSA_Resource_can_delete(NDDSA_Resource *self)
{
    return self->attached_count == 0;
}

void
NDDSA_Resource_finalize(NDDSA_Resource *self)
{
    D2S2_Resource_finalize(&self->base);
    NDDSA_ResourceNative_finalize(&self->native);
}

/******************************************************************************
 *                              ResourceRepresentation
 ******************************************************************************/

RTI_PRIVATE
RTIBool
NDDSA_Resource_convert_representation_xml_to_ref(
    NDDSA_Resource *const self,
    const char *const from_xml,
    char **const to_ref_out)
{
    RTIBool retcode = RTI_FALSE;
    UNUSED_ARG(self);
    UNUSED_ARG(from_xml);
    UNUSED_ARG(to_ref_out);
    /* TODO implement me */
    if (1)
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:

    return retcode;
}

RTIBool
NDDSA_Resource_convert_representation(
    NDDSA_Resource *const self,
    const D2S2_ResourceRepresentation *const from,
    const D2S2_ResourceRepresentationFormat out_fmt,
    D2S2_ResourceRepresentation *const to)
{
    D2S2Log_METHOD_NAME(NDDSA_Resource_convert_representation)
    RTIBool retcode = RTI_FALSE;
    D2S2_ResourceRepresentationFormat in_fmt = 
        D2S2_RESOURCEREPRESENTATIONFORMAT_UNKNOWN;

    D2S2Log_fn_entry()

    in_fmt = from->fmt;

    switch (in_fmt)
    {
    case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
    {
        switch (out_fmt)
        {
        case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
        {
            to->fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_XML;
            to->value.xml = DDS_String_dup(from->value.xml);
            if (to->value.xml == NULL)
            {
                goto done;
            }
            break;
        }
        case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
        {
            if (!NDDSA_Resource_convert_representation_xml_to_ref(
                    self, from->value.xml, &to->value.ref))
            {
                goto done;
            }
            to->fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_REF;
            break;
        }
        default:
        {
            /* unsupported conversion */
            goto done;
        }
        }
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
    {
        switch (out_fmt)
        {
        case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
        {
            to->fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_REF;
            to->value.ref = DDS_String_dup(from->value.ref);
            if (to->value.ref == NULL)
            {
                goto done;
            }
            break;
        }
        default:
        {
            /* unsupported conversion */
            goto done;
        }
        }
        break;
    }
    default:
    {
        /* unsupported conversion */
        goto done;
    }
    }
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}


#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */