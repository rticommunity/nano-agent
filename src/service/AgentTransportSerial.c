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

#include "AgentTransportSerial.h"

#include "osapi/osapi_thread.h"

#include "Agent.h"

#define SERIAL_LOG          NANO_LOG_TRACE
#define SERIAL_LOG_MSG      NANO_LOG_TRACE_MSG

/******************************************************************************
 *                          SerialAgentTransport
 ******************************************************************************/

#define NANO_XRCE_SerialAgentTransport_valid_locator(l_) \
    ((l_)->address[0] != 0)


NANO_PRIVATE
NANO_MessageBuffer*
NANO_XRCE_SerialAgentTransport_allocate_message(
    NANO_XRCE_SerialAgentTransport *const self)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_MessageBuffer *res = NULL;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    NANO_CHECK_RC(
        NANO_MessageBufferPool_allocate(&self->msg_pool, &res),
        NANO_LOG_ERROR("FAILED to allocate recv message",
            NANO_LOG_PTR("transport",self)));
    
done:
    
    NANO_LOG_FN_EXIT_PTR(res)
    return res;
}


NANO_PRIVATE
void
NANO_XRCE_SerialAgentTransport_release_message(
    NANO_XRCE_SerialAgentTransport *const self,
    NANO_MessageBuffer *const msg)
{
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(msg != NULL)

    NANO_MessageBufferPool_release(&self->msg_pool, msg);

    NANO_LOG_FN_EXIT
}

void
NANO_XRCE_SerialAgentTransport_RecvThread_impl(
    NANO_XRCE_SerialAgentTransport_RecvThread *const self)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_TransportLocatorSmall src_addr =
            NANO_XRCE_TRANSPORTLOCATORSMALL_INITIALIZER;
    NANO_XRCE_TransportLocator src =
            NANO_XRCE_TRANSPORTLOCATOR_INITIALIZER;
    NANO_MessageBuffer *msg = NULL;
    NANO_usize msg_len_max = 0;
    NANO_u16 msg_len = 0;
    NANO_bool retained = NANO_BOOL_FALSE;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    while (self->agent_transport->active)
    {
        if (msg == NULL)
        {
            msg = NANO_XRCE_SerialAgentTransport_allocate_message(
                        self->agent_transport);
            if (msg == NULL)
            {
                NANO_LOG_ERROR_MSG("FAILED to allocate receive message");
                goto done;
            }
            msg_len_max = NANO_MessageBuffer_data_len(msg);
        }

        NANO_OSAPI_Memory_zero(&src_addr, sizeof(src_addr));


        rc = NANO_XRCE_SerialClientTransport_receive_frame(
                &self->transport,
                msg,
                self->agent_transport->props.recv_timeout,
                &msg_len,
                src_addr.address);

        if (!self->agent_transport->active)
        {
            continue;
        }
        else if (NANO_RETCODE_OK == rc)
        {
            if ((msg_len >= NANO_XRCE_MESSAGE_SERIALIZED_SIZE_MIN))
            {
                SERIAL_LOG("possible XRCE(Serial) data RECEIVED",
                        NANO_LOG_LOCATOR_SMALL("src",&src_addr)
                        NANO_LOG_USIZE("size",msg_len))

                NANO_XRCE_TransportLocatorSmall_to_locator(&src_addr,&src);

                NANO_XRCE_AgentTransportListener_on_message_received(
                    &self->agent_transport->base.listener,
                    &self->agent_transport->base,
                    &src,
                    msg,
                    msg_len,
                    &retained);
            }
            else if (msg_len > 0)
            {
                SERIAL_LOG("Serial data IGNORED",
                    NANO_LOG_LOCATOR_SMALL("src",&src_addr)
                    NANO_LOG_USIZE("size",msg_len))
            }
        }
        else if (NANO_RETCODE_TIMEOUT == rc)
        {
            SERIAL_LOG("serial::recv TIMED OUT",
                NANO_LOG_TS_MS("timeout",
                    self->agent_transport->props.recv_timeout))
        }
        else
        {
            NANO_LOG_WARNING("serial::recv failed",NANO_LOG_RC(rc))
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
            RTIOsapiSemaphore_give(self->agent_transport->sem_thread_exit))
    {
        NANO_LOG_ERROR_MSG("FAILED to give exit semaphore")
    }
    NANO_LOG_FN_EXIT_RC(rc)
}


void*
NANO_XRCE_SerialAgentTransport_recv_thread(void* thread_param)
{
    NANO_XRCE_SerialAgentTransport_RecvThread *const self =
        (NANO_XRCE_SerialAgentTransport_RecvThread*)thread_param;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    NANO_XRCE_SerialAgentTransport_RecvThread_impl(self);
    
    NANO_LOG_FN_EXIT
    return NULL;
}


