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

#include "NddsClient.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

RTIBool
NDDSA_Client_initialize(NDDSA_Client *self)
{
    NDDSA_Client def_self = NDDSA_CLIENT_INITIALIZER;
    *self = def_self;
    return RTI_TRUE;
}

RTIBool
NDDSA_Client_finalize(NDDSA_Client *self)
{
    RTIBool res = RTI_FALSE;
    NDDSA_Client def_self = NDDSA_CLIENT_INITIALIZER;
    if (REDAInlineList_getFirst(&self->sessions) != NULL)
    {
        goto done;
    }
    *self = def_self;
    res = RTI_TRUE;
done:
    return res;
}

RTIBool
NDDSA_Client_add_session(
    NDDSA_Client *const self,
    NDDSA_ClientSessionRecord *const session)
{
    REDAInlineList_addNodeToBackEA(&self->sessions, &session->node);
    return RTI_TRUE;
}

RTIBool
NDDSA_Client_remove_session(
    NDDSA_Client *const self,
    NDDSA_ClientSessionRecord *const session)
{
    REDAInlineList_removeNodeEA(&self->sessions, &session->node);
    return RTI_TRUE;
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */