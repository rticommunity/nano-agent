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
#include "civetweb.h"
#include "nano/nano_agent_http.h"

#define _host_from_url(svc_) \
    (((svc_)->ssl)? (svc_)->url + 8 : (svc_)->url + 7)

typedef struct NANO_HttpServiceReplyI
{
    D2S2_ExternalServiceReply base;
    struct REDAInlineListNode node;
    uint8_t *reply_data;
    size_t reply_data_len;
    size_t pass;
} NANO_HttpServiceReply;

#define NANO_HttpServiceReply_from_node(n_) \
    ((NANO_HttpServiceReply*)\
        (((unsigned char *)(n_)) - NANO_OSAPI_MEMBER_OFFSET(NANO_HttpServiceReply,node)))

typedef struct NANO_HttpServiceI
{
    D2S2_Agent * agent;
    NANO_bool ssl;
    char * address;
    uint16_t port;
    char * url;
} NANO_HttpService;

typedef struct NANO_HttpServiceResourceI
{
    char * path;
    char * url;
    struct REDAInlineList replies;
    D2S2_ExternalServiceRequestToken next_req_id;
    NANO_HttpService *service;
} NANO_HttpServiceResource;

typedef struct NANO_HttpPluginI
{
    D2S2_ExternalServicePlugin base;
    struct REDAFastBufferPool *pool_services;
    struct REDAFastBufferPool *pool_resources;
    struct REDAFastBufferPool *pool_replies;
} NANO_HttpPlugin;

#define NANO_HTTPPLUGIN_INITIALIZER \
{\
    D2S2_EXTERNALSERVICEPLUGIN_INITIALIZER, /* base */\
    NULL, /* pool_services */\
    NULL, /* pool_resources */\
    NULL  /* pool_replies */\
}

NANO_PRIVATE
char *
encode_url(const char *const url, const size_t url_len)
{
    char *encoded_url = NULL,
         *result = NULL;
    int encoded_len = 0;

    encoded_url = DDS_String_alloc(url_len * 2);
    if (NULL == encoded_url)
    {
        goto done;
    }

    encoded_len = mg_url_encode(url, encoded_url, (url_len * 2) + 1);
    if (encoded_len < url_len)
    {
        goto done;
    }

    result = encoded_url;
done:
    if (NULL == result)
    {
        if (NULL != encoded_url)
        {
            DDS_String_free(encoded_url);
        }
    }
    return result;
}

NANO_PRIVATE
char*
concat_str(
    const char *const a,
    const size_t a_len,
    const char *const b,
    const size_t b_len)
{
    const size_t c_len = a_len + b_len;
    char * concatd = DDS_String_alloc(c_len);
    if (NULL == concatd)
    {
        return NULL;
    }
    if (a_len > 0)
    {
        memcpy(concatd, a, a_len * sizeof(char));
    }
    if (b_len > 0)
    {
        memcpy(concatd + a_len, b, b_len * sizeof(char));
    }
    concatd[c_len] = '\0';
    return concatd;
}

NANO_PRIVATE
DDS_ReturnCode_t
parse_url(
    const char *const url_in,
    const uint16_t default_port,
    char **const address_out,
    uint16_t *const port_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    char *address = NULL;
    size_t address_len = 0;
    int32_t port = default_port;
    const char *colon_pos = strstr(url_in, ":");

    if (NULL != colon_pos)
    {
        address_len = colon_pos - url_in;
        address = DDS_String_alloc(address_len);
        if (NULL == address)
        {
            goto done;
        }
        memcpy(address, url_in, address_len * sizeof(char));
        address[address_len] = '\0';

        port = strtol(colon_pos+1, NULL, 0);
    }
    else
    {
        address = DDS_String_dup(url_in);
        if (NULL == address)
        {
            goto done;
        }
    }

    if (port < 0 || port > 65535)
    {
        goto done;
    }

    *address_out = address;
    *port_out = (uint16_t)port;

    retcode = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retcode)
    {
        if (NULL != address)
        {
            DDS_String_free(address);
        }
    }
    return retcode;
}

