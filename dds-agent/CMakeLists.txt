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

project(dds-agent
    LANGUAGES   C
    VERSION     0.1.0
    DESCRIPTION "A DDS Agent built with RTI Connext DDS")

################################################################################
include(../nano-client/core/resource/cmake/nano-build.cmake)
################################################################################

set(INCLUDE_PREFIX      dds_agent)
set(TARGET_NAMESPACE    RTIConnextDDS::)
set(LIBRARY_PREFIX      "n")

nano_project()

################################################################################

nano_project_load_connextdds()

################################################################################
# "ddsagent" component
################################################################################

set(DDS_AGENT_INCLUDE_FILES_PUBLIC
        include/dds_agent/dds_agent.h
        include/dds_agent/dds_agent_config.h
        include/dds_agent/dds_agent_log.h
        include/dds_agent/dds_agent_intf.h
        include/dds_agent/dds_agent_server.h
        include/dds_agent/dds_agent_impl_ndds.h)

set(DDS_AGENT_INCLUDE_FILES_PRIVATE
        src/impl_ndds/NddsAgent.h
        src/impl_ndds/NddsAgentIntf.h
        src/impl_ndds/NddsAgentDb.h
        src/impl_ndds/NddsAgentDtd.h
        src/impl_ndds/ApplicationLibraryXml.h
        src/impl_ndds/NddsClientSession.h
        src/impl_ndds/NddsInfrastructure.h
        src/impl_ndds/NddsDcpsVisitor.h
        src/impl_ndds/NddsXmlVisitor.h
        src/impl_ndds/NddsReader.h
        src/impl_ndds/NddsRefResource.h
        src/impl_ndds/NddsResourceFactory.h
        src/impl_ndds/NddsResourceNative.h
        src/impl_ndds/NddsResource.h
        src/impl_ndds/NddsWriter.h
        src/impl_ndds/NddsXmlResource.h)

set(DDS_AGENT_SOURCE_FILES
        src/log/Log.c
        src/impl_ndds/NddsAgent.c
        src/impl_ndds/NddsAgentIntf.c
        src/impl_ndds/NddsAgentDb.c
        src/impl_ndds/ApplicationLibraryXml.c
        src/impl_ndds/NddsClientSession.c
        src/impl_ndds/NddsEntityName.c
        src/impl_ndds/NddsInfrastructure.c
        src/impl_ndds/NddsDcpsVisitor.c
        src/impl_ndds/NddsXmlVisitor.c
        src/impl_ndds/NddsReader.c
        src/impl_ndds/NddsRefResource.c
        src/impl_ndds/NddsResource.c
        src/impl_ndds/NddsResourceFactory.c
        src/impl_ndds/NddsResourceNative.c
        src/impl_ndds/NddsWriter.c
        src/impl_ndds/NddsXmlResource.c)

set(DDS_AGENT_LIBS          RTIConnextDDS::c_api)

set(DDS_AGENT_LIBRARY       ddsagent)

nano_component(DDS_AGENT)
nano_component_library(DDS_AGENT)
nano_component_install(DDS_AGENT)

################################################################################

nano_project_install()

################################################################################

nano_project_summary()

################################################################################
