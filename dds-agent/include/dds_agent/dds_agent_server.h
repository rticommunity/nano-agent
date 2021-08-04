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

#ifndef dds_agent_server_h
#define dds_agent_server_h

typedef void
    (*D2S2_AgentServerInterface_OnInterfaceRegisteredCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent);

typedef void
    (*D2S2_AgentServerInterface_OnInterfaceDisposedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent);

typedef void
    (*D2S2_AgentServerInterface_OnBeforeInterfaceDisposedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent);

typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnAgentStartedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent);

typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnAgentStoppedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent);

typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnSessionOpenedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data);

typedef void
    (*D2S2_AgentServerInterface_OnSessionTimedOutCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data);

typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnSessionResetCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const old_session_data,
        void *const session_data,
        void **const session_data_out);

typedef void
    (*D2S2_AgentServerInterface_OnSessionClosedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        const D2S2_ClientSessionKey *const session_key,
        void *const session_data);

typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnResourceCreatedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_ResourceKind resource_kind,
        const D2S2_ResourceId *const resource_id,
        const D2S2_AttachedResourceId attach_id,
        void *const request_data,
        void **const resource_data_out);

typedef void
    (*D2S2_AgentServerInterface_OnResourceEventCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_ResourceKind resource_kind,
        const D2S2_ResourceId *const resource_id,
        const D2S2_AttachedResourceId attach_id,
        void *const resource_data,
        void *const request_data);


typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnDataAvailableCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId reader_id,
        void *const reader_data,
        const D2S2_ReceivedData *const recvd_data,
        DDS_Boolean *const retained_out,
        DDS_Boolean *const try_again_out);

typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnReadCreatedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId resource_id,
        void *const resource_data,
        const D2S2_ReadSpecification *const read_spec,
        void *const request_data);

typedef void
    (*D2S2_AgentServerInterface_OnReadCompleteCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId resource_id,
        void *const resource_data);

typedef DDS_ReturnCode_t
    (*D2S2_AgentServerInterface_OnReleaseReadSamplesCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId resource_id,
        void *const resource_data);


typedef void
    (*D2S2_AgentServerInterface_OnWriteEventCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId writer_id,
        void *const writer_data,
        const D2S2_DataRepresentation *const data,
        void *const request_data);

typedef void
    (*D2S2_AgentServerInterface_OnServiceRequestSubmittedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId svc_resource_id,
        void *const resource_data,
        const D2S2_ExternalServiceRequestFlags svc_flags,
        const D2S2_Buffer * const svc_query,
        const D2S2_Buffer * const svc_data,
        const D2S2_Buffer * const svc_metadata,
        void *const request_data);

typedef void
    (*D2S2_AgentServerInterface_OnReleaseServiceRepliesCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId svc_resource_id,
        void *const resource_data);

typedef void
    (*D2S2_AgentServerInterface_OnServiceReplyAvailableCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        const D2S2_AttachedResourceId svc_resource_id,
        void *const resource_data,
        const D2S2_ExternalServiceReplyStatus svc_reply_status,
        const DDS_UnsignedLong data_len,
        const DDS_UnsignedLong metadata_len,
        const D2S2_ReceivedData *const reply,
        DDS_Boolean * const retained_out,
        DDS_Boolean * const try_again_out);

typedef void
    (*D2S2_AgentServerInterface_OnMessageReceivedCallback)(
        D2S2_AgentServerInterface *const self,
        D2S2_Agent *const agent,
        D2S2_ClientSession *const session,
        void *const session_data,
        void *const message,
        DDS_Boolean *const dispose_out);

typedef struct D2S2_AgentServerInterfaceApiI
{
    D2S2_AgentServerInterface_OnSessionOpenedCallback on_session_opened;
    D2S2_AgentServerInterface_OnSessionResetCallback on_session_reset;

    D2S2_AgentServerInterface_OnSessionClosedCallback on_session_closed;

    D2S2_AgentServerInterface_OnSessionTimedOutCallback on_session_timed_out;

    D2S2_AgentServerInterface_OnResourceCreatedCallback on_resource_created;

    D2S2_AgentServerInterface_OnResourceEventCallback on_resource_deleted;

    D2S2_AgentServerInterface_OnResourceEventCallback on_resource_updated;

    D2S2_AgentServerInterface_OnReadCreatedCallback on_read_created;
    D2S2_AgentServerInterface_OnReadCompleteCallback on_read_complete;
    D2S2_AgentServerInterface_OnReleaseReadSamplesCallback on_release_read_samples;

    D2S2_AgentServerInterface_OnDataAvailableCallback on_data_available;

    D2S2_AgentServerInterface_OnWriteEventCallback on_data_written;

    D2S2_AgentServerInterface_OnInterfaceRegisteredCallback on_interface_registered;
    D2S2_AgentServerInterface_OnInterfaceDisposedCallback on_interface_disposed;
    D2S2_AgentServerInterface_OnBeforeInterfaceDisposedCallback on_before_interface_disposed;

    D2S2_AgentServerInterface_OnMessageReceivedCallback on_message_received;

    D2S2_AgentServerInterface_OnAgentStartedCallback on_agent_started;
    D2S2_AgentServerInterface_OnAgentStoppedCallback on_agent_stopped;

    D2S2_AgentServerInterface_OnServiceRequestSubmittedCallback on_service_request_submitted;
    D2S2_AgentServerInterface_OnReleaseServiceRepliesCallback on_release_service_replies;
    D2S2_AgentServerInterface_OnServiceReplyAvailableCallback on_service_reply_available;
} D2S2_AgentServerInterfaceApi;

