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

#ifndef dds_agent_log_h
#define dds_agent_log_h

#include "log/log_common.h"

#define D2S2_CURRENT_SUBMODULE              1

#define D2S2_SUBMODULE_MASK_ALL             (0xFFFF)

extern RTILogBitmap D2S2Log_g_submoduleMask;

extern RTILogBitmap D2S2Log_g_instrumentationMask;

extern void D2S2Log_setBitmaps(RTILogBitmap submoduleMask,
                            RTILogBitmap instrumentationMask);

extern void D2S2Log_getBitmaps(RTILogBitmap *submoduleMask,
                            RTILogBitmap *instrumentationMask);

void D2S2Log_setVerbosity(const RTILogBitmap verbosity);

#define D2S2_LOG_VERBOSITY_ERROR \
    (RTI_LOG_BIT_EXCEPTION)

#define D2S2_LOG_VERBOSITY_WARNING \
    (RTI_LOG_BIT_EXCEPTION |\
        RTI_LOG_BIT_WARN)

#define D2S2_LOG_VERBOSITY_INFO \
    (RTI_LOG_BIT_EXCEPTION |\
        RTI_LOG_BIT_WARN |\
            RTI_LOG_BIT_LOCAL)

#define D2S2_LOG_VERBOSITY_DEBUG \
    (RTI_LOG_BIT_EXCEPTION |\
        RTI_LOG_BIT_WARN |\
            RTI_LOG_BIT_LOCAL)

#define D2S2_LOG_VERBOSITY_TRACE \
    (RTI_LOG_BIT_EXCEPTION |\
        RTI_LOG_BIT_WARN |\
            RTI_LOG_BIT_LOCAL |\
                RTI_LOG_BIT_PERIODIC)

#define D2S2_LOG_VERBOSITY_TRACE_FN \
    (RTI_LOG_BIT_EXCEPTION |\
        RTI_LOG_BIT_WARN |\
            RTI_LOG_BIT_LOCAL |\
                RTI_LOG_BIT_PERIODIC |\
                    RTI_LOG_BIT_ACTIVITY)

