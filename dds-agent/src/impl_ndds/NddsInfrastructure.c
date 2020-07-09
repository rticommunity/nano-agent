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

#include "NddsInfrastructure.h"


#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

RTIBool
NDDSA_StringUtil_append(
    const char *const s1,
    const char *const sep,
    const char *const s2,
    char **const str_out)
{
    RTIBool res = RTI_FALSE;
    char *str = NULL,
         *ptr = NULL;
    DDS_UnsignedLong str_len = 0,
                     s1_len = 0,
                     sep_len = 0,
                     s2_len = 0;

    *str_out = NULL;

    s1_len = strlen(s1);
    sep_len = strlen(sep);
    s2_len = strlen(s2);

    str_len += s1_len;
    str_len += sep_len;
    str_len += s2_len;

    RTIOsapiHeap_allocateString(&str, str_len);
    if (str == NULL)
    {
        goto done;
    }
    ptr = str;
    
    RTIOsapiMemory_copy(ptr, s1, s1_len);
    ptr += s1_len;

    RTIOsapiMemory_copy(ptr, sep, sep_len);
    ptr += sep_len;

    RTIOsapiMemory_copy(ptr, s2, s2_len);
    ptr += s2_len;

    str[str_len] = '\0';

    *str_out = str;

    res = RTI_TRUE;
done:
    if (!res)
    {

    }
    return res;
}

/******************************************************************************
 *                               D2S2_Buffer
 ******************************************************************************/
RTIBool
D2S2_Buffer_initialize(
    D2S2_Buffer *const self,
    unsigned char *const data,
    const DDS_UnsignedLong data_len)

{
    RTIBool retcode = RTI_FALSE;

    if (data == NULL || data_len == 0)
    {
        goto done;
    }

    RTIOsapiMemory_zero(self, sizeof(D2S2_Buffer));

    RTIOsapiHeap_allocateArray(&self->data, data_len, unsigned char);
    if (self->data == NULL)
    {
        goto done;
    }
    RTIOsapiMemory_copy(self->data, data, data_len);
    self->data_len = data_len;
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}

void
D2S2_Buffer_finalize(D2S2_Buffer *const self)
{
    if (self->data != NULL)
    {
        RTIOsapiHeap_freeArray(self->data);
        self->data = NULL;
    }
    self->data_len = 0;
}

RTIBool
D2S2_Buffer_copy(
    D2S2_Buffer *const dst,
    const D2S2_Buffer *const src)
{
    RTIBool retcode = RTI_FALSE;

    if (dst->data_len != src->data_len)
    {
        RTIOsapiHeap_freeArray(dst->data);
        RTIOsapiHeap_allocateArray(&dst->data, src->data_len, unsigned char);
        if (dst->data == NULL)
        {
            goto done;
        }
        dst->data_len = src->data_len;
    }
    RTIOsapiMemory_copy(dst->data, src->data, src->data_len);
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}

int
D2S2_Buffer_compare(
    D2S2_Buffer *const left, 
    const D2S2_Buffer *const right)
{
    int retcode = -1;

    if (left->data == NULL)
    {
        if (right->data == NULL)
        {
            retcode = 0;
        }
        else
        {
            retcode = 1;
        }
    }
    else if (right->data == NULL)
    {
        retcode = -1;
    }
    else if (left->data == right->data && 
                left->data_len == right->data_len)
    {
        retcode = 0;
    }
    else if (left->data_len != right->data_len)
    {;
        retcode = REDAOrderedDataType_compareUInt(
                        &left->data_len, &right->data_len);
    }
    else
    {
        retcode = RTIOsapiMemory_compare(
                    left->data, right->data, left->data_len);
    }
    
    return retcode;
}

/******************************************************************************
 *                               D2S2_XcdrData
 ******************************************************************************/
RTIBool
D2S2_XcdrData_initialize(
    D2S2_XcdrData *const self,
    const D2S2_XcdrEncodingKind encoding,
    const D2S2_Buffer *const buffer)
{
    RTIBool retcode = RTI_FALSE;

    RTIOsapiMemory_zero(self, sizeof(D2S2_XcdrData));

    if (!D2S2_XcdrEncodingKind_is_valid(encoding) ||
        !D2S2_Buffer_is_valid(buffer))
    {
        goto done;
    }
    self->encoding = encoding;
    
    if (!D2S2_Buffer_copy(&self->buffer,buffer))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}

void
D2S2_XcdrData_finalize(D2S2_XcdrData *const self)
{
    D2S2_Buffer_finalize(&self->buffer);
    self->encoding = D2S2_XCDRENCODINGKIND_UNKNOWN;
}

