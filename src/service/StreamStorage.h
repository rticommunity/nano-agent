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

#ifndef StreamStorage_h
#define StreamStorage_h


NANO_XRCE_CUSTOMSESSIONSTORAGE_DEFINE_TYPES(
    NANO_XRCE_DefaultAgent,
    1,
    0,
    NANO_LIMIT_SESSION_MAX_USER_STREAMS_BESTEFFORT,
    NANO_LIMIT_SESSION_MAX_USER_STREAMS_RELIABLE);

#define NANO_XRCE_DEFAULTAGENTSESSIONSTORAGE_INITIALIZER \
    NANO_XRCE_CUSTOMSESSIONSTORAGE_INITIALIZER

#define NANO_XRCE_DefaultAgentSessionStorage_initialize(s_)\
    NANO_XRCE_CUSTOMSESSIONSTORAGE_INITIALIZE(s_)


typedef struct NANODllExport NANO_XRCE_SessionStorageRecordI
{
    struct REDAInlineListNode node;
    NANO_XRCE_DefaultAgentSessionStorage storage;
} NANO_XRCE_SessionStorageRecord;

#define NANO_XRCE_SESSIONSTORAGERECORD_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NANO_XRCE_DEFAULTAGENTSESSIONSTORAGE_INITIALIZER /* storage */\
}

NANO_XRCE_SessionStorageRecord*
NANO_XRCE_SessionStorageRecord_from_storage(void *ptr);

#define NANO_XRCE_SessionStorageRecord_from_storage(s_) \
((NANO_XRCE_SessionStorageRecord*)\
        (((unsigned char *)(s_)) - NANO_OSAPI_MEMBER_OFFSET(NANO_XRCE_SessionStorageRecord,storage)))

typedef struct NANODllExport NANO_XRCE_StreamStorageRecordI
{
    struct REDAInlineListNode node;
    NANO_XRCE_StreamStorage storage;
} NANO_XRCE_StreamStorageRecord;

#define NANO_XRCE_STREAMSTORAGERECORD_INITIALIZER \
{\
    REDAInlineListNode_INITIALIZER, /* node */\
    NANO_XRCE_STREAMSTORAGE_INITIALIZER /* storage */\
}

NANO_XRCE_StreamStorageRecord*
NANO_XRCE_StreamStorageRecord_from_storage(void *ptr);

#define NANO_XRCE_StreamStorageRecord_from_storage(s_) \
((NANO_XRCE_StreamStorageRecord*)\
        (((unsigned char *)(s_)) - NANO_OSAPI_MEMBER_OFFSET(NANO_XRCE_StreamStorageRecord,storage)))


#endif /* StreamStorage_h */