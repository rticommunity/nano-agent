################################################################################
#
# (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
# 
# RTI grants Licensee a license to use, modify, compile, and create derivative
# works of the Software solely in combination with RTI Connext DDS. Licensee
# may redistribute copies of the Software provided that all such copies are
# subject to this License. The Software is provided "as is", with no warranty
# of any type, including any warranty for fitness for any purpose. RTI is
# under no obligation to maintain or support the Software. RTI shall not be
# liable for any incidental or consequential damages arising out of the use or
# inability to use the Software. For purposes of clarity, nothing in this
# License prevents Licensee from using alternate versions of DDS, provided
# that Licensee may not combine or link such alternate versions of DDS with
# the Software.
#
################################################################################

cmake_minimum_required(VERSION 3.9)

################################################################################

project(nano-agent
    LANGUAGES   C
    VERSION     0.1.0
    DESCRIPTION "An XRCE Agent built with RTI Connext DDS")

################################################################################
include(nano-client/core/resource/cmake/nano-build.cmake)
################################################################################

nano_project()

################################################################################

nano_project_option(
    ENABLE_CLIENT
    "Build XRCE Client API (all APIs)"
    ON
    "Include nano-client in build")

nano_project_option(
    ENABLE_CLIENT_C
    "Build XRCE Client API (C API)"
    ${${PROJECT_NAME}_ENABLE_CLIENT}
    "Include nano-client C API in build")

nano_project_option(
    ENABLE_CLIENT_CPP
    "Build XRCE Client API (C++ API)"
    ${${PROJECT_NAME}_ENABLE_CLIENT}
    "Include nano-client C++ API in build")

################################################################################

nano_project_load_connextdds()

################################################################################

function(build_nanocoreagent)
    set(BUILD_DOCS          OFF)
    set(BUILD_DOCS_API      OFF)
    set(BUILD_DOCS_MANUAL   OFF)
    set(BUILD_LIBRARIES     ${${PROJECT_NAME}_BUILD_LIBRARIES})
    set(BUILD_EXECUTABLES   OFF)
    set(ENABLE_EXAMPLES     OFF)
    set(ENABLE_XRCE_AGENT   true)
    add_subdirectory(nano-client/core       core)
endfunction()

if(NOT TARGET nano::nanocoreagent)
    build_nanocoreagent()
endif()

################################################################################

function(build_ddsagent)
    add_subdirectory(dds-agent)
endfunction()

if(NOT TARGET RTIConnextDDS::ddsagent)
    build_ddsagent()
endif()

################################################################################
# "agent" component
################################################################################

set(NANO_AGENT_INCLUDE_FILES_PUBLIC
        include/nano/nano_agent.h
        include/nano/nano_agent_config.h
        include/nano/nano_agent_service.h
        include/nano/nano_agent_transport.h
        include/nano/nano_agent_transport_udpv4.h
        include/nano/nano_agent_transport_serial.h)

set(NANO_AGENT_INCLUDE_FILES_PRIVATE
        src/service/Agent.h
        src/service/AgentTransportSerial.h
        src/service/AgentTransportUdpv4.h
        src/service/ProxyClient.h
        src/service/StreamStorage.h)

set(NANO_AGENT_INCLUDE_DIRS_PRIVATE
        src/service)

set(NANO_AGENT_SOURCE_FILES
        src/service/Agent.c
        src/service/AgentIntf.c
        src/service/AgentTransport.c
        src/service/AgentTransportSerial.c
        src/service/AgentTransportUdpv4.c
        src/service/ProxyClient.c
        src/service/MessageBufferPool.c)

set(NANO_AGENT_DEFINES          -DNANO_FEAT_AGENT=1)

set(NANO_AGENT_LIBS             nano::nanocoreagent
                                nano::nanotransportserialagent
                                RTIConnextDDS::nddsagent
                                RTIConnextDDS::c_api)

set(NANO_AGENT_LIBRARY          agent)

nano_component(NANO_AGENT)
nano_component_library(NANO_AGENT)
nano_component_install(NANO_AGENT)

unset(NANO_AGENT_INCLUDE_FILES_PUBLIC)

################################################################################
# "daemon" executable
################################################################################

set(NANO_AGENT_nanoagentd_SOURCE_FILES
        src/daemon/AgentDaemonMain.c
        src/daemon/AgentDaemon.c)

set(NANO_AGENT_nanoagentd_HEADER_FILES
        src/daemon/AgentDaemon.h
        src/daemon/AgentDaemonArgs.h)

set(NANO_AGENT_nanoagentd_INCLUDE_DIRS
        src/daemon)

nano_component_executable(NANO_AGENT nanoagentd)

################################################################################


if (${PROJECT_NAME}_BUILD_DOCS)

    set(DOC_SOURCES     index.rst
                        installation.rst
                        building.rst
                        xrce.rst
                        vars.rst
                        svc_manual.rst
                        quickstart.rst)
    
    nano_project_docs()

endif()

################################################################################

function(build_nanoclient_c)
    set(ENABLE_XRCE_AGENT   false)
    add_subdirectory(nano-client                            client-c)
endfunction()

function(build_nanoclient_cpp)
    set(ENABLE_XRCE_AGENT   false)
    add_subdirectory(nano-client/extras/nano-client-cpp     client-cpp)
endfunction()

if(${PROJECT_NAME}_ENABLE_CLIENT_C)
    build_nanoclient_c()
endif()

if(${PROJECT_NAME}_ENABLE_CLIENT_CPP)
    build_nanoclient_cpp()
endif()

################################################################################

nano_project_install()

################################################################################

nano_project_summary()

################################################################################