NANO_PRIVATE
char*
make_http_request(
  const char *const host,
  const char *const method,
  const char *const path,
  const char *const query,
  const size_t query_len,
  const char *const data,
  const size_t data_len,
  const char *const metadata,
  const size_t metadata_len)
{
    size_t method_len = 0,
           path_len = 0,
           host_len = 0,
           address_len = 0,
           copied_len = 0,
           req_body_len = 0,
           data_len_str_len = 0;
    char *req_body = NULL,
         *result = NULL;
    
    method_len = strlen(method);
    path_len = strlen(path);
    host_len = strlen(host);

    if (method_len == 0)
    {
        NANO_LOG_ERROR_MSG("empty HTTP method")
        goto done;
    }

    if (path_len == 0)
    {
        NANO_LOG_ERROR_MSG("empty HTTP path")
        goto done;
    }

    if (query_len > 0 && (query == NULL || query[0] == '\0'))
    {
        NANO_LOG_ERROR("invalid HTTP query",
            NANO_LOG_USIZE("query_len", query_len)
            NANO_LOG_PTR("query", query)
            if (NULL != query)
            {
                NANO_LOG_H8("query[0]", query[0])
            })
        goto done;
    }

    if (data_len > 0)
    {
        data_len_str_len = snprintf(NULL, 0, "%lu", data_len);
    }

#define _response_cp(str_, len_) \
{\
    memcpy(req_body + copied_len, (str_), (len_) * sizeof(char));\
    copied_len += (len_);\
}
#define _response_fmt(fmt_, arg_) \
{\
    size_t printed_ = 0;\
    printed_ = sprintf((char*)req_body + copied_len, fmt_, arg_);\
    copied_len += (printed_);\
}
#define _len(str_)      (sizeof(str_) - 1)
#define _response_str(str_) \
  _response_cp(str_, _len(str_))

#define _space          " "
#define _newl           "\r\n"
#define _http_version   "HTTP/1.1"
#define _host           "Host:"
#define _user_agent     "User-Agent: XRCE/0.1"
#define _content_len    "Content-Length:"

    req_body_len = 
        method_len + _len(_space) + path_len + query_len + _len(_space) +
            _len(_http_version) + _len(_newl) +
        _len(_host) + _len(_space) + host_len + _len(_newl) +
        _len(_user_agent) + _len(_newl)  +
        ((metadata_len > 0)?
            metadata_len + _len(_newl) : 0) +
        _len(_newl)  +
        ((data_len > 0)?
            _len(_content_len) + _len(_space) +
            data_len_str_len + _len(_newl) +
            data_len : 0);

    req_body = DDS_String_alloc(req_body_len);
    if (NULL == req_body)
    {
        goto done;
    }

    _response_cp(method, method_len);
    _response_str(_space);
    _response_cp(path, path_len);
    if (query_len > 0)
    {
        _response_cp(query, query_len);
    }
    _response_str(_space);
    _response_str(_http_version);
    _response_str(_newl);

    _response_str(_host);
    _response_str(_space);
    _response_cp(host, host_len);
    _response_str(_newl);

    _response_str(_user_agent);
    _response_str(_newl);

    if (metadata_len > 0)
    {
        _response_cp(metadata, metadata_len);
        _response_str(_newl);
    }

    if (data_len > 0)
    {
        _response_str(_content_len);
        _response_str(_space);
        _response_fmt("%lu", data_len);
        _response_str(_newl);
        _response_str(_newl);
        _response_cp(data, data_len);
        // _response_str(_newl);
    }
    else
    {
        _response_str(_newl);
    }
    req_body[copied_len] = '\0';

    if (copied_len != req_body_len)
    {
        NANO_LOG_ERROR("unexpected HTTP body length",
            NANO_LOG_USIZE("expected", req_body_len)
            NANO_LOG_USIZE("copied", copied_len))
        goto done;
    }

    NANO_LOG_TRACE("prepared HTTP body",
        NANO_LOG_STR("req_body", req_body))

    NANO_LOG_TRACE_FN("prepared HTTP body",
        NANO_LOG_BYTES("req_body", req_body, req_body_len + 1))

    result = req_body;
done:
    if (NULL == result)
    {
        if (NULL != req_body)
        {
            DDS_String_free(req_body);
        }
    }
    return result;
}

