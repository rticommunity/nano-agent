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

#ifndef dds_agent_intf_h
#define dds_agent_intf_h


typedef enum D2S2_ResourceKindI
{
    D2S2_RESOURCEKIND_UNKNOWN             = 0x00,
    D2S2_RESOURCEKIND_DOMAINPARTICIPANT   = 0x01,
    D2S2_RESOURCEKIND_TOPIC               = 0x02,
    D2S2_RESOURCEKIND_PUBLISHER           = 0x03,
    D2S2_RESOURCEKIND_SUBSCRIBER          = 0x04,
    D2S2_RESOURCEKIND_DATAWRITER          = 0x05,
    D2S2_RESOURCEKIND_DATAREADER          = 0x06,
    D2S2_RESOURCEKIND_DOMAIN              = 0x07,
    D2S2_RESOURCEKIND_TYPE                = 0x08,
    D2S2_RESOURCEKIND_QOSPROFILE          = 0x09,
    D2S2_RESOURCEKIND_APPLICATION         = 0x0A
} D2S2_ResourceKind;

#define D2S2_ResourceKind_to_str(s_) \
    (((s_) == D2S2_RESOURCEKIND_DOMAINPARTICIPANT)?\
        "DOMAINPARTICIPANT" : \
    ((s_) == D2S2_RESOURCEKIND_TOPIC)?\
        "TOPIC" : \
    ((s_) == D2S2_RESOURCEKIND_PUBLISHER)?\
        "PUBLISHER" : \
    ((s_) == D2S2_RESOURCEKIND_SUBSCRIBER)?\
        "SUBSCRIBER" : \
    ((s_) == D2S2_RESOURCEKIND_DATAWRITER)?\
        "DATAWRITER" : \
    ((s_) == D2S2_RESOURCEKIND_DATAREADER)?\
        "DATAREADER" : \
    ((s_) == D2S2_RESOURCEKIND_DOMAIN)?\
        "DOMAIN" : \
    ((s_) == D2S2_RESOURCEKIND_TYPE)?\
        "TYPE" : \
    ((s_) == D2S2_RESOURCEKIND_QOSPROFILE)?\
        "QOSPROFILE" : \
    ((s_) == D2S2_RESOURCEKIND_APPLICATION)?\
        "APPLICATION" : \
        "UNKNOWN")

typedef enum D2S2_ResourceIdKindI
{
    D2S2_RESOURCEIDKIND_NONE                = 0x00,
    D2S2_RESOURCEIDKIND_GUID                = 0x01,
    D2S2_RESOURCEIDKIND_REF                 = 0x02
} D2S2_ResourceIdKind;

typedef union D2S2_ResourceIdValueI
{
    char *ref;
    DDS_GUID_t guid;
} D2S2_ResourceIdValue;

#define D2S2_RESOURCEIDVALUE_INITIALIZER \
{\
    NULL /* ref */\
}

typedef struct D2S2_ResourceIdI
{
    D2S2_ResourceIdKind kind;
    D2S2_ResourceIdValue value;
} D2S2_ResourceId;

#define D2S2_RESOURCEID_INITIALIZER \
{\
    D2S2_RESOURCEKIND_UNKNOWN, /* kind */\
    D2S2_RESOURCEIDVALUE_INITIALIZER /* value */\
}

void
D2S2_ResourceId_set_unknown(D2S2_ResourceId *const self);

#define D2S2_ResourceId_set_unknown(s_) \
{\
    (s_)->kind = D2S2_RESOURCEIDKIND_NONE;\
    RTIOsapiMemory_zero(&(s_)->value, sizeof((s_)->value));\
}

void
D2S2_ResourceId_set_ref(D2S2_ResourceId *const self, const char *ref);

#define D2S2_ResourceId_set_ref(s_,r_) \
{\
    (s_)->kind = D2S2_RESOURCEIDKIND_REF;\
    (s_)->value.ref = (r_);\
}

void
D2S2_ResourceId_set_guid(D2S2_ResourceId *const self, const DDS_GUID_t *guid);

#define D2S2_ResourceId_set_guid(s_,g_) \
{\
    (s_)->kind = D2S2_RESOURCEIDKIND_GUID;\
    (s_)->value.guid = *(g_);\
}

DDS_SEQUENCE(D2S2_ResourceIdSeq, D2S2_ResourceId);

