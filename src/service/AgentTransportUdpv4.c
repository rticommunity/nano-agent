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

#include "AgentTransportUdpv4.h"

#include "osapi/osapi_thread.h"

#include "Agent.h"

#include "nano/nano_core_xrce_transport_udpv4.h"

/******************************************************************************
 *                          Udpv4AgentTransport
 ******************************************************************************/

const NANO_u32 ZERO = 0;

#define NANO_XRCE_Udpv4AgentTransport_valid_locator_medium(l_) \
    ((NANO_OSAPI_Memory_compare(\
        (l_)->address, &ZERO, sizeof(ZERO)) != 0) && (l_)->port > 0)




NANO_PRIVATE
NANO_MessageBuffer*
NANO_XRCE_Udpv4AgentTransport_allocate_message(
    NANO_XRCE_Udpv4AgentTransport *const self,
    const NANO_bool metadata)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_MessageBuffer *res = NULL;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    NANO_CHECK_RC(
        NANO_MessageBufferPool_allocate(&self->recv_pool, &res),
        NANO_LOG_ERROR("FAILED to allocate recv message",
            NANO_LOG_PTR("transport",self)));
    
    if (metadata)
    {
        NANO_MessageBuffer_flags_set(res, NANO_XRCE_MESSAGEFLAGS_DISCOVERY);
    }

done:
    
    NANO_LOG_FN_EXIT_PTR(res)
    return res;
}


NANO_PRIVATE
void
NANO_XRCE_Udpv4AgentTransport_release_message(
    NANO_XRCE_Udpv4AgentTransport *const self,
    NANO_MessageBuffer *const msg)
{
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(msg != NULL)

    NANO_MessageBufferPool_release(&self->recv_pool, msg);

    NANO_LOG_FN_EXIT
}

void
NANO_XRCE_Udpv4AgentTransport_recv_thread_impl(
    NANO_XRCE_Udpv4AgentTransport *const self,
    struct RTIOsapiThread *recv_thread,
    NANO_OSAPI_Udpv4Socket *const socket,
    const NANO_bool metadata)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_TransportLocatorMedium src_ip =
            NANO_XRCE_TRANSPORTLOCATORMEDIUM_INITIALIZER;
    NANO_XRCE_TransportLocator src =
            NANO_XRCE_TRANSPORTLOCATOR_INITIALIZER;
    NANO_MessageBuffer *msg = NULL;
    NANO_usize msg_len = 0,
               msg_len_max = 0;
    NANO_bool retained = NANO_BOOL_FALSE;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(recv_thread);

    NANO_PCOND(self != NULL)

    while (self->active)
    {
        if (msg == NULL)
        {
            msg =
                NANO_XRCE_Udpv4AgentTransport_allocate_message(self, metadata);
            if (msg == NULL)
            {
                NANO_LOG_ERROR_MSG("FAILED to allocate receive message");
                goto done;
            }
            msg_len_max = NANO_MessageBuffer_data_len(msg);
        }

        rc = NANO_OSAPI_Udpv4Socket_recv(
                    socket,
                    src_ip.address,
                    &src_ip.port,
                    msg,
                    &msg_len);

        if (NANO_RETCODE_OK == rc)
        {
            if ((msg_len >= NANO_XRCE_MESSAGE_SERIALIZED_SIZE_MIN))
            {
                NANO_LOG_TRACE("possible XRCE(UDP) data RECEIVED",
                        NANO_LOG_LOCATOR_MEDIUM("src",&src_ip)
                        NANO_LOG_USIZE("size",msg_len))

                NANO_XRCE_TransportLocatorMedium_to_locator(&src_ip,&src);

                NANO_XRCE_AgentTransportListener_on_message_received(
                    &self->base.listener,
                    &self->base,
                    &src,
                    msg,
                    msg_len,
                    &retained);
            }
        }
        else if (NANO_RETCODE_TIMEOUT == rc)
        {
            NANO_LOG_TRACE_FN_MSG("socket::recv timedout")
        }
        else
        {
            NANO_LOG_ERROR("socket::recv failed",NANO_LOG_RC(rc))
        }
        rc = NANO_RETCODE_ERROR;

        if (retained)
        {
            /* message was retained by upstream listener, we will allocate a 
               new one on the next iteration */
            msg = NULL;
        }
        else
        {
            /* reset length of msg to it's maximum length, and reuse it */
            NANO_MessageBuffer_set_data_len(msg, msg_len_max);
        }

        retained = NANO_BOOL_FALSE;
    }

    rc = NANO_RETCODE_OK;

