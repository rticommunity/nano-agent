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

#ifndef dds_agent_impl_default_h
#define dds_agent_impl_default_h

typedef enum NDDSA_ReadStartPointI
{
    NDDSA_READSTARTPOINT_FIRST_CACHED = 0x00,
    NDDSA_READSTARTPOINT_LAST_RETURNED = 0x01,
    NDDSA_READSTARTPOINT_LAST_FORWARDED = 0x02
} NDDSA_ReadStartPoint;

typedef struct NDDSA_AgentPropertiesI
{
    RTIBool auto_attach_resources;
    NDDSA_ReadStartPoint read_start_point;
} NDDSA_AgentProperties;

#define NDDSA_AGENTPROPERTIES_INITIALIZER \
{\
    RTI_FALSE, /* auto_attach_resources */\
    NDDSA_READSTARTPOINT_FIRST_CACHED /* read_start_point */\
}

D2S2_Agent*
NDDSA_Agent_create(
    const NDDSA_AgentProperties *const user_properties);

#define NDDSA_SINGLE_EA      1

#endif /* dds_agent_impl_default_h */