#define D2S2_ENTITYNAME_DEPTH_LIBRARY         1
#define D2S2_ENTITYNAME_DEPTH_APPLICATION     2
#define D2S2_ENTITYNAME_DEPTH_PARTICIPANT     2
#define D2S2_ENTITYNAME_DEPTH_QOSPROFILE      2
#define D2S2_ENTITYNAME_DEPTH_DOMAIN          2
#define D2S2_ENTITYNAME_DEPTH_PUBLISHER       3
#define D2S2_ENTITYNAME_DEPTH_SUBSCRIBER      3
#define D2S2_ENTITYNAME_DEPTH_TOPIC           3
#define D2S2_ENTITYNAME_DEPTH_DATAWRITER      4
#define D2S2_ENTITYNAME_DEPTH_DATAREADER      4

typedef struct D2S2_EntityNameI
{
    char **components;
    DDS_UnsignedLong depth;
} D2S2_EntityName;

#define D2S2_ENTITYNAME_INITIALIZER \
{\
    NULL, /* components */\
    0 /* depth */\
}

DDS_Boolean
D2S2_EntityName_from_id(
    D2S2_EntityName *const self,
    const D2S2_ResourceId *const id);

DDS_Boolean
D2S2_EntityName_to_ref(
    const D2S2_EntityName *const self,
    const DDS_UnsignedLong max_depth,
    char **const ref_out);

DDS_Boolean
D2S2_EntityName_component(
    const D2S2_EntityName *const self,
    const DDS_UnsignedLong depth,
    const char **const component_out);

void
D2S2_EntityName_leaf(const D2S2_EntityName *const self);

#define D2S2_EntityName_leaf(s_) \
    (((s_)->depth > 0)? (s_)->components[(s_)->depth - 1] : NULL)

void
D2S2_EntityName_root(const D2S2_EntityName *const self);

#define D2S2_EntityName_root(s_) \
    (((s_)->depth > 0)? (s_)->components[0] : NULL)

DDS_UnsignedLong
D2S2_EntityName_depth(const D2S2_EntityName *const self);

#define D2S2_EntityName_depth(s_) \
    ((s_)->depth)

void
D2S2_EntityName_finalize(D2S2_EntityName *const self);

typedef DDS_UnsignedLong D2S2_ClientKey;
#define D2S2_CLIENTKEY_INVALID          0x00000000

typedef DDS_Octet D2S2_ClientSessionId;
#define D2S2_CLIENTSESSIONID_INVALID          0x00

typedef DDS_UnsignedLong D2S2_AttachedResourceId;
#define D2S2_ATTACHEDRESOURCEID_INVALID 0x00000000


typedef struct D2S2_ClientSessionKeyI
{
    D2S2_ClientKey client;
    D2S2_ClientSessionId id;
} D2S2_ClientSessionKey;

#define D2S2_CLIENTSESSIONKEY_INITIALIZER \
{\
    D2S2_CLIENTKEY_INVALID, /* client */\
    D2S2_CLIENTSESSIONID_INVALID /* id */\
}

typedef struct D2S2_ClientSessionI
{
    D2S2_ClientSessionKey key;
} D2S2_ClientSession;

#define D2S2_CLIENTSESSION_INITIALIZER \
{\
    D2S2_CLIENTSESSIONKEY_INITIALIZER /* key */\
}

typedef struct D2S2_ResourceI
{
    D2S2_ResourceKind kind;
    D2S2_ResourceId id;
} D2S2_Resource;

#define D2S2_RESOURCE_INITIALIZER \
{\
    D2S2_RESOURCEKIND_UNKNOWN, /* kind */\
    D2S2_RESOURCEID_INITIALIZER /* id */\
}

typedef struct D2S2_AttachedResourceI
{
    D2S2_ResourceKind kind;
    D2S2_ResourceId resource;
    D2S2_ClientKey client;
    D2S2_ClientSessionId session;
    D2S2_AttachedResourceId id;
} D2S2_AttachedResource;

#define D2S2_ATTACHEDRESOURCE_INITIALIZER \
{\
    D2S2_RESOURCEKIND_UNKNOWN, /* kind */\
    D2S2_RESOURCEID_INITIALIZER, /* resource */\
    D2S2_CLIENTKEY_INVALID, /* client */\
    D2S2_CLIENTSESSIONID_INVALID, /* session */\
    D2S2_ATTACHEDRESOURCEID_INVALID /* id */\
}

