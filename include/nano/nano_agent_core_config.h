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

#ifndef nano_agent_core_config_h
#define nano_agent_core_config_h

#define NANO_FEAT_AGENT                     1
#define NANO_FEAT_TIME                      1
#define NANO_FEAT_SYSTIME                   1
#define NANO_FEAT_TYPED_SEQUENCE            1
#define NANO_FEAT_SEQUENCE                  1
#define NANO_FEAT_OPTIONAL                  1
#define NANO_FEAT_BUFFERPOOL                1
#define NANO_FEAT_RESULTSTATUS              1
#define NANO_FEAT_OBJECT                    1
#define NANO_FEAT_OBJECT_KIND_QOSPROFILE    0
#define NANO_FEAT_OBJECT_KIND_DOMAIN        0
#define NANO_FEAT_OBJECT_KIND_TYPE          0
#define NANO_FEAT_OBJECT_KIND_APPLICATION   0
#define NANO_FEAT_OBJECT_KIND_PARTICIPANT   1
#define NANO_FEAT_OBJECT_KIND_TOPIC         1
#define NANO_FEAT_OBJECT_KIND_SUBSCRIBER    1
#define NANO_FEAT_OBJECT_KIND_PUBLISHER     1
#define NANO_FEAT_OBJECT_KIND_DATAREADER    1
#define NANO_FEAT_OBJECT_KIND_DATAWRITER    1
#define NANO_FEAT_OBJECT_INFO               1
#define NANO_FEAT_OBJECT_INFO_ACTIVITY      1
#define NANO_FEAT_OBJECT_INFO_CONFIG        0
#define NANO_FEAT_OBJECT_CREATE             1
#define NANO_FEAT_OBJECT_DELETE             1
#define NANO_FEAT_REQUEST                   1
#define NANO_FEAT_REPLY                     1
#define NANO_FEAT_DATA                      0
#define NANO_FEAT_DELIVERY_CTRL             1
#define NANO_FEAT_SUBSCRIBE                 1
#define NANO_FEAT_PUBLISH                   1
#define NANO_FEAT_CONTENT_FILTER            1
#define NANO_FEAT_SAMPLEINFO                1
#define NANO_FEAT_DATA_FORMAT_PACKED        0
#define NANO_FEAT_DATA_FORMAT_DATA          1
#define NANO_FEAT_DATA_FORMAT_SAMPLE        1
#define NANO_FEAT_DATA_FORMAT_DATASEQ       0
#define NANO_FEAT_DATA_FORMAT_SAMPLESEQ     0
#define NANO_FEAT_RELIABILITY               1
#define NANO_FEAT_TIME_SYNC                 1
#define NANO_FEAT_FRAGMENT                  1
#define NANO_FEAT_STRING                    1
#define NANO_FEAT_USER_STREAMS              0
#define NANO_FEAT_CDR                       1
#define NANO_FEAT_TRANSPORT_PLUGIN_UDPV4    1
#define NANO_FEAT_TRANSPORT_PLUGIN_TCPV4    0
#define NANO_FEAT_TRANSPORT_PLUGIN_TCPV6    0
#define NANO_FEAT_TRANSPORT_PLUGIN_SERIAL   0
#define NANO_FEAT_TRANSPORT_RESOLVE         0
#define NANO_FEAT_TRANSPORT_AUTOCFG         0
#define NANO_FEAT_TRANSPORT_STRING             0
#define NANO_FEAT_TRANSPORT                 1
#define NANO_FEAT_LOG                       1 //NANO_ENABLE_PRECONDITION
#define NANO_FEAT_LOG_IMPL_PRINTF           1
#define NANO_FEAT_LOG_COLOR                 0
#define NANO_FEAT_LOG_FILENAME              1
#define NANO_FEAT_LOG_FILENAME_FULL         0
#define NANO_FEAT_LOG_FUNCTION              0
#define NANO_FEAT_LOG_TS                    0
#define NANO_LIMIT_LOG_LEVEL_DEFAULT         NANO_LOG_LEVEL_ERROR
#define NANO_LIMIT_LOG_LEVEL_MAX             NANO_LOG_LEVEL_TRACE_FN
#define NANO_FEAT_LOG_DEBUG_ON_ERROR        0

/******************************************************************************
 *                             Resource Limits
 ******************************************************************************/
#define NANO_LIMIT_OBJECTREF_MAX_LENGTH                             128
#define NANO_LIMIT_XMLREPRESENTATION_MAX_LENGTH                     512
#define NANO_LIMIT_CONTENTFILTEREXPR_MAX_LENGTH                     256
#define NANO_LIMIT_RELIABLESTREAM_SENDQUEUE_MAX_LENGTH              1
#define NANO_LIMIT_RELIABLESTREAM_MAX_MESSAGES_PER_HEARTBEAT        4
#define NANO_LIMIT_SESSION_MAX_USER_STREAMS_RELIABLE                0
#define NANO_LIMIT_SESSION_MAX_USER_STREAMS_BESTEFFORT              0
#define NANO_LIMIT_TRANSPORTLOCATORSEQ_MAX_LENGTH                   1
#define NANO_LIMIT_TRANSPORTLOCATORSTRING_ADDRESS_MAX_LENGTH        15
#define NANO_LIMIT_LOG_FILENAME_MAX_LENGTH                          16
#define NANO_LIMIT_CLIENTTRANSPORT_SENDQUEUE_MAX_LENGTH             4
#define NANO_LIMIT_CLIENTTRANSPORT_METADATASENDQUEUE_MAX_LENGTH     1
#define NANO_LIMIT_LWIP_SOCKET_SEND_BUFFER_MAX_LENGTH               256
#define NANO_LIMIT_LWIP_PBUF_OUT_MAX_LENGTH                         4

/******************************************************************************
 *                              Hacks et al.
 ******************************************************************************/
#define NANO_FEAT_LEGACY                0


#endif /* nano_core_config_h */

/** @} *//* nanocore_api_cfg */