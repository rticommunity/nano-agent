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

#include "nano/nano_agent.h"
#include "Agent.h"

NANO_RetCode
NANO_XRCE_ProxyClientTransport_initialize(
    NANO_XRCE_ClientTransport *const transport,
    const NANO_XRCE_ClientTransportListener *const listener,
    const NANO_XRCE_ClientTransportProperties *const properties)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_ProxyClientTransport *self = 
        (NANO_XRCE_ProxyClientTransport*)transport;
    NANO_XRCE_ProxyClientTransportProperties *props =
        (NANO_XRCE_ProxyClientTransportProperties*)properties;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(listener != NULL)
    NANO_PCOND(properties != NULL)

    self->base.listener = *listener;
    self->key = props->key;
    self->id = props->id;
    self->bind_entry.key = props->key;
    
    NANO_LOG_DEBUG("INITIALIZED ProxyClientTransport",
        NANO_LOG_KEY("key", self->key))
    
    rc = NANO_RETCODE_OK;
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_XRCE_ProxyClientTransport_finalize(
    NANO_XRCE_ClientTransport *const transport)
{
    NANO_XRCE_ProxyClientTransport *self = 
        (NANO_XRCE_ProxyClientTransport*)transport;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(transport != NULL)

    NANO_LOG_DEBUG("FINALIZED ProxyClientTransport",
        NANO_LOG_KEY("key", self->key))
    
    NANO_LOG_FN_EXIT
}

/* This method is never called since Session_run is never invoked by the agent */
NANO_RetCode
NANO_XRCE_ProxyClientTransport_process_input(
    NANO_XRCE_ClientTransport *const transport,
    NANO_u32 max_messages,
    NANO_Timeout timeout_ms)
{
    UNUSED_ARG(transport);
    UNUSED_ARG(max_messages);
    UNUSED_ARG(timeout_ms);
    return NANO_RETCODE_ERROR;
}

void
NANO_XRCE_ProxyClientTransport_return_message(
    NANO_XRCE_ClientTransport *const transport,
    NANO_MessageBuffer *const msg)
{
    NANO_XRCE_ProxyClientTransport *self = 
        (NANO_XRCE_ProxyClientTransport*)transport;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(self);
    UNUSED_ARG(msg);

    NANO_PCOND(self != NULL)
    NANO_PCOND(msg != NULL)

    /* This function should never be called on the agent */
    NANO_UNREACHABLE_CODE

    NANO_LOG_FN_EXIT
}

void
NANO_XRCE_ProxyClientTransport_flush_output(
    NANO_XRCE_ClientTransport *const transport)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_MessageBuffer *nxt = NULL;
    NANO_XRCE_ProxyClientTransport *self = 
        (NANO_XRCE_ProxyClientTransport*)transport;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    while (NANO_XRCE_ClientTransport_pending_send(&self->base))
    {
        nxt = NANO_XRCE_ClientTransport_next_send(&self->base);

        NANO_LOG_TRACE("SEND msg TO",
            NANO_LOG_KEY("client", self->bind_entry.key)
            NANO_LOG_LOCATOR("locator", &self->bind_entry.locator)
            NANO_LOG_PTR("client-transport", self)
            NANO_LOG_PTR("agent-transport", self->bind_entry.transport)
            NANO_LOG_MBUF("msg", nxt))
        NANO_CHECK_RC(
            NANO_XRCE_AgentTransport_send_to(
                self->bind_entry.transport,
                &self->bind_entry,
                nxt),
            NANO_LOG_ERROR("FAILED to send message on transport",
                NANO_LOG_KEY("key",self->key)
                NANO_LOG_PTR("transport",transport)
                NANO_LOG_MBUF("msg",nxt)));
        
        NANO_XRCE_ClientTransport_send_complete(
            &self->base, 0, nxt, NANO_RETCODE_OK);
    }

    rc = NANO_RETCODE_OK;


done:
    if (NANO_RETCODE_OK != rc)
    {

    }
    NANO_LOG_FN_EXIT_RC(rc)

}

NANO_RetCode
NANO_XRCE_ProxyClientTransport_update_locator(
    NANO_XRCE_ClientTransport *const transport,
    const NANO_XRCE_ClientTransportLocatorType locator_type,
    const NANO_XRCE_TransportLocator *const locator)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(transport);
    UNUSED_ARG(locator_type);
    UNUSED_ARG(locator);

    NANO_PCOND(transport != NULL)
    NANO_PCOND(locator != NULL)
    NANO_PCOND(NANO_XRCE_ClientTransportLocatorType_is_valid(locator_type))

    /* This function should never be called on the agent */
    NANO_UNREACHABLE_CODE
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}


void
NANO_XRCE_ProxyClientTransport_on_message_received(
    NANO_XRCE_ProxyClientTransport *const self,
    NANO_MessageBuffer *const msg,
    NANO_usize msg_len,
    NANO_bool *const retained)
{
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(msg != NULL)
    NANO_PCOND(msg_len > 0)
    NANO_PCOND(retained != NULL)

    /* Propagate message to upstream listener (i.e. Session) */
    NANO_XRCE_ClientTransportListener_on_message_received(
        &self->base.listener, &self->base, msg, msg_len, retained);

    NANO_LOG_FN_EXIT
}

NANO_RetCode
NANO_XRCE_ProxyClientTransport_set_bind_entry(
    NANO_XRCE_ProxyClientTransport *const self,
    NANO_XRCE_AgentTransport *const src_transport,
    const NANO_XRCE_TransportLocator *const src_locator)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    self->bind_entry.transport = src_transport;
    NANO_CHECK_RC(
        NANO_XRCE_TransportLocator_copy(&self->bind_entry.locator, src_locator),
        NANO_LOG_ERROR_MSG("FAILED to copy transport locator"));
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}