done:

    if (NANO_RETCODE_OK != rc)
    {

    }

    if (RTI_OSAPI_SEMAPHORE_STATUS_OK !=
            RTIOsapiSemaphore_give(self->sem_thread_exit))
    {
        NANO_LOG_ERROR("FAILED to give exit semaphore",
            NANO_LOG_PTR("agent",self))
    }
    NANO_LOG_FN_EXIT_RC(rc)
}


void*
NANO_XRCE_Udpv4AgentTransport_recv_thread(void* thread_param)
{
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)thread_param;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    NANO_XRCE_Udpv4AgentTransport_recv_thread_impl(
        self, self->recv_thread, &self->client_socket, NANO_BOOL_FALSE);
    
    NANO_LOG_FN_EXIT
    return NULL;
}

void*
NANO_XRCE_Udpv4AgentTransport_recv_thread_metadata(void* thread_param)
{
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)thread_param;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    NANO_XRCE_Udpv4AgentTransport_recv_thread_impl(
        self, self->metadata_thread, &self->metadata_socket, NANO_BOOL_TRUE);
    
    NANO_LOG_FN_EXIT
    return NULL;
}


NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_initialize(
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_AgentTransportListener *const listener,
    const NANO_XRCE_AgentTransportProperties *const properties,
    const NANO_XRCE_AgentTransportImplProperties *const impl_properties)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)transport;
    const NANO_XRCE_Udpv4AgentTransportProperties *const props =
        (NANO_XRCE_Udpv4AgentTransportProperties*)properties;
    const NANO_XRCE_TransportLocatorMedium *med_address = NULL;
    const NANO_u16 agent_port = NANO_XRCE_TRANSPORT_UDP_AGENT_PORT_DEFAULT,
                   metadata_port = NANO_XRCE_TRANSPORT_UDP_METADATA_PORT_DEFAULT;
    NANO_bool recv_pool_initd = NANO_BOOL_FALSE;
    NANO_OSAPI_Udpv4SocketProperties metadata_socket_props =
        NANO_OSAPI_UDPV4SOCKETPROPERTIES_INITIALIZER;
    NANO_Ipv4Addr metadata_addr = 0,
                  bind_ip = 0;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(properties != NULL)
    NANO_PCOND(impl_properties != NULL)
    NANO_PCOND(impl_properties->agent != NULL)

    self->base.agent = impl_properties->agent;

    /* store the provided listener */
    if (listener != NULL)
    {
        self->base.listener = *listener;
    }

    /* Initialize a pool of message buffers to receive messages in */
    if (props->base.mtu_max == 0 || props->base.mtu_max > NANO_U16_MAX)
    {
        NANO_LOG_ERROR("INVALID max mtu",
            NANO_LOG_USIZE("msg_size",props->base.mtu_max)
            NANO_LOG_USIZE("max", NANO_U16_MAX))
        goto done;
    }
    NANO_CHECK_RC(
        NANO_MessageBufferPool_initialize(
            &self->recv_pool, props->base.mtu_max),
        NANO_LOG_ERROR("FAILED to initialize receive pool",
            NANO_LOG_USIZE("msg_size",props->base.mtu_max)));
    recv_pool_initd = NANO_BOOL_TRUE;

    /* Open receive socket */

    med_address = 
        NANO_XRCE_TransportLocatorMedium_narrow(&props->base.bind_address);
    if (med_address != NULL)
    {
        self->bind_address = *med_address;
    }

    self->bind_address.port =
        (self->bind_address.port > 0)? self->bind_address.port : agent_port;
    
    NANO_LOG_DEBUG("opening UDP socket",
        NANO_LOG_PTR("transport", self)
        NANO_LOG_LOCATOR_MEDIUM("bind_address",&self->bind_address))

    bind_ip = 
        NANO_XRCE_TransportLocatorMedium_address_to_ipv4addr(
            &self->bind_address);
    if (bind_ip == 0)
    {
#define MAX_ADDRS 16
        NANO_Ipv4Addr addrs[MAX_ADDRS] = { 0 };
        NANO_usize addrs_len = MAX_ADDRS;

        NANO_CHECK_RC(
            NANO_OSAPI_Ipv4Addr_list(addrs, &addrs_len),
            NANO_LOG_ERROR("FAILED to list ip addresses",
                NANO_LOG_RC(rc)));
        
        if (addrs_len == 0)
        {
            NANO_LOG_ERROR_MSG("FAILED to determine local address")
            goto done;
        }
        
        NANO_XRCE_TransportLocatorMedium_address_from_ipv4addr(
            &self->wakeup_address, addrs[0], self->bind_address.port);
    }
    else
    {
        self->wakeup_address = self->bind_address;
    }
    NANO_LOG_DEBUG("WAKEUP locator",
        NANO_LOG_PTR("transport", self)
        NANO_LOG_LOCATOR_MEDIUM("address",&self->wakeup_address))

    NANO_CHECK_RC(
        NANO_OSAPI_Udpv4Socket_open(
            &self->client_socket,
            self->bind_address.address,
            self->bind_address.port,
            &props->socket),
        NANO_LOG_ERROR("socket::open failed",
            NANO_LOG_PTR("transport", self)
            NANO_LOG_LOCATOR_MEDIUM("bind_addr", &self->bind_address)));

    /* Open metadata socket if discovery was enabled by user */
    if (props->base.enable_discovery)
    {
        med_address = 
            NANO_XRCE_TransportLocatorMedium_narrow(
                &props->base.metadata_address);
        if (med_address != NULL)
        {
            self->metadata_address = *med_address;
        }
        self->metadata_address.port = 
            (self->metadata_address.port > 0)?
                self->metadata_address.port : metadata_port;
        
        metadata_socket_props = props->metadata_socket;

        metadata_addr =
            NANO_XRCE_TransportLocatorMedium_address_to_ipv4addr(
                &self->metadata_address);
        if (NANO_OSAPI_Ipv4Addr_is_multicast(&metadata_addr))
        {
            NANO_OSAPI_Udpv4SocketProperties_set_multicast(
                &metadata_socket_props);
        }

        NANO_LOG_DEBUG("opening metadata UDP socket",
            NANO_LOG_PTR("transport", self)
            NANO_LOG_LOCATOR_MEDIUM("address",&self->metadata_address))

        NANO_CHECK_RC(
            NANO_OSAPI_Udpv4Socket_open(
                &self->metadata_socket,
                self->metadata_address.address,
                self->metadata_address.port,
                &metadata_socket_props),
            NANO_LOG_ERROR("socket::open failed",
                NANO_LOG_PTR("transport", self)
                NANO_LOG_LOCATOR_MEDIUM("bind_addr", &self->metadata_address)));
    }

    self->sem_thread_exit = 
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_COUNTING, NULL);
    if (self->sem_thread_exit == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to create thread exit semaphore")
        goto done;
    }

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (self->sem_thread_exit != NULL)
        {
            RTIOsapiSemaphore_delete(self->sem_thread_exit);
            self->sem_thread_exit = NULL;
        }
        if (NANO_OSAPI_Udpv4Socket_is_valid(&self->metadata_socket))
        {
            NANO_OSAPI_Udpv4Socket_close(&self->metadata_socket);
        }
        if (NANO_OSAPI_Udpv4Socket_is_valid(&self->client_socket))
        {
            NANO_OSAPI_Udpv4Socket_close(&self->client_socket);
        }
        if (recv_pool_initd)
        {
            NANO_MessageBufferPool_finalize(&self->recv_pool);
        }
    }

    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

