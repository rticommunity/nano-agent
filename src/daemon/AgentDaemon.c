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

#include "AgentDaemon.h"
#include "nano/nano_agent_http.h"

NANO_AgentDaemon NANO_AgentDaemon_g_self = NANO_AGENTDAEMON_INITIALIZER;

void NANO_AgentDaemon_exit()
{
    if (NANO_AgentDaemon_g_self.sem_exit != NULL)
    {
        RTIOsapiSemaphore_give(
            NANO_AgentDaemon_g_self.sem_exit);
    }
}

#if NANO_PLATFORM_IS_POSIX

#include <signal.h>

/* Signal handler functions */

typedef void(* NANO_AgentDaemon_SigHandlerFn)();

void NANO_AgentDaemon_exit_on_signal()
{
    NANO_AgentDaemon_exit();
}

#if _POSIX_C_SOURCE
NANO_RetCode
NANO_AgentDaemon_set_exit_on_signal(int sig)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    struct sigaction new_action;
    struct sigaction old_action;
    NANO_AgentDaemon_SigHandlerFn handler = NANO_AgentDaemon_exit_on_signal;
    
    NANO_LOG_FN_ENTRY
    
    memset(&new_action, 0, sizeof (struct sigaction));
    new_action.sa_handler = handler;

    /* Temporarily block all other signals during handler execution */
    sigfillset(&new_action.sa_mask);

    if (sigaction(sig, NULL, &old_action) != 0)
    {
        NANO_LOG_ERROR_MSG("sigaction(old) FAILED")
        goto done;
    }

    /* Honor inherited SIG_IGN that's set by some shell's */
    if (old_action.sa_handler != SIG_IGN)
    {
        if (sigaction(sig, &new_action, NULL) != 0) {
            NANO_LOG_ERROR_MSG("sigaction(new) FAILED")
            goto done;
        }
    }
    
    rc = NANO_RETCODE_OK;
    
done:
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}
#else
NANO_RetCode
NANO_AgentDaemon_set_exit_on_signal(int sig)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_AgentDaemon_SigHandlerFn handler = NANO_AgentDaemon_exit_on_signal;
    
    NANO_LOG_FN_ENTRY
    
    signal(sig,handler);

    rc = NANO_RETCODE_OK;
    
    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}
#endif /* _XOPEN_SOURCE >= 700 */

#elif NANO_PLATFORM == NANO_PLATFORM_WINDOWS

BOOL NANO_AgentDaemon_exit_on_signal(DWORD cntrlType)
{
    NANO_AgentDaemon_exit();
    return TRUE;
}

#endif

void
NANO_AgentDaemonArgs_finalize(NANO_AgentDaemonArgs *const self);

void
NANO_AgentDaemon_print_help(const char *arg0)
{
    printf("%s - RTI Connext DDS XRCE Agent Service\n\n", arg0);
    printf("  USAGE: %s [ " NANO_AGENT_ARG_SHORT_STR(UDP) " | " NANO_AGENT_ARG_SHORT_STR(SERIAL) " " NANO_AGENT_ARG_SHORT_STR(SERIAL_DEV) " DEVICE] [options] \n\n", arg0);
    printf("  OPTIONS:\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR(AUTOATTACH) "\n\n"
           "      Automatically attach any DDS resource to any client session.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(CONFIG,"PATH") "\n\n"
           "      Load default DDS resources and configuration from an XML file.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(GENERATEID,"RESOURCE-ID") "\n\n"
           "      Print the automatically generate id for the specified resource\n"
           "      and exit.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(HEARTBEATPERIOD,"PERIOD") "\n\n"
           "      Interval in millisecond for periodic heartbeat messages.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR(HTTP) "\n\n"
           "      Enable support for making HTTP requests.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(KIND,"RESOURCE-KIND") "\n\n"
           "      The type of resource for which an id is to be generated. One of:\n\n"
           "        - domainparticipant [dp]\n"
           "        - topic [t]\n"
           "        - publisher [p]\n"
           "        - subscriber [s],\n"
           "        - datawriter [dw].\n"
           "        - datareader [dr].\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR(SERIAL) "\n\n"
           "      Enable the Agent's serial transport.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(SERIAL_DEV,"DEVICE") "\n\n"
           "      Serial device used by the Agent's serial transport.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(SERIAL_DEV_1,"DEVICE") "\n\n"
           "      Secondary serial device used by the Agent's serial transport.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(SERIAL_SPEED,"SPEED") "\n\n"
           "      Baud rate used to communicate over the serial line.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(SERIAL_SPEED_1,"SPEED") "\n\n"
           "      Baud rate used to communicate over the serial line (secondary device).\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(SERIAL_TIMEOUT,"AMOUNT") "\n\n"
           "      Maximum time in ms to wait for data on the serial line to be"
           "      received.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(TIMEOUT,"AMOUNT") "\n\n"
           "      The maximum allowed inactivity period for a client session in\n"
           "      milliseconds (default: infinite).\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR(UDP) "\n\n"
           "      Enable the Agent's UDP transport.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(UDP_PORT,"PORT") "\n\n"
           "      Listening port used by the Agent's UDP transport.\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR_P(VERBOSITY,"LEVEL") "\n\n"
           "      Verbosity of log messages printed to console:\n\n"
           "        - 0  (error)\n"
           "        - 1  (warning)\n"
           "        - 2  (info)\n"
           "        - 3  (debug)\n"
           "        - 4  (trace)\n"
           "        - 5+ (trace more)\n\n"
           "   " NANO_AGENT_ARGS_HELP_STR(HELP) "\n\n"
           "      Display this help menu.\n\n");
}