RTIBool
D2S2_XcdrData_copy(
    D2S2_XcdrData *const dst,
    const D2S2_XcdrData *const src)
{
    RTIBool retcode = RTI_FALSE;

    if (!D2S2_Buffer_copy(&dst->buffer,&src->buffer))
    {
        goto done;
    }
    dst->encoding = src->encoding;
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}


int
D2S2_XcdrData_compare(
    D2S2_XcdrData *const left,
    const D2S2_XcdrData *const right)
{
    int retcode = -1;

    if (left->encoding != right->encoding)
    {
        switch (left->encoding)
        {
        case D2S2_XCDRENCODINGKIND_1:
        {
            retcode = -1;
            break;
        }
        case D2S2_XCDRENCODINGKIND_1_PL:
        {
            switch (right->encoding)
            {
            case D2S2_XCDRENCODINGKIND_1:
            {
                retcode = 1;
                break;
            }
            default:
            {
                retcode = -1;
                break;
            }
            }
            break;
        }
        case D2S2_XCDRENCODINGKIND_2:
        {
            switch (right->encoding)
            {
            case D2S2_XCDRENCODINGKIND_1:
            case D2S2_XCDRENCODINGKIND_1_PL:
            {
                retcode = 1;
                break;
            }
            default:
            {
                retcode = -1;
                break;
            }
            }
            break;
        }
        case D2S2_XCDRENCODINGKIND_2_PL:
        {
            switch (right->encoding)
            {
            case D2S2_XCDRENCODINGKIND_1:
            case D2S2_XCDRENCODINGKIND_1_PL:
            case D2S2_XCDRENCODINGKIND_2:
            {
                retcode = 1;
                break;
            }
            default:
            {
                retcode = -1;
                break;
            }
            }
            break;
        }
        default:
        {
            switch (right->encoding)
            {
            case D2S2_XCDRENCODINGKIND_1:
            case D2S2_XCDRENCODINGKIND_1_PL:
            case D2S2_XCDRENCODINGKIND_2:
            case D2S2_XCDRENCODINGKIND_2_PL:
            {
                retcode = 1;
                break;
            }
            default:
            {
                retcode = -1;
                break;
            }
            }
            break;
        }
        }
    }
    else
    {
        if (left->little_endian)
        {
            if (right->little_endian)
            {
                retcode = D2S2_Buffer_compare(&left->buffer, &right->buffer);
            }
            else
            {
                retcode = -1;
            }
        }
        else
        {
            if (right->little_endian)
            {
                retcode = 1;
            }
            else
            {
                retcode = D2S2_Buffer_compare(&left->buffer, &right->buffer);
            }
        }
    }
    return retcode;
}

/******************************************************************************
 *                          ResourceRepresentation
 ******************************************************************************/

RTIBool
D2S2_ResourceRepresentation_initialize_xml(
    D2S2_ResourceRepresentation *const self,
    const char *const xml)
{
    RTIBool retcode = RTI_FALSE;

    self->fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_XML;
    self->value.xml = DDS_String_dup(xml);
    if (self->value.xml == NULL)
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    return retcode;
}


RTIBool
D2S2_ResourceRepresentation_initialize_json(
    D2S2_ResourceRepresentation *const self,
    const char *const json)
{
    RTIBool retcode = RTI_FALSE;

    self->fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_JSON;
    self->value.json = DDS_String_dup(json);
    if (self->value.json == NULL)
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    return retcode;
}

RTIBool
D2S2_ResourceRepresentation_initialize_ref(
    D2S2_ResourceRepresentation *const self,
    const char *const ref)
{
    RTIBool retcode = RTI_FALSE;

    self->fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_REF;
    self->value.ref = DDS_String_dup(ref);
    if (self->value.ref == NULL)
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    return retcode;
}

RTIBool
D2S2_ResourceRepresentation_initialize_bin(
    D2S2_ResourceRepresentation *const self,
    const D2S2_XcdrData *const bin)
{
    RTIBool retcode = RTI_FALSE;

    self->fmt = D2S2_RESOURCEREPRESENTATIONFORMAT_BIN;
    if (!D2S2_XcdrData_copy(&self->value.bin, bin))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    return retcode;
}