NANO_RetCode
NANO_XRCE_SerialAgentTransport_initialize(
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_AgentTransportListener *const listener,
    const NANO_XRCE_AgentTransportProperties *const properties,
    const NANO_XRCE_AgentTransportImplProperties *const impl_properties)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SerialAgentTransport *const self =
        (NANO_XRCE_SerialAgentTransport*)transport;
    const NANO_XRCE_SerialAgentTransportProperties *const props =
        (NANO_XRCE_SerialAgentTransportProperties*)properties;
    const NANO_XRCE_TransportLocatorSmall *sm_address = NULL;
    struct REDAFastBufferPoolProperty pool_props =
            REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    
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
            &self->msg_pool, props->base.mtu_max),
        NANO_LOG_ERROR("FAILED to initialize receive pool",
            NANO_LOG_USIZE("msg_size",props->base.mtu_max)));

    self->recv_threads_pool = 
        REDAFastBufferPool_newForStructure(
            NANO_XRCE_SerialAgentTransport_RecvThread, &pool_props);
    if (self->recv_threads_pool == NULL)
    {
        goto done;
    }

    sm_address = 
        NANO_XRCE_TransportLocatorSmall_narrow(&props->base.bind_address);
    if (sm_address == NULL)
    {
        goto done;
    }
    self->props = *props;

    self->mutex = 
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_MUTEX, NULL);
    if (self->mutex == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to create mutex semaphore")
        goto done;
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
        /* TODO dispose things */
    }

    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

void
NANO_XRCE_SerialAgentTransport_finalize(
    NANO_XRCE_AgentTransport *const transport)
{
    NANO_XRCE_SerialAgentTransport *const self =
        (NANO_XRCE_SerialAgentTransport*)transport;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(self->sem_thread_exit != NULL)

    RTIOsapiSemaphore_delete(self->mutex);
    RTIOsapiSemaphore_delete(self->sem_thread_exit);
    self->sem_thread_exit = NULL;
    NANO_MessageBufferPool_finalize(&self->msg_pool);
    REDAFastBufferPool_delete(self->recv_threads_pool);

    NANO_LOG_FN_EXIT
}


NANO_PRIVATE
void
NANO_XRCE_SerialAgentTransport_on_message_received(
    NANO_XRCE_ClientTransportListener *const listener,
    NANO_XRCE_ClientTransport *const transport,
    NANO_MessageBuffer *const msg,
    NANO_usize msg_len,
    NANO_bool *const retained)
{

    NANO_XRCE_SerialAgentTransport_RecvThread *self = NULL;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(self);
    UNUSED_ARG(msg);
    UNUSED_ARG(transport);
    UNUSED_ARG(listener);
    UNUSED_ARG(msg_len);
    UNUSED_ARG(retained);

    self = (NANO_XRCE_SerialAgentTransport_RecvThread*)listener->user_data;
    *retained = NANO_BOOL_FALSE;


    NANO_LOG_FN_EXIT
    return;
}

NANO_PRIVATE
void
NANO_XRCE_SerialAgentTransport_on_message_sent(
    NANO_XRCE_ClientTransportListener *const listener,
    NANO_XRCE_ClientTransport *const transport,
    NANO_MessageBuffer *const msg,
    const NANO_RetCode rc_send)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SerialAgentTransport_RecvThread *const self =
        (NANO_XRCE_SerialAgentTransport_RecvThread*)listener->user_data;
    
    NANO_LOG_FN_ENTRY

    UNUSED_ARG(self);
    UNUSED_ARG(rc_send);
    UNUSED_ARG(msg);
    UNUSED_ARG(transport);

    NANO_PCOND(self != NULL)

    rc = NANO_RETCODE_OK;

    NANO_LOG_FN_EXIT_RC(rc)
    return;
}

#if NANO_FEAT_RELIABILITY && \
    NANO_FEAT_RELIABLESTREAM_RECVQUEUE
NANO_MessageBuffer *
NANO_XRCE_SerialAgentTransport_reclaim_message(
    NANO_XRCE_ClientTransportListener *const listener,
    NANO_XRCE_ClientTransport *const transport)
{
    NANO_XRCE_SerialAgentTransport_RecvThread *self = NULL;

    NANO_LOG_FN_ENTRY

    UNUSED_ARG(transport);

    NANO_PCOND(listener != NULL)

    self = (NANO_XRCE_SerialAgentTransport_RecvThread *)listener->user_data;
    UNUSED_ARG(self);
    NANO_PCOND(self != NULL)

    NANO_LOG_FN_EXIT
    return NULL;
}
#endif /* NANO_FEAT_RELIABILITY && \
            NANO_FEAT_RELIABLESTREAM_RECVQUEUE */