NANO_RetCode
NANO_AgentDaemon_initialize_xrce(NANO_AgentDaemon *const self)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_XRCE_Udpv4AgentTransportProperties transport_udpv4_props =
        NANO_XRCE_UDPV4AGENTTRANSPORTPROPERTIES_INITIALIZER;
    NANO_XRCE_SerialAgentTransportProperties transport_serial_props =
        NANO_XRCE_SERIALAGENTTRANSPORTPROPERTIES_INITIALIZER;
    NANO_XRCE_AgentProperties props = NANO_XRCE_AGENTPROPERTIES_INITIALIZER;
    D2S2_ExternalServicePlugin *http_plugin = NULL;

    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    props.client_session_timeout = self->args.session_timeout;
    if (self->args.heartbeat_period > 0)
    {
        NANO_Time_from_millis(
            &props.heartbeat_period, self->args.heartbeat_period);
        NANO_LOG_INFO("HEARTBEAT period",
            NANO_LOG_I32("sec",props.heartbeat_period.sec)
            NANO_LOG_U32("nanosec",props.heartbeat_period.nanosec))
    }

    self->agent = NANO_XRCE_Agent_new(&props);
    if (self->agent == NULL)
    {
        NANO_LOG_ERROR_MSG("FAILED to initialize agent")
        goto done;
    }
    NANO_LOG_INFO("XRCE agent initialized",
        NANO_LOG_PTR("agent",self->agent))
    
    if (!(self->args.xrce_udp_enable ||
            self->args.xrce_serial_enable))
    {
        NANO_LOG_ERROR_MSG("no transports enabled for Agent")
        goto done;
    }

    if (self->args.xrce_udp_enable)
    {
        if (self->args.xrce_udp_port > 0)
        {
            NANO_XRCE_TransportLocator_address_from_ipv4addr(
                &transport_udpv4_props.base.bind_address,
                0 /* any */,
                self->args.xrce_udp_port);
        }
        transport_udpv4_props.socket.timeout_ms = NANO_TIMEOUT_INFINITE;

        NANO_LOG_INFO("ENABLING udp transport",
                NANO_LOG_LOCATOR("bind_address",
                    &transport_udpv4_props.base.bind_address))
        
        NANO_CHECK_RC(
            NANO_XRCE_Agent_register_transport(
                self->agent,
                &self->transport_udpv4.base, 
                &transport_udpv4_props.base),
            NANO_LOG_ERROR_MSG("FAILED to register UDPv4 transport"))
    }

    if (self->args.xrce_serial_enable)
    {
        if (self->args.xrce_serial_dev == NULL &&
            self->args.xrce_serial_dev_1 == NULL)
        {
            NANO_LOG_ERROR_MSG("no serial device specified")
            goto done;
        }

        transport_serial_props.connection.device =
            self->args.xrce_serial_dev;
        if (self->args.xrce_serial_speed > 0)
        {
            transport_serial_props.connection.speed =
                self->args.xrce_serial_speed;
        }

        if (self->args.xrce_serial_timeout != -2)
        {
            transport_serial_props.recv_timeout =
                self->args.xrce_serial_timeout;
        }

        transport_serial_props.base.bind_address.format =
            NANO_XRCE_ADDRESS_FORMAT_SMALL;
        transport_serial_props.base.bind_address.value.small =
            self->args.xrce_serial_address;
        
        if (self->args.xrce_serial_dev != NULL)
        {    
            NANO_LOG_INFO("ENABLING primary serial transport",
                NANO_LOG_STR("device",
                    transport_serial_props.connection.device)
                NANO_LOG_TS_MS("timeout",
                    transport_serial_props.recv_timeout)
                NANO_LOG_U32("speed",
                    transport_serial_props.connection.speed))

            NANO_CHECK_RC(
                NANO_XRCE_Agent_register_transport(
                    self->agent,
                    &self->transport_serial.base, 
                    &transport_serial_props.base),
                NANO_LOG_ERROR_MSG("FAILED to register serial transport"))
        }
        

        if (self->args.xrce_serial_dev_1 != NULL)
        {
            transport_serial_props.connection.device =
                self->args.xrce_serial_dev_1;
            
            if (self->args.xrce_serial_speed_1 > 0)
            {
                transport_serial_props.connection.speed =
                    self->args.xrce_serial_speed_1;
            }
            
            NANO_LOG_INFO("ENABLING secondary serial transport",
                NANO_LOG_STR("device",
                    transport_serial_props.connection.device)
                NANO_LOG_TS_MS("timeout",
                    transport_serial_props.recv_timeout)
                NANO_LOG_U32("speed",
                    transport_serial_props.connection.speed))
            
            NANO_CHECK_RC(
                NANO_XRCE_Agent_register_transport(
                    self->agent,
                    &self->transport_serial_1.base, 
                    &transport_serial_props.base),
                NANO_LOG_ERROR_MSG("FAILED to register serial transport"))
        }

        if (self->args.xrce_serial_dev_2 != NULL)
        {
            transport_serial_props.connection.device =
                self->args.xrce_serial_dev_2;
            
            if (self->args.xrce_serial_speed_2 > 0)
            {
                transport_serial_props.connection.speed =
                    self->args.xrce_serial_speed_2;
            }
            
            NANO_LOG_INFO("ENABLING tertiary serial transport",
                NANO_LOG_STR("device",
                    transport_serial_props.connection.device)
                NANO_LOG_TS_MS("timeout",
                    transport_serial_props.recv_timeout)
                NANO_LOG_U32("speed",
                    transport_serial_props.connection.speed))
            
            NANO_CHECK_RC(
                NANO_XRCE_Agent_register_transport(
                    self->agent,
                    &self->transport_serial_2.base, 
                    &transport_serial_props.base),
                NANO_LOG_ERROR_MSG("FAILED to register serial transport"))
        }
    }


    if (DDS_RETCODE_OK !=
            D2S2_Agent_register_interface(
                self->server, NANO_XRCE_Agent_as_interface(self->agent)))
    {
        goto done;
    }

    if (self->args.http_enable)
    {
        http_plugin = NANO_HttpPlugin_new();
        if (NULL == http_plugin)
        {
            goto done;
        }
        if (DDS_RETCODE_OK !=
            D2S2_Agent_register_external_service_plugin(
              self->server, http_plugin))
        {
            goto done;
        }
    }
    
    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