NANO_PRIVATE
DDS_ReturnCode_t
consume_connection(
    struct mg_connection* conn,
    uint8_t **const buffer_out,
    size_t *const buffer_len_out,
    size_t *const read_len_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    uint8_t *buff = NULL,
            *new_buff = NULL;
    size_t buff_len = 0;
    const size_t buff_len_incr = 1 << 20;
    size_t read_len = 0;

    while (1)
    {
        /* negative is only possible on first iteration when buff_len is 0 */
        if (buff_len - read_len <= 0)
        {
            RTIOsapiHeap_allocateArray(&new_buff, buff_len + buff_len_incr, uint8_t);
            if (NULL == new_buff)
            {
                goto done;
            }
            if (NULL != buff)
            {
                memcpy(new_buff, buff, buff_len * sizeof(uint8_t));
                RTIOsapiHeap_free(buff);
            }
            buff = new_buff;
            buff_len += buff_len_incr;
        }
        size_t sz_to_read = buff_len - read_len;
        int this_read = mg_read(conn, buff + read_len, sz_to_read);
        if (this_read <= 0)
        {
            break;
        }
        else
        {
            read_len += this_read;
        }
    }

    *buffer_out = buff;
    *buffer_len_out = buff_len;
    *read_len_out = read_len;

    NANO_LOG_TRACE("consumed REQUEST:",
        NANO_LOG_USIZE("buffer", buff_len)
        NANO_LOG_USIZE("size", read_len))

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        if (NULL != new_buff)
        {
           RTIOsapiHeap_free(new_buff);
        }
        if (NULL != buff)
        {
           RTIOsapiHeap_free(buff);
        }
    }
    return retcode;
}

#define startswith_https(str_) \
  (strncmp("https://", (str_), 8) == 0)

#define startswith_http(str_) \
  (strncmp("http://", (str_), 7) == 0)

DDS_ReturnCode_t
NANO_HttpPlugin_create_service(
    D2S2_ExternalServicePlugin *const plugin,
    D2S2_Agent * const agent,
    const D2S2_ResourceId *const svc_id,
    const char * const svc_url,
    const char * const svc_descriptor,
    void **const svc_data_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpService *svc = NULL;

    NANO_LOG_DEBUG("create HTTP service",
        NANO_LOG_STR("id", svc_id->value.ref)
        NANO_LOG_STR("url", svc_url)
        NANO_LOG_STR("descriptor", svc_descriptor))

    if (svc_url == NULL ||
        svc_url[0] == '\0' ||
        !(startswith_https(svc_url) || startswith_http(svc_url)))
    {
        NANO_LOG_ERROR("invalid HTTP URL",
            NANO_LOG_STR("id", svc_id->value.ref)
            NANO_LOG_STR("url", svc_url))
        goto done;
    }

    svc = (NANO_HttpService*)REDAFastBufferPool_getBuffer(self->pool_services);
    if (NULL == svc)
    {
        /* TODO log */
        goto done;
    }
    svc->agent = agent;
    svc->ssl = startswith_https(svc_url);
    svc->url = DDS_String_dup(svc_url);
    if (DDS_RETCODE_OK !=
        parse_url(
            _host_from_url(svc),
            svc->ssl? 443 : 80,
            &svc->address,
            &svc->port))
    {
        NANO_LOG_ERROR("failed to parse URL",
            NANO_LOG_STR("id", svc_id->value.ref)
            NANO_LOG_STR("url", svc_url))
        goto done;
    }
    if (NULL == svc_url)
    {
        goto done;
    }
    
    *svc_data_out = svc;

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        if (NULL != svc)
        {
            if (NULL == svc->address)
            {
                DDS_String_free(svc->address);
            }
            REDAFastBufferPool_returnBuffer(self->pool_services, svc);
        }
    }
    return retcode;
}

DDS_ReturnCode_t
NANO_HttpPlugin_delete_service(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_data)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpService *svc = (NANO_HttpService*)svc_data;

    DDS_String_free(svc->address);
    DDS_String_free(svc->url);
    REDAFastBufferPool_returnBuffer(self->pool_services, svc);
    
    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