struct D2S2_AgentServerInterfaceI
{
    struct REDAInlineListNode node;
    D2S2_Agent *agent;
    const D2S2_AgentServerInterfaceApi *intf;
};

#define D2S2_AGENTSERVERINTERFACE_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NULL, /* agent */\
    NULL /* intf */\
}

void
D2S2_AgentServerInterface_on_interface_registered(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent);

#define D2S2_AgentServerInterface_on_interface_registered(s_,ss_) \
    (s_)->intf->on_interface_registered((s_),(ss_))

void
D2S2_AgentServerInterface_on_interface_disposed(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent);

#define D2S2_AgentServerInterface_on_interface_disposed(s_,ss_) \
    (s_)->intf->on_interface_disposed((s_),(ss_))

void
D2S2_AgentServerInterface_on_before_interface_disposed(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent);

#define D2S2_AgentServerInterface_on_before_interface_disposed(s_,ss_) \
    (s_)->intf->on_before_interface_disposed((s_),(ss_))

DDS_ReturnCode_t
D2S2_AgentServerInterface_on_agent_started(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent);

#define D2S2_AgentServerInterface_on_agent_started(s_,ss_) \
    (s_)->intf->on_agent_started((s_),(ss_))

DDS_ReturnCode_t
D2S2_AgentServerInterface_on_agent_stopped(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent);

#define D2S2_AgentServerInterface_on_agent_stopped(s_,ss_) \
    (s_)->intf->on_agent_stopped((s_),(ss_))


DDS_ReturnCode_t
D2S2_AgentServerInterface_on_session_opened(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data);

#define D2S2_AgentServerInterface_on_session_opened(s_,ss_,sid_,sd_) \
    (s_)->intf->on_session_opened((s_),(ss_),(sid_),(sd_))

DDS_ReturnCode_t
D2S2_AgentServerInterface_on_session_reset(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const old_session_data,
    void *const session_data,
    void **const session_data_out);

#define D2S2_AgentServerInterface_on_session_reset(s_,ss_,sid_,osd_,sd_,sdo_) \
    (s_)->intf->on_session_reset((s_),(ss_),(sid_),(osd_),(sd_),(sdo_))

void
D2S2_AgentServerInterface_on_session_closed(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    const D2S2_ClientSessionKey *const session_key,
    void *const session_data);

#define D2S2_AgentServerInterface_on_session_closed(s_,ss_,sid_,sd_) \
    (s_)->intf->on_session_closed((s_),(ss_),(sid_),(sd_))

void
D2S2_AgentServerInterface_on_session_timed_out(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data);

#define D2S2_AgentServerInterface_on_session_timed_out(s_,ss_,sid_,sd_) \
    (s_)->intf->on_session_timed_out((s_),(ss_),(sid_),(sd_))

DDS_ReturnCode_t
D2S2_AgentServerInterface_on_resource_created(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id,
    const D2S2_AttachedResourceId attach_id,
    void *const request_data,
    void **const resource_data_out);

#define D2S2_AgentServerInterface_on_resource_created(s_,ss_,sid_,sd_,rk_,rid_,aid_,rqd_,rdo_) \
    (s_)->intf->on_resource_created((s_),(ss_),(sid_),(sd_),(rk_),(rid_),(aid_),(rqd_),(rdo_))

void
D2S2_AgentServerInterface_on_resource_deleted(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    const D2S2_ClientSessionKey *const session_id,
    void *const session_data,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id,
    const D2S2_AttachedResourceId attach_id,
    void *const resource_data,
    void *const request_data);

#define D2S2_AgentServerInterface_on_resource_deleted(s_,ss_,sid_,sd_,rk_,rid_,aid_,rd_,rqd_) \
    (s_)->intf->on_resource_deleted((s_),(ss_),(sid_),(sd_),(rk_),(rid_),(aid_),(rd_),(rqd_))

void
D2S2_AgentServerInterface_on_resource_updated(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_ResourceKind resource_kind,
    const D2S2_ResourceId *const resource_id,
    const D2S2_AttachedResourceId attach_id,
    void *const resource_data,
    void *const request_data);