typedef struct D2S2_BufferI
{
    unsigned char *data;
    DDS_UnsignedLong data_len;
} D2S2_Buffer;

#define D2S2_BUFFER_INITIALIZER \
{\
    NULL, /* data */\
    0 /* data_len */\
}


DDS_Boolean
D2S2_Buffer_is_valid(D2S2_Buffer *const self);

#define D2S2_Buffer_is_valid(s_) \
    ((s_)->data != NULL && (s_)->data_len > 0)

typedef enum D2S2_XcdrEncodingKindI
{
    D2S2_XCDRENCODINGKIND_UNKNOWN   = 0x00,
    D2S2_XCDRENCODINGKIND_1         = 0x01,
    D2S2_XCDRENCODINGKIND_1_PL      = 0x02,
    D2S2_XCDRENCODINGKIND_2         = 0x03,
    D2S2_XCDRENCODINGKIND_2_PL      = 0x04
} D2S2_XcdrEncodingKind;

DDS_Boolean
D2S2_XcdrEncodingKind_is_valid(D2S2_XcdrEncodingKind *const self);

#define D2S2_XcdrEncodingKind_is_valid(s_) \
    ((s_) == D2S2_XCDRENCODINGKIND_1 || \
        (s_) == D2S2_XCDRENCODINGKIND_1_PL || \
        (s_) == D2S2_XCDRENCODINGKIND_2 || \
        (s_) == D2S2_XCDRENCODINGKIND_2_PL)

typedef struct D2S2_XcdrDataI
{
    D2S2_XcdrEncodingKind encoding;
    DDS_Boolean little_endian;
    D2S2_Buffer buffer;
} D2S2_XcdrData;

#define D2S2_XCDRDATA_INITIALIZER \
{\
    D2S2_XCDRENCODINGKIND_UNKNOWN, /* encoding */\
    DDS_BOOLEAN_FALSE, /* little_endian */\
    D2S2_BUFFER_INITIALIZER /* buffer */\
}

typedef union D2S2_ResourceRepresentationValueI
{
    D2S2_XcdrData bin;
    char *ref;
    char *xml;
    char *json;
} D2S2_ResourceRepresentationValue;

#define D2S2_RESOURCEREPRESENTATIONVALUE_INITIALIZER \
{\
    D2S2_XCDRDATA_INITIALIZER /* bin */\
}

typedef enum D2S2_ResourceRepresentationFormatI
{
    D2S2_RESOURCEREPRESENTATIONFORMAT_UNKNOWN     = 0x00,
    /**
     * @brief A textual representation format which uses the DDS/XML standard 
     * encoding.
     * 
     */
    D2S2_RESOURCEREPRESENTATIONFORMAT_XML         = 0x01,
    /**
     * @brief A textual representation format which uses the DDS/JSON standard
     * encoding.
     * 
     */
    D2S2_RESOURCEREPRESENTATIONFORMAT_JSON        = 0x02,
    /**
     * @brief A textual representation format where each resources is 
     * identified by a fully qualified "reference string"
     * (e.g. "Foo::Bar::Baz").
     * 
     */
    D2S2_RESOURCEREPRESENTATIONFORMAT_REF         = 0x03,
    /**
     * @brief A binary representation format which encodes each resource using
     * the XCDR serialization of its DDS Discovery data (only supported by
     * DomainParticipant, DataWriter, DataReader, and Topic).
     * 
     */
    D2S2_RESOURCEREPRESENTATIONFORMAT_BIN   = 0x04
} D2S2_ResourceRepresentationFormat;

typedef struct D2S2_ResourceRepresentationI
{
    D2S2_ResourceRepresentationFormat fmt;
    D2S2_ResourceRepresentationValue value;
} D2S2_ResourceRepresentation;

#define D2S2_RESOURCEREPRESENTATION_INITIALIZER \
{\
    D2S2_RESOURCEREPRESENTATIONFORMAT_UNKNOWN, /* fmt */\
    D2S2_RESOURCEREPRESENTATIONVALUE_INITIALIZER /* value */\
}

RTIBool
D2S2_ResourceRepresentation_initialize_xml(
    D2S2_ResourceRepresentation *const self,
    const char *const xml);