NANO_HttpPlugin_create_resource(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_data,
    const D2S2_ResourceId *const svc_resource_id,
    const char *const svc_resource_path,
    const char *const svc_resource_descriptor,
    void **const svc_resource_data_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpService *svc = (NANO_HttpService*)svc_data;
    NANO_HttpServiceResource *svc_res = NULL;

    NANO_LOG_DEBUG("create HTTP resource",
        NANO_LOG_STR("id", svc_resource_id->value.ref)
        NANO_LOG_STR("path", svc_resource_path)
        NANO_LOG_STR("descriptor", svc_resource_descriptor))

    if (svc_resource_path == NULL ||
        svc_resource_path[0] == '\0')
    {
        NANO_LOG_ERROR("invalid empty HTTP resource path",
            NANO_LOG_STR("id", svc_resource_id->value.ref))
        goto done;
    }

    svc_res = (NANO_HttpServiceResource*)REDAFastBufferPool_getBuffer(self->pool_resources);
    if (NULL == svc_res)
    {
        /* TODO log */
        goto done;
    }
    svc_res->service = svc;
    svc_res->next_req_id = 0;
    REDAInlineList_init(&svc_res->replies);
    svc_res->path = DDS_String_dup(svc_resource_path);
    if (NULL == svc_res->path)
    {
        /* TODO log */
        goto done;
    }

    *svc_resource_data_out = svc_res;
    
    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        if (NULL != svc_res)
        {
            if (NULL != svc_res->path)
            {
                DDS_String_free(svc_res->path);
            }
            REDAFastBufferPool_returnBuffer(self->pool_resources, svc_res);
        }
    }
    return retcode;
}

DDS_ReturnCode_t
NANO_HttpPlugin_delete_resource(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_resource_data)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpServiceResource *svc_res = (NANO_HttpServiceResource*)svc_resource_data;

    DDS_String_free(svc_res->path);
    REDAFastBufferPool_returnBuffer(self->pool_resources, svc_res);

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
NANO_HttpPlugin_make_request(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_resource_data,
    const D2S2_ExternalServiceRequestFlags request_flags,
    const D2S2_Buffer *const request_query,
    const D2S2_Buffer *const request_data,
    const D2S2_Buffer *const request_metadata,
    const DDS_Boolean no_reply,
    D2S2_ExternalServiceRequestToken *const svc_request_token_out,
    RTIBool *const complete_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpServiceResource *svc_res = (NANO_HttpServiceResource*)svc_resource_data;
    NANO_HttpService *svc = svc_res->service;
    char *req_body = NULL;
    const char *method_str = NULL;
    NANO_HttpMethod method = NANO_HTTPMETHOD_UNKNOWN;
    struct mg_connection * connect = NULL;
    const struct mg_response_info * response_info = NULL;
    char *error_buff = NULL;
    const size_t error_buff_len = 1024;
    uint8_t *reply_data = NULL;
    size_t reply_buffer_len = 0,
           reply_data_len = 0;
    NANO_HttpServiceReply *reply = NULL;

    method = NANO_HttpMethod_from_flags(request_flags);
    method_str = NANO_HttpMethod_to_str(method);

    if (method == NANO_HTTPMETHOD_UNKNOWN ||
        strcmp(method_str, "UNKNOWN") == 0)
    {
        goto done;
    }

    error_buff = DDS_String_alloc(error_buff_len);
    if (NULL == error_buff)
    {
        goto done;
    }

    NANO_LOG_DEBUG("build HTTP request",
        NANO_LOG_STR("method", method_str)
        NANO_LOG_STR("path", svc_res->path)
        NANO_LOG_U32("query.len", request_query->data_len)
        NANO_LOG_U32("data.len", request_data->data_len)
        NANO_LOG_U32("metadata.len", request_metadata->data_len))

    req_body = make_http_request(
        _host_from_url(svc),
        method_str,
        svc_res->path,
        request_query->data,
        request_query->data_len,
        request_data->data,
        request_data->data_len,
        request_metadata->data,
        request_metadata->data_len);
    if (NULL == req_body)
    {
        NANO_LOG_ERROR("failed to build HTTP request",
            NANO_LOG_STR("method", method_str)
            NANO_LOG_STR("path", svc_res->path)
            NANO_LOG_U32("query.len", request_query->data_len)
            NANO_LOG_U32("data.len", request_data->data_len)
            NANO_LOG_U32("metadata.len", request_metadata->data_len))
        goto done;
    }

    NANO_LOG_DEBUG("HTTP download:",
        NANO_LOG_STR("address", svc->address)
        NANO_LOG_U16("port", svc->port)
        NANO_LOG_BOOL("ssl", svc->ssl)
        NANO_LOG_STR("request", req_body))
    connect = mg_download(svc->address, svc->port, svc->ssl,
      error_buff, error_buff_len, "%s", req_body);
    if (NULL == connect)
    {
        NANO_LOG_ERROR("failed to submit HTTP request",
            NANO_LOG_STR("address", svc->address)
            NANO_LOG_U16("port", svc->port)
            NANO_LOG_BOOL("ssl", svc->ssl)
            NANO_LOG_STR("body", req_body)
            NANO_LOG_STR("error", error_buff))
        goto done;
    }
    // NANO_LOG_DEBUG("HTTP downloaded:",
    //     NANO_LOG_STR("body", req_body))

    if (!no_reply)
    {
        if (DDS_RETCODE_OK !=
            consume_connection(
                connect, &reply_data, &reply_buffer_len, &reply_data_len))
        {
            goto done;
        }

        response_info = mg_get_response_info(connect);
        if (NULL == response_info)
        {
            goto done;
        }

        reply = (NANO_HttpServiceReply*)
            REDAFastBufferPool_getBuffer(self->pool_replies);
        if (NULL == reply)
        {
            goto done;
        }
        reply->reply_data = reply_data;
        reply->reply_data_len = reply_data_len;
        reply->base.request = ++svc_res->next_req_id;
        reply->base.status = NANO_HttpStatus_encode(response_info->status_code);
        reply->base.data_len = reply->reply_data_len;
        reply->base.metadata_len = 0;
        reply->base.data.data = reply->reply_data;
        reply->base.data.data_len = reply->reply_data_len;
        REDAInlineList_addNodeToBackEA(&svc_res->replies, &reply->node);

        *svc_request_token_out = reply->base.request;
    }
    else
    {
        *svc_request_token_out = D2S2_EXTERNALSERVICEREQUESTTOKEN_UNKNOWN;
    }

    *complete_out = RTI_TRUE;

    retcode = DDS_RETCODE_OK;
    
done:
    if (NULL != connect)
    {
        mg_close_connection(connect);
    }
    if (NULL != req_body)
    {
        DDS_String_free(req_body);
    }
    if (NULL != error_buff)
    {
        DDS_String_free(error_buff);
    }
    if (DDS_RETCODE_OK != retcode)
    {
        if (NULL != reply)
        {
            REDAFastBufferPool_returnBuffer(self->pool_replies, reply);
        }
    }
    return retcode;
}