NANO_XRCE_SerialAgentTransport_RecvThread*
NANO_XRCE_SerialAgentTransport_create_recv_thread(
    NANO_XRCE_SerialAgentTransport *const self)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SerialAgentTransport_RecvThread *res = NULL;
    const NANO_XRCE_SerialAgentTransport_RecvThread def_res = 
            NANO_XRCE_SERIALAGENTTRANSPORT_RECVTHREAD_INITIALIZER;
    NANO_XRCE_SerialClientTransportProperties client_props =
        NANO_XRCE_SERIALCLIENTTRANSPORTPROPERTIES_INITIALIZER;
    NANO_XRCE_ClientTransportListener client_listener =
        NANO_XRCE_CLIENTTRANSPORTLISTENER_INITIALIZER;
    char *rti_ptr = NULL;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    res = (NANO_XRCE_SerialAgentTransport_RecvThread*)
            REDAFastBufferPool_getBuffer(self->recv_threads_pool);
    if (res == NULL)
    {
        goto done;
    }
    *res = def_res;

    client_listener.user_data = res;
    client_listener.on_message_received =
        NANO_XRCE_SerialAgentTransport_on_message_received;
    client_listener.on_message_sent =
        NANO_XRCE_SerialAgentTransport_on_message_sent;

#if NANO_FEAT_RELIABILITY && \
    NANO_FEAT_RELIABLESTREAM_RECVQUEUE
    client_listener.reclaim_message =
        NANO_XRCE_SerialAgentTransport_reclaim_message;
#endif /* NANO_FEAT_RELIABILITY && \
            NANO_FEAT_RELIABLESTREAM_RECVQUEUE */

    client_props.base.mtu = self->props.base.mtu_max;
    client_props.connection = self->props.connection;
    client_props.bind_address = self->props.base.bind_address.value.small;

    /* This buffer is not actually used by the instance of
       SerialClientTransport, but we need to pass it in to keep the
       initialize() implementation happy. The buffer will need to be cached
       before finalizing the transport, and manually released afterwards */
    RTIOsapiHeap_allocateBufferAligned(
        &rti_ptr,
        client_props.base.mtu,
        RTI_OSAPI_ALIGNMENT_DEFAULT);
    client_props.recv_buffer = (NANO_u8*)rti_ptr;
    if (client_props.recv_buffer == NULL)
    {
        goto done;
    }
    client_props.recv_buffer_size = client_props.base.mtu;

    NANO_CHECK_RC(
        NANO_XRCE_SerialClientTransport_initialize(
            &res->transport.base, &client_listener, &client_props.base),
        NANO_LOG_ERROR_MSG("FAILED to initialize client serial transport"))
    
    res->agent_transport = self;

    res->recv_thread = 
        RTIOsapiThread_new(
            NANO_XRCE_SERIALAGENTTRANSPORT_RECV_THREAD_NAME,
            RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
            RTI_OSAPI_THREAD_OPTION_DEFAULT,
            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
            NULL,
            NANO_XRCE_SerialAgentTransport_recv_thread,
            res);
    if (res->recv_thread == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to create serial RECV thread")
        goto done;
    }

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        /* TODO dealloc stuff */
    }
    
    NANO_LOG_FN_EXIT_RC(rc)
    return res;
}

NANO_RetCode
NANO_XRCE_SerialAgentTransport_listen(
    NANO_XRCE_AgentTransport *const transport)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SerialAgentTransport *const self =
        (NANO_XRCE_SerialAgentTransport*)transport;
    NANO_XRCE_SerialAgentTransport_RecvThread *recv_thread = NULL;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    /* Start receive thread */
    self->active = NANO_BOOL_TRUE;

    recv_thread = NANO_XRCE_SerialAgentTransport_create_recv_thread(self);
    if (recv_thread == NULL)
    {
        goto done;
    }

    REDAInlineList_addNodeToBackEA(&self->recv_threads, &recv_thread->node);

    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        
    }

    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

NANO_RetCode
NANO_XRCE_SerialAgentTransport_close(
    NANO_XRCE_AgentTransport *const transport)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SerialAgentTransport *const self =
        (NANO_XRCE_SerialAgentTransport*)transport;
    NANO_XRCE_SerialAgentTransport_RecvThread *recv_thread = NULL;
    NANO_u8 *recv_buffer = NULL;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)

    /* Stop receive threads */
    self->active = NANO_BOOL_FALSE;

    recv_thread = 
        (NANO_XRCE_SerialAgentTransport_RecvThread *)
            REDAInlineList_getFirst(&self->recv_threads);
    while (recv_thread != NULL)
    {
        RTIOsapiSemaphore_take(self->sem_thread_exit, RTI_NTP_TIME_INFINITE);
        REDAInlineList_removeNodeEA(&self->recv_threads, &recv_thread->node);
        recv_buffer =
            NANO_MessageBuffer_data_ptr(&recv_thread->transport.recv_msg);
        NANO_XRCE_SerialClientTransport_finalize(&recv_thread->transport.base);
        RTIOsapiHeap_freeBuffer(recv_buffer);
        RTIOsapiThread_delete(recv_thread->recv_thread);
        REDAFastBufferPool_returnBuffer(self->recv_threads_pool, recv_thread);
        recv_thread = 
            (NANO_XRCE_SerialAgentTransport_RecvThread *)
                REDAInlineList_getFirst(&self->recv_threads);
    }

    rc = NANO_RETCODE_OK;
    
    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