void
NANO_XRCE_Udpv4AgentTransport_finalize(
    NANO_XRCE_AgentTransport *const transport)
{
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)transport;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(self->recv_thread == NULL)
    NANO_PCOND(self->metadata_thread == NULL)
    NANO_PCOND(self->sem_thread_exit != NULL)

    RTIOsapiSemaphore_delete(self->sem_thread_exit);
    self->sem_thread_exit = NULL;

    NANO_PCOND(self != NULL)
    NANO_PCOND(NANO_OSAPI_Udpv4Socket_is_valid(&self->client_socket))
    
    if (NANO_OSAPI_Udpv4Socket_is_valid(&self->metadata_socket))
    {
        NANO_OSAPI_Udpv4Socket_close(&self->metadata_socket);
    }

    NANO_OSAPI_Udpv4Socket_close(&self->client_socket);

    NANO_MessageBufferPool_finalize(&self->recv_pool);

    NANO_LOG_FN_EXIT
}

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_listen(
    NANO_XRCE_AgentTransport *const transport)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)transport;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(self->recv_thread == NULL)
    NANO_PCOND(self->metadata_thread == NULL)
    NANO_PCOND(NANO_OSAPI_Udpv4Socket_is_valid(&self->client_socket))

    /* Start receive thread */
    self->active = NANO_BOOL_TRUE;

    self->recv_thread = 
        RTIOsapiThread_new(
            NANO_XRCE_UDPV4AGENTTRANSPORT_RECV_THREAD_NAME,
            RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
            RTI_OSAPI_THREAD_OPTION_DEFAULT,
            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
            NULL,
            NANO_XRCE_Udpv4AgentTransport_recv_thread,
            self);
    if (self->recv_thread == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to create RECV thread")
        goto done;
    }

    if (NANO_OSAPI_Udpv4Socket_is_valid(&self->metadata_socket))
    {
        /* Start receive thread for metadata */
        self->metadata_thread = 
            RTIOsapiThread_new(
                NANO_XRCE_UDPV4AGENTTRANSPORT_RECV_METADATA_THREAD_NAME,
                RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
                RTI_OSAPI_THREAD_OPTION_DEFAULT,
                RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
                NULL,
                NANO_XRCE_Udpv4AgentTransport_recv_thread_metadata,
                self);
        if (self->metadata_thread == NULL)
        {
            NANO_LOG_ERROR_MSG("FAILED to create RECV METADATA thread")
            goto done;
        }
    }

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (self->recv_thread != NULL)
        {
            RTIOsapiThread_delete(self->recv_thread);
            self->recv_thread = NULL;
        }

        if (self->metadata_thread != NULL)
        {
            RTIOsapiThread_delete(self->metadata_thread);
            self->metadata_thread = NULL;
        }
    }

    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_close(
    NANO_XRCE_AgentTransport *const transport)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)transport;
    NANO_MessageBufferData exit_msg_buf[
        NANO_MESSAGEBUFFER_INLINE_SIZE(
            NANO_XRCE_MESSAGE_SERIALIZED_SIZE_MIN)] = { 0 };
    NANO_MessageBuffer *const exit_msg = (NANO_MessageBuffer*)&exit_msg_buf;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(self->recv_thread != NULL)

    /* Stop receive threads */
    self->active = NANO_BOOL_FALSE;
    
    NANO_MessageBuffer_flags_set_inline(exit_msg);
    NANO_MessageBuffer_set_data_len(
        exit_msg, NANO_XRCE_MESSAGE_SERIALIZED_SIZE_MIN);
    NANO_CHECK_RC(
        NANO_OSAPI_Udpv4Socket_send(
            &self->client_socket,
            self->wakeup_address.address,
            self->wakeup_address.port,
            exit_msg),
        NANO_LOG_ERROR("FAILED to send wake up msg on socket",
            NANO_LOG_PTR("transport",self)
            NANO_LOG_LOCATOR_MEDIUM("addr",&self->wakeup_address)
            NANO_LOG_MBUF("msg",exit_msg)));

    RTIOsapiSemaphore_take(self->sem_thread_exit, RTI_NTP_TIME_INFINITE);
    RTIOsapiThread_delete(self->recv_thread);
    self->recv_thread = NULL;


    if (self->metadata_thread != NULL)
    {
        NANO_CHECK_RC(
        NANO_OSAPI_Udpv4Socket_send(
            &self->client_socket,
            self->metadata_address.address,
            self->metadata_address.port,
            exit_msg),
        NANO_LOG_ERROR("FAILED to send wake up msg on socket",
            NANO_LOG_PTR("transport",self)
            NANO_LOG_LOCATOR_MEDIUM("addr",&self->metadata_address)
            NANO_LOG_MBUF("msg",exit_msg)));
        RTIOsapiSemaphore_take(self->sem_thread_exit, RTI_NTP_TIME_INFINITE);
        RTIOsapiThread_delete(self->metadata_thread);
        self->metadata_thread = NULL;
    }
    
    rc = NANO_RETCODE_OK;
    