NANO_RetCode
NANO_AgentDaemon_initialize(
    NANO_AgentDaemon *const self,
    const int argc,
    const char **const argv)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NDDSA_AgentProperties server_props = NDDSA_AGENTPROPERTIES_INITIALIZER;
    
#if NANO_PLATFORM_IS_POSIX
    sigset_t set, oset;
#define MAX_SIGNALS     5
    int signals[MAX_SIGNALS] = { SIGTERM, SIGHUP, SIGINT, SIGABRT, SIGPIPE };
    int i = 0;
#endif
    
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    
    NANO_LOG_INFO_MSG("XRCE agent starting...")

    NANO_CHECK_RC(
        NANO_AgentDaemon_parse_args(self, argc, argv),
        NANO_LOG_ERROR_MSG("FAILED to parse command line arguments")
        NANO_AgentDaemon_print_help(argv[0]););
    
    if (self->args.show_help)
    {
        NANO_AgentDaemon_print_help(argv[0]);
        exit(0);
    }

    switch (self->args.verbosity)
    {
    case 0:
    {
        NANO_LOG_SET_LEVEL(LEVEL_ERROR)
        D2S2Log_setVerbosity(D2S2_LOG_VERBOSITY_ERROR);
        break;
    }
    case 1:
    {
        NANO_LOG_SET_LEVEL(LEVEL_WARNING)
        D2S2Log_setVerbosity(D2S2_LOG_VERBOSITY_WARNING);
        break;
    }
    case 2:
    {
        NANO_LOG_SET_LEVEL(LEVEL_INFO)
        D2S2Log_setVerbosity(D2S2_LOG_VERBOSITY_INFO);
        break;
    }
    case 3:
    {
        NANO_LOG_SET_LEVEL(LEVEL_DEBUG)
        D2S2Log_setVerbosity(D2S2_LOG_VERBOSITY_DEBUG);
        break;
    }
    case 4:
    {
        NANO_LOG_SET_LEVEL(LEVEL_TRACE)
        D2S2Log_setVerbosity(D2S2_LOG_VERBOSITY_TRACE);
        break;
    }
    default:
    {
        NANO_LOG_SET_LEVEL(LEVEL_TRACE_FN)
        D2S2Log_setVerbosity(D2S2_LOG_VERBOSITY_TRACE_FN);
        break;
    }
    }

    self->sem_exit =
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);
    if (self->sem_exit == NULL)
    {
        goto done;
    }