RTIBool
D2S2_ResourceRepresentation_initialize_json(
    D2S2_ResourceRepresentation *const self,
    const char *const json);

RTIBool
D2S2_ResourceRepresentation_initialize_ref(
    D2S2_ResourceRepresentation *const self,
    const char *const ref);

RTIBool
D2S2_ResourceRepresentation_initialize_bin(
    D2S2_ResourceRepresentation *const self,
    const D2S2_XcdrData *const bin);

void
D2S2_ResourceRepresentation_finalize(
    D2S2_ResourceRepresentation *const self);

RTIBool
D2S2_ResourceRepresentation_copy(
    D2S2_ResourceRepresentation *const dst,
    const D2S2_ResourceRepresentation *const src);

int
D2S2_ResourceRepresentation_compare(
    D2S2_ResourceRepresentation *const left,
    const D2S2_ResourceRepresentation *const right);

typedef enum D2S2_DataRepresentationFormatI
{
    D2S2_DATAREPRESENTATIONFORMAT_UNKNOWN       = 0x00,
    D2S2_DATAREPRESENTATIONFORMAT_XCDR          = 0x01,
    D2S2_DATAREPRESENTATIONFORMAT_XML           = 0x02,
    D2S2_DATAREPRESENTATIONFORMAT_JSON          = 0x03
} D2S2_DataRepresentationFormat;

typedef union D2S2_DataRepresentationValueI
{
    D2S2_XcdrData xcdr;
    char *xml;
    char *json;
} D2S2_DataRepresentationValue;

#define D2S2_DATAREPRESENTATIONVALUE_INITIALIZER \
{\
    D2S2_XCDRDATA_INITIALIZER /* xcdr */\
}

typedef struct D2S2_DataRepresentationI
{
    D2S2_DataRepresentationFormat fmt;
    D2S2_DataRepresentationValue value;
} D2S2_DataRepresentation;

#define D2S2_DATAREPRESENTATION_INITIALIZER \
{\
    D2S2_DATAREPRESENTATIONFORMAT_UNKNOWN, /* fmt */\
    D2S2_DATAREPRESENTATIONVALUE_INITIALIZER /* value */\
}

typedef struct D2S2_AgentI D2S2_Agent;

typedef struct D2S2_ReadSpecificationI
{
    DDS_UnsignedLong max_samples;
    DDS_UnsignedLong max_elapsed_time;
    DDS_UnsignedLong max_bytes_per_second;
    DDS_UnsignedLong min_pace_period;
    char *content_filter_expr;
} D2S2_ReadSpecification;

#define D2S2_READSPECIFICATION_INITIALIZER \
{\
    0, /* max_samples */\
    0, /* max_elapsed_time */\
    0, /* max_bytes_per_second */\
    0, /* min_pace_period */\
    NULL /* content_filter_expr */\
}

typedef struct D2S2_ReceivedDataI
{
    D2S2_ClientSessionKey session_key;
    D2S2_AttachedResourceId reader_id;
    const D2S2_DataRepresentation *data;
} D2S2_ReceivedData;

#define D2S2_RECEIVEDDATA_INITIALIZER \
{\
    D2S2_CLIENTSESSIONKEY_INITIALIZER, /* session_key */\
    D2S2_ATTACHEDRESOURCEID_INVALID, /* reader_id */\
    NULL /* data */\
}

typedef union D2S2_AgentConfigValueI
{
    const char *url;
    D2S2_DataRepresentation literal;
} D2S2_AgentConfigValue;

#define D2S2_AGENTCONFIGVALUE_INITIALIZER \
{\
    NULL /* url */\
}

typedef enum D2S2_AgentConfigFormatI
{
    D2S2_AGENTCONFIGFORMAT_UNKNOWN = 0x00,
    D2S2_AGENTCONFIGFORMAT_URL = 0x01,
    D2S2_AGENTCONFIGFORMAT_LITERAL = 0x02
} D2S2_AgentConfigFormat;

typedef struct D2S2_AgentConfigI
{
    D2S2_AgentConfigFormat fmt;
    D2S2_AgentConfigValue value;
} D2S2_AgentConfig;