done:
    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_send_to(
    NANO_XRCE_AgentTransport *const transport,
    NANO_XRCE_AgentTransportBindEntry *const client_entry,
    NANO_MessageBuffer *const msg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)transport;
    NANO_XRCE_TransportLocatorMedium *med_locator = NULL;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(client_entry != NULL)
    NANO_PCOND(client_entry->transport == transport)
    NANO_PCOND(NANO_XRCE_ClientKey_is_valid(&client_entry->key))

    med_locator =
        NANO_XRCE_TransportLocatorMedium_narrow(&client_entry->locator);
    NANO_PCOND(med_locator != NULL)
    NANO_PCOND(
        NANO_XRCE_Udpv4AgentTransport_valid_locator_medium(med_locator))
    NANO_PCOND(msg != NULL)

    NANO_CHECK_RC(
        NANO_OSAPI_Udpv4Socket_send(
            &self->client_socket, med_locator->address, med_locator->port, msg),
        NANO_LOG_ERROR("FAILED to send message on socket",
            NANO_LOG_PTR("transport",self)
            NANO_LOG_MBUF("msg",msg)));
    
    NANO_LOG_TRACE("UDP message sent", 
        NANO_LOG_LOCATOR_MEDIUM("locator",med_locator)
        NANO_LOG_MSGHDR("hdr",
            *((NANO_XRCE_MessageHeader*)NANO_MessageBuffer_data_ptr(msg)))
        NANO_LOG_SUBMSGHDR("submsg",
            *((NANO_XRCE_SubmessageHeader*)NANO_XRCE_InlineHeaderBuffer_submsgid_ptr(msg)))
        NANO_LOG_MBUF("mbuf",msg))

    rc = NANO_RETCODE_OK;
    