/* Install signal handlers */
#if NANO_PLATFORM == NANO_PLATFORM_WINDOWS
    /* is a console app, set signal handler */
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)NANO_AgentDaemon_exit_on_signal, TRUE))
    {
        NANO_LOG_ERROR_MSG("FAILED to set signal handler")
        goto done;
    }
#elif NANO_PLATFORM_IS_POSIX
#if _XOPEN_SOURCE
    /*
     * Mask all signals during startup.
     */
    if (sigfillset(&set))
    {
        NANO_LOG_ERROR_MSG("FAILED to initialize signal mask")
        goto done;
    }
    if (pthread_sigmask(SIG_SETMASK, &set, &oset)) 
    {
        NANO_LOG_ERROR_MSG("FAILED to disable signals")
        goto done;
    }
#endif /* _XOPEN_SOURCE */

    /*
     * Install signals handlers
     */
    for (i = 0; i < MAX_SIGNALS; i++)
    {
        int sig = signals[i];
        NANO_LOG_DEBUG("setting exit handler", NANO_LOG_I32("signal", sig))
        NANO_CHECK_RC(
            NANO_AgentDaemon_set_exit_on_signal(sig),
            NANO_LOG_ERROR("FAILED to set signal handler",
                NANO_LOG_I32("signal",sig)));
    }
#endif

    server_props.auto_attach_resources = self->args.auto_attach_resources;
    server_props.read_start_point = self->args.read_start;
    self->server = NDDSA_Agent_create(&server_props);
    if (self->server == NULL)
    {
        goto done;
    }

    if (self->args.gen_resource_id != NULL)
    {
        D2S2_AttachedResourceId attached_id = D2S2_ATTACHEDRESOURCEID_INVALID;
        D2S2_ResourceId resource_id = D2S2_RESOURCEID_INITIALIZER;

        resource_id.kind = D2S2_RESOURCEIDKIND_REF;
        resource_id.value.ref = self->args.gen_resource_id;

        if (DDS_RETCODE_OK !=
                D2S2_Agent_generate_attached_resource_id(
                    self->server,
                    &resource_id,
                    self->args.gen_resource_kind,
                    &attached_id))
        {
            NANO_LOG_ERROR_MSG("FAILED to generate attached resource id");
            goto done;
        }

        NANO_LOG_INFO("GENERATED id",
            NANO_LOG_STR("resource", resource_id.value.ref)
            NANO_LOG_STR("kind", D2S2_ResourceKind_to_str(self->args.gen_resource_kind))
            NANO_LOG_H16("id", attached_id))
        
        printf("[%s] %s 0x%04X\n",
            D2S2_ResourceKind_to_str(self->args.gen_resource_kind),
            resource_id.value.ref,
            attached_id);

        RTIOsapiSemaphore_give(self->sem_exit);

        rc = NANO_RETCODE_OK;
        goto done;
    }

    NANO_CHECK_RC(
        NANO_AgentDaemon_initialize_xrce(self),
        NANO_LOG_ERROR_MSG("FAILED to initialize XRCE service"));

    if (self->args.config_file_path != NULL)
    {
        if (DDS_RETCODE_OK !=
                D2S2_Agent_load_resources_from_xml(
                    self->server, self->args.config_file_path, NULL))
        {
            goto done;
        }
    }

    if (DDS_RETCODE_OK != D2S2_Agent_start(self->server))
    {
        goto done;
    }


    rc = NANO_RETCODE_OK;
    