#define D2S2_AGENTCONFIG_INITIALIZER \
{\
    D2S2_AGENTCONFIGFORMAT_UNKNOWN, /* fmt */\
    D2S2_AGENTCONFIGVALUE_INITIALIZER /* value */\
}


typedef D2S2_Agent*
    (*D2S2_Agent_CreateFn)();

typedef void
    (*D2S2_Agent_DeleteFn)(D2S2_Agent *const self);

typedef struct D2S2_ClientSessionPropertiesI
{
    struct DDS_Duration_t timeout;
} D2S2_ClientSessionProperties;

#define D2S2_CLIENTSESSIONPROPERTIES_INITIALIZER \
{\
    { DDS_DURATION_INFINITE_SEC, DDS_DURATION_INFINITE_NSEC }, /* timeout */\
}

typedef struct D2S2_ResourcePropertiesI
{
    DDS_Boolean reuse;
    DDS_Boolean replace;
    DDS_DomainId_t domain_id;
} D2S2_ResourceProperties;

#define D2S2_RESOURCEPROPERTIES_INITIALIZER \
{\
    DDS_BOOLEAN_FALSE, /* reuse */\
    DDS_BOOLEAN_FALSE, /* replace */\
    0 /* domain_id */\
}

typedef struct D2S2_AgentServerInterfaceI D2S2_AgentServerInterface;

typedef 
DDS_ReturnCode_t
    (*D2S2_Agent_RegisterInterfaceFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const intf);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_OpenSessionFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        const D2S2_ClientSessionKey *const session_key,
        const D2S2_ClientSessionProperties *const properties,
        void *const session_data);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_CloseSessionFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        const D2S2_ClientSessionKey *const session_key);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_CreateResourceFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        D2S2_ClientSession *const session,
        const D2S2_AttachedResourceId resource_id,
        const D2S2_ResourceKind kind,
        const D2S2_ResourceRepresentation *const resource_repr,
        const D2S2_AttachedResourceId parent_id,
        const D2S2_ResourceProperties *const properties,
        void *const request_param);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_DeleteResourceFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        D2S2_ClientSession *const session,
        const D2S2_AttachedResourceId resource_id,
        void *const request_param);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_LookupResourceFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        D2S2_ClientSession *const session,
        const D2S2_AttachedResourceId resource_id,
        DDS_Boolean *const resource_exists_out,
        void **const resource_data_out);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_ReadFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        D2S2_ClientSession *const session,
        const D2S2_AttachedResourceId reader_id,
        const D2S2_ReadSpecification *const read_spec,
        void *const request_param);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_CancelReadFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        D2S2_ClientSession *const session,
        const D2S2_AttachedResourceId reader_id,
        void *const request_param);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_ReturnLoanFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        D2S2_ClientSession *const session,
        D2S2_ReceivedData *const data);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_WriteFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const src,
        D2S2_ClientSession *const session,
        const D2S2_AttachedResourceId writer_id,
        const D2S2_DataRepresentation *const data,
        void *const request_param);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_LoadResourcesFromXmlFn)(
        D2S2_Agent *const self,
        const char *const xml_url,
        const D2S2_ResourceProperties *const res_properties);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_ReceiveMessageFn)(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key,
    void *const message);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_StartFn)(D2S2_Agent *const self);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_StopFn)(D2S2_Agent *const self);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_GenerateAttachedResourceIdFn)(
        D2S2_Agent *const self,
        const D2S2_ResourceId *resource_id,
        const D2S2_ResourceKind resource_kind,
        D2S2_AttachedResourceId *const id_out);

typedef struct D2S2_ClientSessionEventI
{
    D2S2_AgentServerInterface *intf;
    D2S2_ClientSessionKey session_key;
    void *listener;    
} D2S2_ClientSessionEvent;

typedef void
    (*D2S2_Agent_OnSessionEventCallback)(
        D2S2_Agent *const self,
        D2S2_ClientSession *const session,
        D2S2_ClientSessionEvent *const event,
        DDS_Boolean *const reschedule_out);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_CreateSessionEventFn)(
        D2S2_Agent *const self,
        D2S2_AgentServerInterface *const intf,
        D2S2_ClientSession *const session,
        void *const event_listener,
        D2S2_Agent_OnSessionEventCallback on_event,
        D2S2_ClientSessionEvent **const event_out);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_FireSessionEventFn)(
        D2S2_Agent *const self,
        D2S2_ClientSessionEvent *const event,
        const struct RTINtpTime *const delay);

