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
#include "AgentDaemon.h"

#define LOG_LEVEL           LEVEL_WARNING

int main(int argc, char const *argv[])
{
    int rc = -1;
    NANO_bool daemon_initd = NANO_BOOL_FALSE,
              daemon_enabled = NANO_BOOL_FALSE;
    NANO_AgentDaemon *daemon = NULL;

#ifdef LOG_LEVEL
    NANO_LOG_SET_LEVEL(LOG_LEVEL)
#endif

    NANO_LOG_INFO_MSG("initializing XRCE daemon...")

    daemon = NANO_AgentDaemon_get_instance();

    NANO_CHECK_RC(
        NANO_AgentDaemon_initialize(daemon, argc, argv),
        NANO_LOG_ERROR_MSG("failed to initialize agent daemon"));
    daemon_initd = NANO_BOOL_TRUE;
    
    NANO_LOG_INFO_MSG("XRCE daemon started.")

    NANO_AgentDaemon_wait_for_exit(daemon);

    rc = 0;

done:

    NANO_LOG_INFO_MSG("exiting...")

    if (daemon_initd)
    {
        NANO_AgentDaemon_finalize(daemon);
    }
    
    NANO_LOG_INFO("done.", NANO_LOG_RC(rc))

    return rc;
}