#if DDS_AGENT_LOGAPI == DDS_AGENT_LOGAPI_CONNEXT_600
#define D2S2Log_exception(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_EXCEPTION) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILog_printContextAndMsg(\
            RTI_LOG_BIT_EXCEPTION, method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_warn(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_WARN) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILog_printContextAndMsg(\
            RTI_LOG_BIT_WARN, method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_local(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_LOCAL) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILog_printContextAndMsg(\
            RTI_LOG_BIT_LOCAL, method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_activity(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_ACTIVITY) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILog_printContextAndMsg(\
            RTI_LOG_BIT_ACTIVITY, method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_periodic(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_PERIODIC) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILog_printContextAndMsg(\
            RTI_LOG_BIT_PERIODIC, method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_freeForm(mask) \
    if ((D2S2Log_g_instrumentationMask & (mask)) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
        RTILog_debug

#elif DDS_AGENT_LOGAPI == DDS_AGENT_LOGAPI_CONNEXT_610
#define D2S2Log_exception(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_EXCEPTION) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILogMessage_printWithParams( \
               RTI_LOG_PRINT_FORMAT_MASK_ALL, \
               RTI_LOG_BIT_EXCEPTION, \
               0, \
               __FILE__, \
               __LINE__, \
               method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_warn(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_WARN) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILogMessage_printWithParams( \
               RTI_LOG_PRINT_FORMAT_MASK_ALL, \
               RTI_LOG_BIT_WARN, \
               0, \
               __FILE__, \
               __LINE__, \
               method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_local(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_LOCAL) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILogMessage_printWithParams( \
               RTI_LOG_PRINT_FORMAT_MASK_ALL, \
               RTI_LOG_BIT_LOCAL, \
               0, \
               __FILE__, \
               __LINE__, \
               method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_activity(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_ACTIVITY) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILogMessage_printWithParams( \
               RTI_LOG_PRINT_FORMAT_MASK_ALL, \
               RTI_LOG_BIT_ACTIVITY, \
               0, \
               __FILE__, \
               __LINE__, \
               method_, fmt_, __VA_ARGS__);\
    }

#define D2S2Log_periodic(method_, fmt_, ...) \
    if ((D2S2Log_g_instrumentationMask & RTI_LOG_BIT_PERIODIC) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
    {\
        RTILogMessage_printWithParams( \
               RTI_LOG_PRINT_FORMAT_MASK_ALL, \
               RTI_LOG_BIT_PERIODIC, \
               0, \
               __FILE__, \
               __LINE__, \
               method_, fmt_, __VA_ARGS__);\
    }

#include <stdio.h>

#define D2S2Log_freeForm(mask) \
    if ((D2S2Log_g_instrumentationMask & (mask)) && \
        (D2S2Log_g_submoduleMask & D2S2_CURRENT_SUBMODULE)) \
        printf

#else
#define D2S2Log_exception(method_, fmt_, ...)
#define D2S2Log_warn(method_, fmt_, ...)
#define D2S2Log_local(method_, fmt_, ...)
#define D2S2Log_activity(method_, fmt_, ...)
#define D2S2Log_periodic(method_, fmt_, ...)
#endif

#define D2S2_LOG_MODULE_NAME            "D2S2"
#define D2S2_LOG_SUBMODULE_NAME_DB      "DB"
#define D2S2_LOG_SUBMODULE_NAME_AGENT   "Agent"

/******************************************************************************
 * Log Helpers
 ******************************************************************************/

#define D2S2Log_fn_entry() \
    D2S2Log_activity(method_name, &RTI_LOG_ANY_s, " >> call");

#define D2S2Log_fn_exit() \
    D2S2Log_activity(method_name, &RTI_LOG_ANY_s, " << return");

#define D2S2Log_METHOD_NAME(fn_) \
    const char *const method_name = #fn_;

/******************************************************************************
 * Log Messages
 ******************************************************************************/

#define D2S2_LOG_MSG_ALLOC_BUFFER_FAILED \
    "FAILED to allocate buffer from pool: "

/******************************************************************************
 * Event Queue
 ******************************************************************************/

#define D2S2_LOG_MSG_EVENT_QUEUE_INIT_TIMED_EVENT_FAILED \
    "FAILED to initialize timed event"

#define D2S2_LOG_MSG_EVENT_QUEUE_ATTACH_TIMED_EVENT_FAILED \
    "FAILED to attach timed event"

/******************************************************************************
 * Database
 ******************************************************************************/
#define D2S2_LOG_MSG_DB_LOCK_SESSIONS_FAILED \
    "FAILED to lock sessions table"

#define D2S2_LOG_MSG_DB_UNLOCK_SESSIONS_FAILED \
    "FAILED to unlock sessions table"

#define D2S2_LOG_MSG_DB_LOCK_RESOURCES_FAILED \
    "FAILED to lock resource table"

#define D2S2_LOG_MSG_DB_UNLOCK_RESOURCES_FAILED \
    "FAILED to unlock resource table"

#define D2S2_LOG_MSG_DB_LOOKUP_RESOURCE_FAILED \
    "FAILED to lookup resource record in database"

#define D2S2_LOG_MSG_DB_INSERT_RESOURCE_FAILED \
    "FAILED to insert resource record in database"

#define D2S2_LOG_MSG_DB_RELEASE_RESOURCE_FAILED \
    "FAILED to release resource record to database"

#define D2S2_LOG_MSG_DB_FIND_RESOURCE_FAILED \
    "FAILED to find resource record in database"

#define D2S2_LOG_MSG_DB_ITERATE_RESOURCES_FAILED \
    "FAILED to iterate resource records in database"

#define D2S2_LOG_MSG_DB_FINISH_ITERATE_RESOURCES_FAILED \
    "FAILED to finish iterating resources in database"

#define D2S2_LOG_MSG_DB_DELETE_RESOURCE_FAILED \
    "FAILED to delete resource record from database"


#define D2S2_LOG_MSG_DB_LOOKUP_SESSION_FAILED \
    "FAILED to lookup resource record in database"

#define D2S2_LOG_MSG_DB_INSERT_SESSION_FAILED \
    "FAILED to insert session record in database"

#define D2S2_LOG_MSG_DB_RELEASE_SESSION_FAILED \
    "FAILED to release session record to database"

#define D2S2_LOG_MSG_DB_DELETE_SESSION_FAILED \
    "FAILED to delete session record from database"

/******************************************************************************
 * Agent Interface
 ******************************************************************************/
#define D2S2_LOG_MSG_INTF_ON_SESSION_OPENED_FAILED \
    "AgentServerInterface::on_session_opened() FAILED"

#define D2S2_LOG_MSG_INTF_ON_SESSION_RESET_FAILED \
    "AgentServerInterface::on_session_reset() FAILED"

/******************************************************************************
 * Agent
 ******************************************************************************/
#define D2S2_LOG_MSG_AGENT_MARK_SESSION_ACTIVE_FAILED \
    "FAILED to mark session as active"

#define D2S2_LOG_MSG_AGENT_CREATE_TIMED_EVENT_FAILED \
    "FAILED to create timed event"

#define D2S2_LOG_MSG_AGENT_CREATE_CONDITION_EVENT_FAILED \
    "FAILED to create condition event"

#define D2S2_LOG_MSG_AGENT_ATTACH_CONDITION_EVENT_FAILED \
    "FAILED to attach condition event"

#define D2S2_LOG_MSG_AGENT_CREATE_RESOURCE_FAILED \
    "FAILED to create resource"

#define D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_REPRESENTATION_FORMAT \
    "INVALID resource representation format"

#define D2S2_LOG_MSG_AGENT_INVALID_RESOURCE_KIND \
    "INVALID resource kind"

#define D2S2_LOG_MSG_AGENT_UNEXPECTED_PARENT_RESOURCE \
    "resource does NOT require a parent"

#define D2S2_LOG_MSG_AGENT_MISSING_PARENT_RESOURCE \
    "resource REQUIRES a parent"

#define D2S2_LOG_MSG_AGENT_ATTACHED_RESOURCE_NOT_FOUND \
    "attached resource NOT FOUND in session"

#define D2S2_LOG_MSG_AGENT_RESOURCE_ALREADY_ATTACHED \
    "resource already attached to session"

#define D2S2_LOG_MSG_AGENT_REUSE_REPLACE_DISABLED \
    "reuse and replace flags DISABLED"

#define D2S2_LOG_MSG_AGENT_RESOURCE_NOT_FOUND \
    "DDS resource NOT FOUND"

#define D2S2_LOG_MSG_AGENT_CONVERT_RESOURCE_REPRESENTATION_FAILED \
    "FAILED to convert resource representation"

#define D2S2_LOG_MSG_AGENT_INCOMPATIBLE_RESOURCE_REPRESENTATION \
    "INCOMPATIBLE resource representation"

#define D2S2_LOG_MSG_AGENT_CREATE_NATIVE_RESOURCE_FAILED \
    "FAILED to create native resource"

#define D2S2_LOG_MSG_AGENT_LOOKUP_NATIVE_RESOURCE_FAILED \
    "FAILED to lookup native resource"

#define D2S2_LOG_MSG_AGENT_INITIALIZE_RESOURCE_FAILED \
    "FAILED to initialize resource record"

#define D2S2_LOG_MSG_AGENT_INITIALIZE_READER_STATE_FAILED \
    "FAILED to initialize reader state"

#define D2S2_LOG_MSG_AGENT_INITIALIZE_WRITER_STATE_FAILED \
    "FAILED to initialize writer state"

#define D2S2_LOG_MSG_AGENT_GENERATE_RESOURCE_ID_FAILED \
    "FAILED to generate attached resource id"

#define D2S2_LOG_MSG_AGENT_COPY_FAILED \
    "FAILED to copy"



#endif /* dds_agent_log_h */