typedef DDS_ReturnCode_t
    (*D2S2_Agent_DeleteSessionEventFn)(
        D2S2_Agent *const self,
        D2S2_ClientSessionEvent *const event);

typedef struct D2S2_AgentIntfI
{
    D2S2_Agent_DeleteFn delete_agent;
    D2S2_Agent_RegisterInterfaceFn register_interface;
    D2S2_Agent_OpenSessionFn open_session;
    D2S2_Agent_CloseSessionFn close_session;
    D2S2_Agent_CreateResourceFn create_resource;
    D2S2_Agent_DeleteResourceFn delete_resource;
    D2S2_Agent_LookupResourceFn lookup_resource;
    D2S2_Agent_ReadFn read;
    D2S2_Agent_CancelReadFn cancel_read;
    D2S2_Agent_ReturnLoanFn return_loan;
    D2S2_Agent_WriteFn write;
    D2S2_Agent_ReceiveMessageFn receive_message;
    D2S2_Agent_LoadResourcesFromXmlFn load_resources_from_xml;
    D2S2_Agent_StartFn start;
    D2S2_Agent_StopFn stop;
    D2S2_Agent_GenerateAttachedResourceIdFn generate_attached_resource_id;
    D2S2_Agent_CreateSessionEventFn create_session_event;
    D2S2_Agent_FireSessionEventFn fire_session_event;
    D2S2_Agent_DeleteSessionEventFn delete_session_event;
} D2S2_AgentIntf;

#define D2S2_AGENTINTF_INITIALIZER \
{\
    NULL, /* delete_agent */\
    NULL, /* register_interface */\
    NULL, /* open_session */\
    NULL, /* close_session */\
    NULL, /* create_resource */\
    NULL, /* delete_resource */\
    NULL, /* lookup_resource */\
    NULL, /* read */\
    NULL, /* cancel_read */\
    NULL, /* return_loan */\
    NULL, /* write */\
    NULL, /* receive_message */\
    NULL, /* load_resources_from_config */\
    NULL, /* start */\
    NULL, /* stop */\
    NULL, /* generate_attached_resource_id */\
    NULL, /* create_session_event */\
    NULL, /* fire_session_event */\
    NULL  /* delete_session_event */\
}

struct D2S2_AgentI
{
    const D2S2_AgentIntf *intf;
};

#define D2S2_AGENT_INITIALIZER \
{\
    NULL /* intf */\
}

void
D2S2_Agent_delete(D2S2_Agent *const self);

#define D2S2_Agent_delete(s_) \
    (s_)->intf->delete_agent((s_))

DDS_ReturnCode_t
D2S2_Agent_register_interface(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const intf);

#define D2S2_Agent_register_interface(s_,i_) \
    (s_)->intf->register_interface((s_),(i_))

DDS_ReturnCode_t
D2S2_Agent_open_session(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key,
    const D2S2_ClientSessionProperties *const properties,
    void *const session_data);

#define D2S2_Agent_open_session(s_,si_,k_,p_,sd_) \
    (s_)->intf->open_session((s_),(si_),(k_),(p_),(sd_))

DDS_ReturnCode_t
D2S2_Agent_close_session(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key);

#define D2S2_Agent_close_session(s_,si_,sk_) \
    (s_)->intf->close_session((s_),(si_),(sk_))

DDS_ReturnCode_t
D2S2_Agent_create_resource(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceRepresentation *const resource_repr,
    const D2S2_AttachedResourceId parent_id,
    const D2S2_ResourceProperties *const properties,
    void *const request_param);

#define D2S2_Agent_create_resource(s_,si_,sk_,r_,k_,rr_,pi_,p_,rp_) \
    (s_)->intf->create_resource((s_),(si_),(sk_),(r_),(k_),(rr_),(pi_),(p_),(rp_))

DDS_ReturnCode_t
D2S2_Agent_delete_resource(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    void *const request_param);

#define D2S2_Agent_delete_resource(s_,si_,sk_,r_,rp_) \
    (s_)->intf->delete_resource((s_),(si_),(sk_),(r_),(rp_))

DDS_ReturnCode_t
D2S2_Agent_lookup_resource(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId resource_id,
    DDS_Boolean *const resource_exists_out,
    void **const resource_data_out);