#define D2S2_AgentServerInterface_on_resource_updated(s_,ss_,sid_,sd_,rk_,rid_,aid_,rd_,rqd_) \
    (s_)->intf->on_resource_updated((s_),(ss_),(sid_),(sd_),(rk_),(rid_),(aid_),(rd_),(rqd_))

void
D2S2_AgentServerInterface_on_data_available(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    const D2S2_ClientSessionKey *const session_id,
    void *const session_data,
    const D2S2_AttachedResourceId resource_id,
    void *const resource_data,
    const D2S2_ReceivedData *const recvd_data,
    DDS_Boolean *const retained_out,
    DDS_Boolean *const try_again_out);

#define D2S2_AgentServerInterface_on_data_available(s_,ss_,sid_,sd_,r_,rd_,d_,ro_,to_) \
    (s_)->intf->on_data_available((s_),(ss_),(sid_),(sd_),(r_),(rd_),(d_),(ro_),(to_))

DDS_ReturnCode_t
D2S2_AgentServerInterface_on_read_created(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const reader_data,
    const D2S2_ReadSpecification *const read_spec,
    void *const request_data);

#define D2S2_AgentServerInterface_on_read_created(s_,ss_,sid_,sd_,r_,rd_,rs_,rqd_) \
    (s_)->intf->on_read_created((s_),(ss_),(sid_),(sd_),(r_),(rd_),(rs_),(rqd_))

void
D2S2_AgentServerInterface_on_read_complete(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const resource_data);

#define D2S2_AgentServerInterface_on_read_complete(s_,ss_,sid_,sd_,r_,rs_) \
    (s_)->intf->on_read_complete((s_),(ss_),(sid_),(sd_),(r_),(rs_))

DDS_ReturnCode_t
D2S2_AgentServerInterface_on_release_read_samples(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId reader_id,
    void *const resource_data);

#define D2S2_AgentServerInterface_on_release_read_samples(s_,ss_,sid_,sd_,r_,rs_) \
    (s_)->intf->on_release_read_samples((s_),(ss_),(sid_),(sd_),(r_),(rs_))

void
D2S2_AgentServerInterface_on_data_written(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId writer_id,
    void *const writer_data,
    const D2S2_DataRepresentation *const data,
    void *const request_data);

#define D2S2_AgentServerInterface_on_data_written(s_,ss_,sid_,sd_,w_,wd_,d_,rqd_) \
    (s_)->intf->on_data_written((s_),(ss_),(sid_),(sd_),(w_),(wd_),(d_),(rqd_))

void
D2S2_AgentServerInterface_on_message_received(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    void *const message,
    DDS_Boolean *const dispose_out);

#define D2S2_AgentServerInterface_on_message_received(s_,a_,ss_,sd_,m_,d_)\
    (s_)->intf->on_message_received((s_),(a_),(ss_),(sd_),(m_),(d_))

void
D2S2_AgentServerInterface_on_service_request_submitted(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId svc_resource_id,
    void *const resource_data,
    const D2S2_ExternalServiceRequestFlags svc_flags,
    const D2S2_Buffer * const svc_query,
    const D2S2_Buffer * const svc_data,
    const D2S2_Buffer * const svc_metadata,
    void *const request_data);

#define D2S2_AgentServerInterface_on_service_request_submitted(s_, a_, ss_, sd_, hi_, rsd_, hm_, hq_, hd_, hmd_, rd_) \
  (s_)->intf->on_service_request_submitted(\
      (s_), (a_), (ss_), (sd_), (hi_), (rsd_), (hm_), (hq_), (hd_), (hmd_), (rd_))


void
D2S2_AgentServerInterface_on_release_service_replies(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId svc_resource_id,
    void *const resource_data);

#define D2S2_AgentServerInterface_on_release_service_replies(s_, a_, ss_, sd_, hi_, hrd_) \
  (s_)->intf->on_release_service_replies((s_), (a_), (ss_), (sd_), (hi_), (hrd_))

void
D2S2_AgentServerInterface_on_service_reply_available(
    D2S2_AgentServerInterface *const self,
    D2S2_Agent *const agent,
    D2S2_ClientSession *const session,
    void *const session_data,
    const D2S2_AttachedResourceId svc_resource_id,
    void *const resource_data,
    const D2S2_ExternalServiceReplyStatus svc_reply_status,
    const DDS_UnsignedLong data_len,
    const DDS_UnsignedLong metadata_len,
    const D2S2_ReceivedData *const reply,
    DDS_Boolean * const retained_out,
    DDS_Boolean * const try_again_out);

#define D2S2_AgentServerInterface_on_service_reply_available(s_, a_, ss_, sd_, hi_, hrd_, hs_, dl_, mdl_, hd_, ro_, to_) \
  (s_)->intf->on_service_reply_available(\
      (s_), (a_), (ss_), (sd_), (hi_), (hrd_), (hs_), (dl_), (mdl_), (hd_), (ro_), (to_))

#endif /* dds_agent_server_h */