done:
    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}


NANO_RetCode
NANO_XRCE_Udpv4AgentTransport_send_direct(
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_TransportLocator *const locator,
    NANO_MessageBuffer *const msg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)transport;
    const NANO_XRCE_TransportLocatorMedium *med_address = NULL;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(locator != NULL)
    NANO_PCOND(msg != NULL)

    med_address = NANO_XRCE_TransportLocatorMedium_narrow(locator);
    if (med_address == NULL)
    {
        NANO_LOG_ERROR("INVALID send direct locator for UDPv4 transport",
            NANO_LOG_PTR("transport",self)
            NANO_LOG_LOCATOR("locator",locator))
        goto done;
    }

    NANO_CHECK_RC(
        NANO_OSAPI_Udpv4Socket_send(
            &self->client_socket, med_address->address, med_address->port, msg),
        NANO_LOG_ERROR("FAILED to send message on socket",
            NANO_LOG_PTR("transport",self)
            NANO_LOG_LOCATOR_MEDIUM("locator",med_address)
            NANO_LOG_MBUF("msg",msg)));

    rc = NANO_RETCODE_OK;
    
done:
    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

void
NANO_XRCE_Udpv4AgentTransport_return(
    NANO_XRCE_AgentTransport *const transport,
    NANO_MessageBuffer *const msg)
{
    NANO_XRCE_Udpv4AgentTransport *const self =
        (NANO_XRCE_Udpv4AgentTransport*)transport;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(msg != NULL)
    
    NANO_XRCE_Udpv4AgentTransport_release_message(self, msg);
    
    NANO_LOG_FN_EXIT
}