#define D2S2_Agent_lookup_resource(s_,si_,sk_,rid_,reo_,ro_) \
    (s_)->intf->lookup_resource((s_),(si_),(sk_),(rid_),(reo_),(ro_))

DDS_ReturnCode_t
D2S2_Agent_read(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId reader_id,
    const D2S2_ReadSpecification *const read_spec,
    void *const request_param);

#define D2S2_Agent_read(s_,si_,sk_,r_,rs_,rp_) \
    (s_)->intf->read((s_),(si_),(sk_),(r_),(rs_),(rp_))

DDS_ReturnCode_t
D2S2_Agent_cancel_read(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId reader_id,
    void *const request_param);

#define D2S2_Agent_cancel_read(s_,si_,sk_,r_,rp_) \
    (s_)->intf->cancel_read((s_),(si_),(sk_),(r_),(rp_))

DDS_ReturnCode_t
D2S2_Agent_return_loan(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    D2S2_ReceivedData *const data);

#define D2S2_Agent_return_loan(s_,si_,ss_,d_) \
    (s_)->intf->return_loan((s_),(si_),(ss_),(d_))

DDS_ReturnCode_t
D2S2_Agent_write(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    D2S2_ClientSession *const session,
    const D2S2_AttachedResourceId writer_id,
    const D2S2_DataRepresentation *const data,
    void *const request_param);

#define D2S2_Agent_write(s_,si_,sk_,w_,d_,rp_) \
    (s_)->intf->write((s_),(si_),(sk_),(w_),(d_),(rp_))

DDS_ReturnCode_t
D2S2_Agent_load_resources_from_xml(
    D2S2_Agent *const self,
    const char *const xml_url,
    const D2S2_ResourceProperties *const res_properties);

#define D2S2_Agent_load_resources_from_xml(s_,x_,rp_) \
    (s_)->intf->load_resources_from_xml((s_),(x_),(rp_))

DDS_ReturnCode_t
D2S2_Agent_receive_message(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const src,
    const D2S2_ClientSessionKey *const session_key,
    void *const message);

#define D2S2_Agent_receive_message(s_,si_,sk_,m_) \
    (s_)->intf->receive_message((s_),(si_),(sk_),(m_))

DDS_ReturnCode_t
D2S2_Agent_start(D2S2_Agent *const self);

#define D2S2_Agent_start(s_) \
    (s_)->intf->start((s_))

DDS_ReturnCode_t
D2S2_Agent_stop(D2S2_Agent *const self);

#define D2S2_Agent_stop(s_) \
    (s_)->intf->stop((s_))

DDS_ReturnCode_t
D2S2_Agent_generate_attached_resource_id(
    D2S2_Agent *const self,
    const D2S2_ResourceId *resource_id,
    const D2S2_ResourceKind resource_kind,
    D2S2_AttachedResourceId *const id_out);

#define D2S2_Agent_generate_attached_resource_id(s_, rid_, rk_, aid_) \
    (s_)->intf->generate_attached_resource_id((s_),(rid_),(rk_),(aid_))


DDS_ReturnCode_t
D2S2_Agent_create_session_event(
    D2S2_Agent *const self,
    D2S2_AgentServerInterface *const intf,
    D2S2_ClientSession *const session,
    void *const event_listener,
    D2S2_Agent_OnSessionEventCallback on_event,
    D2S2_ClientSessionEvent **const event_out);

#define D2S2_Agent_create_session_event(s_, i_, sk_, el_, oe_, e_) \
    (s_)->intf->create_session_event((s_),(i_),(sk_),(el_),(oe_),(e_))

DDS_ReturnCode_t
D2S2_Agent_fire_session_event(
    D2S2_Agent *const self,
    D2S2_ClientSessionEvent *const event,
    const struct RTINtpTime *const delay);

#define D2S2_Agent_fire_session_event(s_, e_, d_) \
    (s_)->intf->fire_session_event((s_),(e_),(d_))

DDS_ReturnCode_t
D2S2_Agent_delete_session_event(
    D2S2_Agent *const self,
    D2S2_ClientSessionEvent *const event);

#define D2S2_Agent_delete_session_event(s_, e_) \
    (s_)->intf->delete_session_event((s_),(e_))

#endif /* dds_agent_intf_h */