NANO_RetCode
NANO_XRCE_SerialAgentTransport_send_to(
    NANO_XRCE_AgentTransport *const transport,
    NANO_XRCE_AgentTransportBindEntry *const client_entry,
    NANO_MessageBuffer *const msg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SerialAgentTransport *const self =
        (NANO_XRCE_SerialAgentTransport*)transport;
    NANO_XRCE_TransportLocatorSmall *sm_locator = NULL;
    NANO_XRCE_SerialAgentTransport_RecvThread *recv_thread = NULL;

    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(client_entry != NULL)
    NANO_PCOND(client_entry->transport == transport)
    NANO_PCOND(NANO_XRCE_ClientKey_is_valid(&client_entry->key))

    sm_locator =
        NANO_XRCE_TransportLocatorSmall_narrow(&client_entry->locator);
    NANO_PCOND(sm_locator != NULL)
    NANO_PCOND(
        NANO_XRCE_SerialAgentTransport_valid_locator(sm_locator))
    NANO_PCOND(msg != NULL)

    recv_thread = 
        (NANO_XRCE_SerialAgentTransport_RecvThread*)
            REDAInlineList_getFirst(&self->recv_threads);
    // NANO_PCOND(recv_thread != NULL)
    if (recv_thread == NULL)
    {
        NANO_LOG_ERROR_MSG("serial transport already finalized")
        rc = NANO_RETCODE_OK;
        goto done;
    }

    recv_thread->transport.agent_address = *sm_locator;

    // RTIOsapiSemaphore_take(self->mutex, RTI_NTP_TIME_INFINITE);

    NANO_CHECK_RC(
        NANO_XRCE_SerialClientTransport_send(
            &recv_thread->transport,
            &self->props.base.bind_address.value.small,
            &client_entry->locator.value.small,
            msg),
        NANO_LOG_ERROR_MSG("FAILED to send serial message"));

    rc = NANO_RETCODE_OK;
    
done:
    // RTIOsapiSemaphore_give(self->mutex);

    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}


NANO_RetCode
NANO_XRCE_SerialAgentTransport_send_direct(
    NANO_XRCE_AgentTransport *const transport,
    const NANO_XRCE_TransportLocator *const locator,
    NANO_MessageBuffer *const msg)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_SerialAgentTransport *const self =
        (NANO_XRCE_SerialAgentTransport*)transport;
    const NANO_XRCE_TransportLocatorSmall *sm_address = NULL;
    NANO_XRCE_SerialAgentTransport_RecvThread *recv_thread = NULL;
    
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(locator != NULL)
    NANO_PCOND(msg != NULL)


    sm_address = NANO_XRCE_TransportLocatorSmall_narrow(locator);
    if (sm_address == NULL)
    {
        NANO_LOG_ERROR("INVALID send direct locator for serial transport",
            NANO_LOG_PTR("transport",self)
            NANO_LOG_LOCATOR("locator",locator))
        goto done;
    }

    recv_thread = 
        (NANO_XRCE_SerialAgentTransport_RecvThread*)
            REDAInlineList_getFirst(&self->recv_threads);
    NANO_PCOND(recv_thread != NULL)
    
    recv_thread->transport.agent_address = *sm_address;
    
    // RTIOsapiSemaphore_take(self->mutex, RTI_NTP_TIME_INFINITE);
    // in_ea = NANO_BOOL_TRUE;

    NANO_CHECK_RC(
        NANO_XRCE_SerialClientTransport_send(
            &recv_thread->transport,
            &self->props.base.bind_address.value.small,
            &locator->value.small,
            msg),
        NANO_LOG_ERROR_MSG("FAILED to send serial message"));

    rc = NANO_RETCODE_OK;
    
done:
    // if (in_ea)
    // {
    //     RTIOsapiSemaphore_give(self->mutex);
    // }

    NANO_LOG_FN_EXIT_RC(rc)
    
    return rc;
}

void
NANO_XRCE_SerialAgentTransport_return(
    NANO_XRCE_AgentTransport *const transport,
    NANO_MessageBuffer *const msg)
{
    NANO_XRCE_SerialAgentTransport *const self =
        (NANO_XRCE_SerialAgentTransport*)transport;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(msg != NULL)
    
    NANO_XRCE_SerialAgentTransport_release_message(self, msg);
    
    NANO_LOG_FN_EXIT
}