void
D2S2_ResourceRepresentation_finalize(
    D2S2_ResourceRepresentation *const self)
{
    switch (self->fmt)
    {
    case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
    {
        DDS_String_free(self->value.xml);
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_JSON:
    {
        DDS_String_free(self->value.json);
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
    {
        DDS_String_free(self->value.ref);
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_BIN:
    {
        D2S2_XcdrData_finalize(&self->value.bin);
        break;
    }
    default:
        break;
    }
    RTIOsapiMemory_zero(self,sizeof(D2S2_ResourceRepresentation));
}

RTIBool
D2S2_ResourceRepresentation_copy(
    D2S2_ResourceRepresentation *const dst,
    const D2S2_ResourceRepresentation *const src)
{
    RTIBool retcode = RTI_FALSE;

    if (dst->fmt != src->fmt)
    {
        D2S2_ResourceRepresentation_finalize(dst);
    }

    switch (src->fmt)
    {
    case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
    {
        DDS_String_replace(&dst->value.xml,src->value.xml);
        if (dst->value.xml == NULL)
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_JSON:
    {
        DDS_String_replace(&dst->value.json,src->value.json);
        if (dst->value.json == NULL)
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
    {
        DDS_String_replace(&dst->value.ref,src->value.ref);
        if (dst->value.ref == NULL)
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEREPRESENTATIONFORMAT_BIN:
    {
        if (!D2S2_XcdrData_copy(&dst->value.bin, &src->value.bin))
        {
            goto done;
        }
        break;
    }
    default:
        goto done;
    }
    dst->fmt = src->fmt;

    retcode = RTI_TRUE;
    
done:
    return retcode;
}

int
D2S2_ResourceRepresentation_compare(
    D2S2_ResourceRepresentation *const left,
    const D2S2_ResourceRepresentation *const right)
{
    int res = -1;

    if (left->fmt != right->fmt)
    {
        switch (left->fmt)
        {
        case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
        {
            switch (right->fmt)
            {
            case D2S2_RESOURCEREPRESENTATIONFORMAT_JSON:
            case D2S2_RESOURCEREPRESENTATIONFORMAT_BIN:
            {
                res = -1;
                break;
            }
            default:
            {
                res = 1;
                break;
            }
            }
            break;
        }
        case D2S2_RESOURCEREPRESENTATIONFORMAT_JSON:
        {
            switch (right->fmt)
            {
            case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
            case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
            {
                res = 1;
                break;
            }
            default:
            {
                res = -1;
                break;
            }
            }
            break;
        }
        case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
        {
            res = -1;
            break;
        }
        case D2S2_RESOURCEREPRESENTATIONFORMAT_BIN:
        {
            switch (right->fmt)
            {
            case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
            case D2S2_RESOURCEREPRESENTATIONFORMAT_JSON:
            case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
            {
                res = -1;
                break;
            }
            default:
            {
                /* Should never happen */
                res = -1;
                break;
            }
            }
            break;
        }
        default:
        {
            res = 1;
            break;
        }
        }
    }
    else
    {
        switch (left->fmt)
        {
        case D2S2_RESOURCEREPRESENTATIONFORMAT_XML:
        {
            // printf("[REDAString_compare 2] XML vs XML ['%s' vs '%s']\n",
            //             left->value.xml,right->value.xml);
            res = REDAString_compare(left->value.xml,right->value.xml);
            break;
        }
        case D2S2_RESOURCEREPRESENTATIONFORMAT_JSON:
        {
            // printf("[REDAString_compare 3] JSON vs JSON ['%s' vs '%s']\n",
            //             left->value.json,right->value.json);
            res = REDAString_compare(left->value.json,right->value.json);
            break;
        }
        case D2S2_RESOURCEREPRESENTATIONFORMAT_REF:
        {
            // printf("[REDAString_compare 4] REF vs REF ['%s' vs '%s']\n",
            //             left->value.ref,right->value.ref);
            res = REDAString_compare(left->value.ref,right->value.ref);
            break;
        }
        case D2S2_RESOURCEREPRESENTATIONFORMAT_BIN:
        {
            res = D2S2_XcdrData_compare(&left->value.bin, &right->value.bin);
            break;
        }
        default:
        {
            res = 1;
            break;
        }
        }
    }


    return res;
}


/******************************************************************************
 *                               ResourceId
 ******************************************************************************/

RTIBool
D2S2_ResourceId_initialize_ref(
    D2S2_ResourceId *const self,
    const char *const ref)
{
    RTIBool retcode = RTI_FALSE;

    self->kind = D2S2_RESOURCEIDKIND_REF;
    DDS_String_replace(&self->value.ref, ref);
    if (ref == NULL)
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}

RTIBool
D2S2_ResourceId_initialize_guid(
    D2S2_ResourceId *const self,
    const DDS_GUID_t *const guid)
{
    self->kind = D2S2_RESOURCEIDKIND_REF;
    self->value.guid = *guid;
    return RTI_TRUE;
}

void
D2S2_ResourceId_finalize(D2S2_ResourceId *const self)
{
    if (self->kind == D2S2_RESOURCEIDKIND_REF)
    {
        DDS_String_free(self->value.ref);
    }
    RTIOsapiMemory_zero(self, sizeof(D2S2_ResourceId));
}

RTIBool
D2S2_ResourceId_copy(
    D2S2_ResourceId *const dst,
    const D2S2_ResourceId *const src)
{
    D2S2Log_METHOD_NAME(D2S2_ResourceId_copy)
    RTIBool retcode = RTI_FALSE;

    D2S2Log_fn_entry()

    if (dst->kind != src->kind)
    {
        D2S2_ResourceId_finalize(dst);
    }

    switch (src->kind)
    {
    case D2S2_RESOURCEIDKIND_REF:
    {
        DDS_String_replace(&dst->value.ref, src->value.ref);
        if (dst->value.ref == NULL)
        {
            goto done;
        }
        break;
    }
    case D2S2_RESOURCEIDKIND_GUID:
    {
        dst->value.guid = src->value.guid;
        break;
    }
    default:
        goto done;
    }

    dst->kind = src->kind;
    
    retcode = RTI_TRUE;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

int
D2S2_ResourceId_compare(
    const D2S2_ResourceId *const left,
    const D2S2_ResourceId *const right)
{
    int retcode = -1;

    if (left->kind != right->kind)
    {
        switch (left->kind)
        {
        case D2S2_RESOURCEIDKIND_GUID:
        {
            retcode = -1;
            break;
        }
        case D2S2_RESOURCEIDKIND_REF:
        {
            retcode = 1;
            break;
        }
        default:
        {
            retcode = 1;
            break;
        }
        }
    }
    else
    {
        switch (left->kind)
        {
        case D2S2_RESOURCEIDKIND_GUID:
        {
            retcode = DDS_GUID_compare(&left->value.guid,&right->value.guid);
            break;
        }
        case D2S2_RESOURCEIDKIND_REF:
        {
            // printf("[REDAString_compare 5] REF vs REF ['%s' vs '%s']\n",
            //             left->value.ref,right->value.ref);
            retcode = REDAString_compare(left->value.ref,right->value.ref);
            break;
        }
        default:
        {
            retcode = 1;
            break;
        }
        }
    }
    
    return retcode;
}


RTIBool
D2S2_ResourceId_as_ref(
    const D2S2_ResourceId *const id,
    const char **const ref_out)
{
    RTIBool retcode = RTI_FALSE;

    if (id->kind != D2S2_RESOURCEIDKIND_REF)
    {
        /* unsupported id type */
        goto done;
    }

    *ref_out = id->value.ref;
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}


/******************************************************************************
 *                          D2S2_Resource
 ******************************************************************************/

RTIBool
D2S2_Resource_initialize(
    D2S2_Resource *const self,
    const D2S2_ResourceKind kind,
    const D2S2_ResourceId *const id)
{
    RTIBool retcode = RTI_FALSE;
    
    self->kind = kind;

    if (!D2S2_ResourceId_copy(&self->id,id))
    {
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    return retcode;
}

void
D2S2_Resource_finalize(D2S2_Resource *const self)
{
    D2S2_ResourceId_finalize(&self->id);
    self->kind = D2S2_RESOURCEKIND_UNKNOWN;
}

RTIBool
D2S2_Resource_copy(
    D2S2_Resource *const dst,
    const D2S2_Resource *const src)
{
    RTIBool retcode = RTI_FALSE;

    if (!D2S2_ResourceId_copy(&dst->id,&src->id))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    return retcode;
}

int
D2S2_Resource_compare(
    const D2S2_Resource *const left,
    const D2S2_Resource *const right)
{
    return D2S2_ResourceId_compare(&left->id, &right->id);
}

/******************************************************************************
 *                          D2S2_ResourceIdSeq
 ******************************************************************************/


RTI_PRIVATE
RTIBool
D2S2_ResourceId_initialize_w_params(
    D2S2_ResourceId *self,
    const struct DDS_TypeAllocationParams_t * allocParams)
{
    D2S2_ResourceId def_self = D2S2_RESOURCEID_INITIALIZER;
    UNUSED_ARG(allocParams);
    *self = def_self;
    return RTI_TRUE;
}

RTI_PRIVATE
RTIBool
D2S2_ResourceId_finalize_w_params(
    D2S2_ResourceId *self,
    const struct DDS_TypeDeallocationParams_t * deallocParams)
{
    UNUSED_ARG(deallocParams);
    D2S2_ResourceId_finalize(self);
    return RTI_TRUE;
}

#define T                       D2S2_ResourceId
#define TSeq                    D2S2_ResourceIdSeq
#define T_initialize_w_params   D2S2_ResourceId_initialize_w_params
#define T_finalize_w_params     D2S2_ResourceId_finalize_w_params
#define T_copy                  D2S2_ResourceId_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */
