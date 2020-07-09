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

#ifndef dds_agent_config_h
#define dds_agent_config_h


#ifndef DDS_AGENT_FEAT_STDAUTOID
#define DDS_AGENT_FEAT_STDAUTOID            0
#endif /* DDS_AGENT_FEAT_STDAUTOID */


#define DDS_AGENT_DDSAPI_UNKNOWN            0
#define DDS_AGENT_DDSAPI_CONNEXT            1
#define DDS_AGENT_DDSAPI_CONNEXTMICRO       2

#ifndef DDS_AGENT_DDSAPI
#define DDS_AGENT_DDSAPI        DDS_AGENT_DDSAPI_CONNEXT
#endif /* DDS_AGENT_DDSAPI */

/******************************************************************************
 *                      Include DDS API headers
 ******************************************************************************/

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT
#include "ndds_c.h"
#elif DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXTMICRO
#include "rti_me_c.h"
#else
#error "Unsupported or unknown DDS API selected"
#endif

#ifndef UNUSED_ARG
#define UNUSED_ARG(x) (void)(x)
#endif

#endif /* dds_agent_config_h */