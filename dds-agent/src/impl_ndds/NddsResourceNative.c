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

#include "NddsResourceNative.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

/******************************************************************************
 *                                GenericResource
 ******************************************************************************/

RTIBool
NDDSA_GenericResource_initialize(
    NDDSA_GenericResource *self,
    const D2S2_ResourceId *const id)
{
    RTIBool res = RTI_FALSE;
    NDDSA_GenericResource def_self = NDDSA_GENERICRESOURCE_INITIALIZER;
    
    *self = def_self;

    if (!D2S2_ResourceId_copy(&self->id, id))
    {
        goto done;
    }

    res = RTI_TRUE;

done:
    return res;
}

void
NDDSA_GenericResource_finalize(NDDSA_GenericResource *self)
{
    D2S2_ResourceId_finalize(&self->id);
    /* self->data is assumed to be externally managed */
}

RTIBool
NDDSA_GenericResource_copy(
    NDDSA_GenericResource *const dst,
    const NDDSA_GenericResource *const src)
{
    RTIBool res = RTI_FALSE;

    if (!D2S2_ResourceId_copy(&dst->id, &src->id))
    {
        goto done;
    }

    res = RTI_TRUE;
done:
    return res;
}

/******************************************************************************
 *                              ResourceNative
 ******************************************************************************/

RTIBool
NDDSA_ResourceNative_initialize_entity(
    NDDSA_ResourceNative *const self,
    const NDDSA_EntityResource *const entity,
    const D2S2_ResourceRepresentation *const create_repr)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceNative_initialize_entity)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()
    
    self->kind = NDDSA_RESOURCENATIVEKIND_ENTITY;
    self->value.entity = *entity;
    if (!D2S2_ResourceRepresentation_copy(&self->create_repr, create_repr))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_ResourceNative_initialize_generic(
    NDDSA_ResourceNative *const self,
    const NDDSA_GenericResource *const generic,
    const D2S2_ResourceRepresentation *const create_repr)
{
    D2S2Log_METHOD_NAME(NDDSA_ResourceNative_initialize_generic)
    RTIBool res = RTI_FALSE;
    D2S2Log_fn_entry()

    self->kind = NDDSA_RESOURCENATIVEKIND_GENERIC;
    if (!NDDSA_GenericResource_copy(&self->value.generic, generic))
    {
        goto done;
    }
    if (!D2S2_ResourceRepresentation_copy(&self->create_repr, create_repr))
    {
        goto done;
    }

    res = RTI_TRUE;
done:
    D2S2Log_fn_exit()
    return res;
}

void
NDDSA_ResourceNative_finalize(NDDSA_ResourceNative *const self)
{
    if (self->kind == NDDSA_RESOURCENATIVEKIND_GENERIC)
    {
        NDDSA_GenericResource_finalize(&self->value.generic);
    }
    D2S2_ResourceRepresentation_finalize(&self->create_repr);
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