NANO_PRIVATE
NANO_HttpServiceReply*
NANO_HttpPlugin_find_reply_by_token(
    NANO_HttpServiceResource *svc_res,
    const D2S2_ExternalServiceRequestToken request_token)
{
    struct REDAInlineListNode *reply_node = NULL;
    NANO_HttpServiceReply *next_reply = NULL;

    reply_node = REDAInlineList_getFirst(&svc_res->replies);
    while (NULL != reply_node &&
            request_token != D2S2_EXTERNALSERVICEREQUESTTOKEN_UNKNOWN)
    {
        next_reply = NANO_HttpServiceReply_from_node(reply_node);
        if (next_reply->base.request == request_token)
        {
            break;
        }
        reply_node = REDAInlineListNode_getNext(reply_node);
    }

    if (NULL == reply_node)
    {
        return NULL;
    }
    else
    {
        return next_reply;
    }
}

NANO_PRIVATE
DDS_ReturnCode_t
NANO_HttpPlugin_return_reply(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_resource_data,
    const D2S2_ExternalServiceReply * const reply_in);


DDS_ReturnCode_t
NANO_HttpPlugin_cancel_request(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_resource_data,
    const D2S2_ExternalServiceRequestToken request_token)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpServiceResource *svc_res = (NANO_HttpServiceResource*)svc_resource_data;
    NANO_HttpServiceReply *next_reply = NULL;
    UNUSED_ARG(self);
    UNUSED_ARG(svc_resource_data);
    UNUSED_ARG(request_token);

    next_reply = NANO_HttpPlugin_find_reply_by_token(svc_res, request_token);
    while (NULL == next_reply)
    {
        if (DDS_RETCODE_OK !=
            NANO_HttpPlugin_return_reply(
                plugin, svc_resource_data, &next_reply->base))
        {
            /* TODO log */
            goto done;
        }
        next_reply = NANO_HttpPlugin_find_reply_by_token(svc_res, request_token);
    }

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