done:
    if (NANO_RETCODE_OK != rc)
    {
        if (self->server != NULL)
        {
            D2S2_Agent_delete(self->server);
            self->server = NULL;
        }
        if (self->agent != NULL)
        {
            NANO_XRCE_Agent_delete(self->agent);
            self->agent = NULL;
        }
    }

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_AgentDaemon_finalize(NANO_AgentDaemon *const self)
{
    NANO_LOG_FN_ENTRY

    NANO_PCOND(self != NULL)
    NANO_PCOND(self->server != NULL)

    D2S2_Agent_delete(self->server);
    self->server = NULL;

    if (self->agent != NULL)
    {
        NANO_XRCE_Agent_delete(self->agent);
        self->agent = NULL;
    }

    RTIOsapiSemaphore_delete(self->sem_exit);
    self->sem_exit = NULL;

    NANO_AgentDaemonArgs_finalize(&self->args);

    NANO_LOG_FN_EXIT
    return;
}

void
NANO_AgentDaemon_wait_for_exit(NANO_AgentDaemon *const self)
{
    NANO_LOG_FN_ENTRY
    
    NANO_PCOND(self != NULL)
    NANO_PCOND(self->sem_exit != NULL)
    
    RTIOsapiSemaphore_take(self->sem_exit, RTI_NTP_TIME_INFINITE);
    
    NANO_LOG_WARNING_MSG("AGENT daemon exiting")

    NANO_LOG_FN_EXIT
    return;
}



