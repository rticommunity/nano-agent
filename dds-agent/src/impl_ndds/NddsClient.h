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

#ifndef NddsClient_h
#define NddsClient_h

#include "dds_agent/dds_agent.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsInfrastructure.h"
#include "NddsClientSession.h"

// typedef struct NDDSA_ClientI
// {
//     D2S2_ClientKey key;
//     struct REDAInlineList sessions;
//     void *user_data;
// } NDDSA_Client;

// #define NDDSA_CLIENT_INITIALIZER \
// {\
//     D2S2_CLIENTKEY_INVALID, /* key */\
//     REDA_INLINE_LIST_EMPTY, /* sessions */\
//     NULL /* user_data */\
// }

// RTIBool
// NDDSA_Client_initialize(NDDSA_Client *self);

// RTIBool
// NDDSA_Client_finalize(NDDSA_Client *const self);

// RTIBool
// NDDSA_Client_new_session(
//     NDDSA_Client *const self,
//     NDDSA_ClientSession *const session);

// RTIBool
// NDDSA_Client_delete_session(
//     NDDSA_Client *const self,
//     NDDSA_ClientSession *const session);

// RTIBool
// NDDSA_Client_allocate_resource(
//     NDDSA_Client *const self,
//     NDDSA_AttachedResourceRecord **const record_out);

// RTIBool
// NDDSA_Client_release_resource(
//     NDDSA_Client *const self,
//     NDDSA_AttachedResourceRecord *const record);


// typedef struct NDDSA_ClientSessionRecordI
// {
//     struct REDAInlineListNode node;
//     struct REDAWeakReference ref;
//     NDDSA_ClientSession session;
// } NDDSA_ClientSessionRecord;

// #define NDDSA_CLIENTSESSIONRECORD_INITIALIZER \
// {\
//     REDAInlineListNode_INITIALIZER, /* node */\
//     REDA_WEAK_REFERENCE_INVALID, /* ref */ \
//     NDDSA_CLIENTSESSION_INITIALIZER /* session */\
// }

// NDDSA_ClientSessionRecord*
// NDDSA_ClientSessionRecord_from_session(NDDSA_ClientSession *const self);

// #define NDDSA_ClientSessionRecord_from_session(s_) \
// ((NDDSA_ClientSessionRecord*) \
//     (((unsigned char*)(s_)) - \
//         NDDSA_OSAPI_MEMBER_OFFSET(NDDSA_ClientSessionRecord, session)))

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */

#endif /* NddsClient_h */