DDS_ReturnCode_t
NANO_HttpPlugin_take_reply(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_resource_data,
    const D2S2_ExternalServiceRequestToken request_token,
    D2S2_ExternalServiceReply **const reply_out)
{
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpServiceResource *svc_res = (NANO_HttpServiceResource*)svc_resource_data;
    NANO_HttpServiceReply *next_reply = NULL;

    next_reply = NANO_HttpPlugin_find_reply_by_token(svc_res, request_token);
    if (NULL == next_reply)
    {
        return DDS_RETCODE_OK;
    }
    REDAInlineList_removeNodeEA(&svc_res->replies, &next_reply->node);
    *reply_out = &next_reply->base;
    return DDS_RETCODE_OK;
}

NANO_PRIVATE
DDS_ReturnCode_t
NANO_HttpPlugin_return_reply(
    D2S2_ExternalServicePlugin *const plugin,
    void *const svc_resource_data,
    const D2S2_ExternalServiceReply * const reply_in)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    NANO_HttpPlugin *self = (NANO_HttpPlugin*)plugin;
    NANO_HttpServiceReply *reply = (NANO_HttpServiceReply*)reply_in;

    RTIOsapiHeap_free(reply->reply_data);
    REDAFastBufferPool_returnBuffer(self->pool_replies, reply);

    retcode = DDS_RETCODE_OK;
    
done:
    if (DDS_RETCODE_OK != retcode)
    {
        /* handle failure */
    }
    return retcode;
}

NANO_PRIVATE
D2S2_ExternalServicePluginIntf NANO_HttpPlugin_fv_Intf =
{
    NANO_HttpPlugin_create_service,
    NANO_HttpPlugin_delete_service,
    NANO_HttpPlugin_create_resource,
    NANO_HttpPlugin_delete_resource,
    NANO_HttpPlugin_make_request,
    NANO_HttpPlugin_cancel_request,
    NANO_HttpPlugin_take_reply,
    NANO_HttpPlugin_return_reply
};

D2S2_ExternalServicePlugin*
NANO_HttpPlugin_new()
{
    static const char *const supported_protocols[3] = {"http", "https", NULL};
    static const NANO_HttpPlugin def_plugin = NANO_HTTPPLUGIN_INITIALIZER;
    NANO_HttpPlugin *plugin = NULL;
    D2S2_ExternalServicePlugin *result = NULL;
    struct REDAFastBufferPoolProperty pool_props =
        REDA_FAST_BUFFER_POOL_PROPERTY_DEFAULT;
    unsigned int http_rc = 0;
    const unsigned int http_flags = 2 /* NO_SSL */;

    http_rc = mg_init_library(http_flags);
    /* Initialize civetweb. */
    if (http_flags != http_rc)
    {
        goto done;
    }

    RTIOsapiHeap_allocateStructure(&plugin, NANO_HttpPlugin);
    if (NULL == plugin)
    {
        goto done;
    }

    *plugin = def_plugin;
    plugin->base.intf = &NANO_HttpPlugin_fv_Intf;
    plugin->base.supported_protocols = supported_protocols;

    plugin->pool_services = 
        REDAFastBufferPool_newForStructure(NANO_HttpService, &pool_props);
    if (NULL == plugin->pool_services)
    {
        goto done;
    }

    plugin->pool_resources = 
        REDAFastBufferPool_newForStructure(NANO_HttpServiceResource, &pool_props);
    if (NULL == plugin->pool_resources)
    {
        goto done;
    }

    plugin->pool_replies = 
        REDAFastBufferPool_newForStructure(NANO_HttpServiceReply, &pool_props);
    if (NULL == plugin->pool_replies)
    {
        goto done;
    }

    result = &plugin->base;

done:
    if (NULL == result && NULL != plugin)
    {
        if (NULL != plugin->pool_services)
        {
            REDAFastBufferPool_delete(plugin->pool_services);
        }
        if (NULL != plugin->pool_resources)
        {
            REDAFastBufferPool_delete(plugin->pool_resources);
        }
        if (NULL != plugin->pool_replies)
        {
            REDAFastBufferPool_delete(plugin->pool_replies);
        }
    }
    return result;
}