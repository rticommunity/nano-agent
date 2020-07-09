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

NANO_RetCode
NANO_MessageBufferPool_initialize(
    NANO_MessageBufferPool *const self,
    const NANO_usize data_size)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    struct REDAFastBufferPoolProperty pool_props =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    NANO_usize buffer_size = 0;
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(self->pool == NULL)
    NANO_PCOND(data_size > 0)

    if (data_size > NANO_MESSAGEBUFFER_DATA_LEN_MAX)
    {
        NANO_LOG_ERROR("buffer size TOO LARGE",
            NANO_LOG_USIZE("requested",buffer_size)
            NANO_LOG_USIZE("max", NANO_MESSAGEBUFFER_DATA_LEN_MAX))
        rc = NANO_RETCODE_INVALID_ARGS;
        goto done;
    }
    
    buffer_size = NANO_MESSAGEBUFFER_INLINE_SIZE_BYTES(data_size);
    
    pool_props.multiThreadedAccess = 1;
    self->pool = 
        REDAFastBufferPool_new(
            buffer_size, NANO_MessageBufferData_alignment, &pool_props);
    if (self->pool == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to create buffer pool")
        goto done;
    }

    rc = NANO_RETCODE_OK;
    
done:
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_MessageBufferPool_finalize(NANO_MessageBufferPool *const self)
{
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(self->pool != NULL)
    
    REDAFastBufferPool_delete(self->pool);
    self->pool = NULL;
    
    NANO_LOG_FN_EXIT
}

NANO_RetCode
NANO_MessageBufferPool_allocate(
    NANO_MessageBufferPool *const self,
    NANO_MessageBuffer **const msg_out)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_MessageBuffer *msg = NULL,
                       def_msg = NANO_MESSAGEBUFFER_INITIALIZER;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    msg = (NANO_MessageBuffer*) REDAFastBufferPool_getBuffer(self->pool);
    if (msg == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to allocate message buffer")
        rc = NANO_RETCODE_OUT_OF_RESOURCES;
        goto done;
    }
    
    *msg = def_msg;
    NANO_MessageBuffer_flags_set_inline(msg);
    NANO_MessageBuffer_set_data_len(
        msg, NANO_MessageBufferPool_data_size(self));
    
    NANO_LOG_TRACE("ALLOCD message buffer", 
        NANO_LOG_USIZE("data_size",NANO_MessageBufferPool_data_size(self)))

    *msg_out = msg;
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_MessageBufferPool_release(
    NANO_MessageBufferPool *const self,
    NANO_MessageBuffer *const msg)
{
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(self->pool != NULL)
    NANO_PCOND(msg != NULL)

    
    REDAFastBufferPool_returnBuffer(self->pool, msg);
    
    NANO_LOG_FN_EXIT
}