NANO_RetCode
NANO_AgentDaemon_parse_args(
    NANO_AgentDaemon *const self,
    const int argc,
    const char **const argv)
{
    NANO_RetCode rc = NANO_RETCODE_ERROR;
    NANO_usize i = 0;
    DDS_UnsignedLong user_str_len = 0;
    const char *user_str = NULL;
    char *invalid_str = NULL;
    
    NANO_LOG_FN_ENTRY

    NANO_LOG_DEBUG("PARSE user args",
        NANO_LOG_I32("argc", argc))
    
    for (i = 1; i < argc; i++)
    {
        user_str = argv[i];

        NANO_LOG_DEBUG("PARSE user arg",
            NANO_LOG_I32("i", i)
            NANO_LOG_STR("argv[i]", user_str))

        if (NANO_AGENT_MATCH_ARG(user_str,CONFIG))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);

            if (user_str_len == 0)
            {
                goto done;
            }

            self->args.config_file_path = DDS_String_alloc(user_str_len + 7);
            if (self->args.config_file_path == NULL)
            {
                goto done;
            }

            sprintf(self->args.config_file_path, "file://%s", user_str);
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,TIMEOUT))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }

            NANO_LOG_DEBUG("session timeout",
                NANO_LOG_USIZE("parsed", user_str_len))
            
            self->args.session_timeout.sec = user_str_len / 1000;
            self->args.session_timeout.nanosec = 
                (user_str_len - (self->args.session_timeout.sec * 1000)) * 1000000;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,AUTOATTACH))
        {
            self->args.auto_attach_resources = NANO_BOOL_TRUE;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,VERBOSITY))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }

            self->args.verbosity = user_str_len;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,UDP))
        {
            self->args.xrce_udp_enable = NANO_BOOL_TRUE;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,UDP_PORT))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }
            if (user_str_len > NANO_U16_MAX)
            {
                NANO_LOG_ERROR("INVALID UDP port number",
                    NANO_LOG_USIZE("found", user_str_len)
                    NANO_LOG_USIZE("max", NANO_U16_MAX))
                goto done;
            }

            self->args.xrce_udp_port = user_str_len;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,GENERATEID))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);

            if (user_str_len == 0)
            {
                goto done;
            }

            self->args.gen_resource_id = DDS_String_dup(user_str);
            if (self->args.gen_resource_id == NULL)
            {
                goto done;
            }
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,KIND))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);

            if (user_str_len == 0)
            {
                goto done;
            }

            if (strcmp("dp",user_str) == 0 ||
                strcmp("domainparticipant",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_DOMAINPARTICIPANT;
            }
            else if (strcmp("t",user_str) == 0 ||
                strcmp("topic",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_TOPIC;
            }
            else if (strcmp("p",user_str) == 0 ||
                strcmp("publisher",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_PUBLISHER;
            }
            else if (strcmp("s",user_str) == 0 ||
                strcmp("subscriber",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_SUBSCRIBER;
            }
            else if (strcmp("dw",user_str) == 0 ||
                strcmp("datawriter",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_DATAWRITER;
            }
            else if (strcmp("dr",user_str) == 0 ||
                strcmp("datareader",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_DATAREADER;
            }
            else if (strcmp("extsvc",user_str) == 0 ||
                strcmp("external_service",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_SERVICE;
            }
            else if (strcmp("svcres",user_str) == 0 ||
                strcmp("service_resource",user_str) == 0)
            {
                self->args.gen_resource_kind = D2S2_RESOURCEKIND_SERVICE_RESOURCE;
            }
            else
            {
                NANO_LOG_ERROR("UNKNOWN or UNSUPPORTED resource kind",
                    NANO_LOG_STR("found", user_str))
                goto done;
            }
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,READSTART))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);

            if (user_str_len == 0)
            {
                goto done;
            }

            if (strcmp("first-cached",user_str) == 0)
            {
                self->args.read_start =
                    NDDSA_READSTARTPOINT_FIRST_CACHED;
            }
            else if (strcmp("last-acked",user_str) == 0)
            {
                self->args.read_start =
                    NDDSA_READSTARTPOINT_LAST_RETURNED;
            }
            else if (strcmp("last-read",user_str) == 0)
            {
                self->args.read_start =
                    NDDSA_READSTARTPOINT_LAST_FORWARDED;
            }
            else
            {
                NANO_LOG_ERROR("UNKNOWN or UNSUPPORTED read-start-policy",
                    NANO_LOG_STR("found", user_str))
                goto done;
            }
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL))
        {
            self->args.xrce_serial_enable = NANO_BOOL_TRUE;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_DEV))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            self->args.xrce_serial_dev = user_str;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_SPEED))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }

            self->args.xrce_serial_speed = user_str_len;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_DEV_1))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            self->args.xrce_serial_dev_1 = user_str;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_SPEED_1))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }

            self->args.xrce_serial_speed_1 = user_str_len;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_DEV_2))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            self->args.xrce_serial_dev_2 = user_str;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_SPEED_2))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }

            self->args.xrce_serial_speed_2 = user_str_len;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_ADDRESS))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }
            if (user_str_len > NANO_U8_MAX)
            {
                /* invalid address */
                goto done;
            }

            self->args.xrce_serial_address.address[0] = user_str_len;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,SERIAL_TIMEOUT))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }
            if (strcmp("inf",user_str) == 0 ||
                strcmp("INF",user_str) == 0 ||
                strcmp("infinite",user_str) == 0 ||
                strcmp("INFINITE",user_str) == 0)
            {
                self->args.xrce_serial_timeout = NANO_TIMEOUT_INFINITE;
            }
            else
            {
                user_str_len = strtoul(user_str, &invalid_str, 0);

                if (invalid_str[0] != '\0')
                {
                    goto done;
                }
                self->args.xrce_serial_timeout = user_str_len;
            }

        }
        else if (NANO_AGENT_MATCH_ARG(user_str,ACKNACKPERIOD))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }
            self->args.acknack_period = user_str_len;

        }
        else if (NANO_AGENT_MATCH_ARG(user_str,HEARTBEATPERIOD))
        {
            if (i == (argc - 1))
            {
                goto done;
            }
            i += 1;
            user_str = argv[i];
            user_str_len = strlen(user_str);
            
            if (user_str_len == 0)
            {
                goto done;
            }

            user_str_len = strtoul(user_str, &invalid_str, 0);

            if (invalid_str[0] != '\0')
            {
                goto done;
            }
            self->args.heartbeat_period = user_str_len;

        }
        else if (NANO_AGENT_MATCH_ARG(user_str,HELP))
        {
            self->args.show_help = NANO_BOOL_TRUE;
        }
        else if (NANO_AGENT_MATCH_ARG(user_str,HTTP))
        {
            self->args.http_enable = NANO_BOOL_TRUE;
        }
        else
        {
            goto done;
        }
    }

    rc = NANO_RETCODE_OK;
    
done:

    NANO_LOG_FN_EXIT_RC(rc)
    return rc;
}

void
NANO_AgentDaemonArgs_finalize(NANO_AgentDaemonArgs *const self)
{
    if (self->config_file_path != NULL)
    {
        DDS_String_free(self->config_file_path);
        self->config_file_path = NULL;
    }
    if (self->gen_resource_id != NULL)
    {
        DDS_String_free(self->gen_resource_id);
        self->gen_resource_id = NULL;
    }
}