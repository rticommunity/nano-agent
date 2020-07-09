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

#include "dds_agent/dds_agent.h"


#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#include "NddsInfrastructure.h"

#define REF_SEP         "::"
#define REF_SEP_LEN     2

RTI_PRIVATE
void
D2S2_EntityName_free_ref_components(
    char **ref_components,
    const DDS_UnsignedLong ref_components_size)
{
    DDS_UnsignedLong i = 0;
    for (i = 0; i < ref_components_size; i++)
    {
        RTIOsapiHeap_free(ref_components[i]);
        ref_components[i] = NULL;
    }
}

RTI_PRIVATE
void
D2S2_EntityName_count_ref_components(
    const char *const ref,
    RTIBool *const valid_ref_out,
    DDS_UnsignedLong *const components_count_out,
    DDS_UnsignedLong *const ref_len_out,
    DDS_UnsignedLong *const ref_start_out)
{
    DDS_UnsignedLong components_count = 0,
                     ref_len = 0,
                     ch_i = 0,
                     ref_start = 0;
    const char *sep = NULL,
               *ref_head = NULL;

    *valid_ref_out = DDS_BOOLEAN_FALSE;
    *components_count_out = 0;
    *ref_len_out = 0;
    *ref_start_out = 0;

    ref_len = strlen(ref);
    if (ref_len == 0)
    {
        /* empty ref => invalid ref */
        goto done;
    }
    ref_head = ref;

    do {
        DDS_UnsignedLong consumed = 0;
        RTIBool valid_component = RTI_FALSE;

        sep = strstr(ref_head, REF_SEP);
        if (sep != NULL)
        {
            if (sep == ref_head)
            {
                if (ch_i == 0)
                {
                    /* ref starts with REF_SEP, ignore it */
                    consumed = REF_SEP_LEN;
                    ref_start += REF_SEP_LEN;
                }
                else
                {
                    /* "empty" intermediate component => invalid ref */
                    goto done;
                }
            }
            else
            {
                consumed = (sep - ref_head) + REF_SEP_LEN;
                valid_component = RTI_TRUE;
            }
        } 
        else 
        {
            /* ref_head is a single component with no REF_SEP */
            consumed = ref_len - (ref_head - ref);
            valid_component = RTI_TRUE;
        }

        if (valid_component)
        {
            components_count += 1;
            ref_head += consumed;
            ch_i = (ref_head - ref);
        }

    } while (sep != NULL && ch_i < ref_len);


    *valid_ref_out = DDS_BOOLEAN_TRUE;
    *components_count_out = components_count;
    *ref_len_out = ref_len;
    *ref_start_out = ref_start;

done:

    return;
}

RTI_PRIVATE
RTIBool
D2S2_EntityName_parse_ref_components(
    const char *ref,
    char ***ref_components_out,
    DDS_UnsignedLong *const ref_components_size_out)
{
    RTIBool retcode = RTI_FALSE,
            valid_ref = RTI_FALSE;
    DDS_UnsignedLong ref_components_size = 0;
    DDS_UnsignedLong remaining = 0,
                     i = 0,
                     nxt_len = 0;
    char **ref_components = NULL;
    const char *nxt_end = NULL,
               *nxt_start = NULL,
               *ref_head = ref;
    
    *ref_components_out = NULL;
    *ref_components_size_out = 0;

    D2S2_EntityName_count_ref_components(
        ref, &valid_ref, &ref_components_size, &remaining, &i);
    
    if (!valid_ref)
    {
        goto done;
    }

    RTIOsapiHeap_allocateArray(&ref_components, ref_components_size, char *);
    if (ref_components == NULL)
    {
        goto done;
    }

    for (;i < ref_components_size && remaining > 0; i++)
    {
        nxt_start = ref_head;
        nxt_end = strstr(nxt_start, "::");
        
        if (nxt_end != NULL)
        {
            nxt_len = nxt_end - nxt_start;
            /* assert nxt_len > 0 */
            ref_head = nxt_end + REF_SEP_LEN;
            remaining -= nxt_len + REF_SEP_LEN;
        }
        else
        {
            nxt_len = remaining;
            ref_head = NULL;
            remaining = 0;
        }

        RTIOsapiHeap_allocateString(&(ref_components[i]),nxt_len);
        if (ref_components[i] == NULL)
        {
            goto done;
        }
        RTIOsapiMemory_copy(ref_components[i], nxt_start, nxt_len);
        ref_components[i][nxt_len] = '\0';
    }
    /* assert: i == ref_components_size_max */
    
    *ref_components_out = ref_components;
    *ref_components_size_out = ref_components_size;
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (ref_components != NULL)
        {
            D2S2_EntityName_free_ref_components(
                ref_components, ref_components_size);
            RTIOsapiHeap_free(ref_components);
        }
    }
    return retcode;

}


DDS_Boolean
D2S2_EntityName_from_id(
    D2S2_EntityName *const self,
    const D2S2_ResourceId *const id)
{
    DDS_Boolean retcode = DDS_BOOLEAN_FALSE;
    const char *ref = NULL;

    if (!D2S2_ResourceId_as_ref(id, &ref))
    {
        goto done;
    }

    if (!D2S2_EntityName_parse_ref_components(
            ref, &self->components, &self->depth))
    {
        goto done;
    }

    retcode = DDS_BOOLEAN_TRUE;
    
done:

    return retcode;
}

DDS_Boolean
D2S2_EntityName_to_ref(
    const D2S2_EntityName *const self,
    const DDS_UnsignedLong max_depth,
    char **const ref_out)
{
    DDS_Boolean retcode = DDS_BOOLEAN_FALSE;
    DDS_UnsignedLong ref_len = 0,
                     i = 0,
                     cmp_len = 0;
    char *ref = NULL,
         *ref_head = NULL;

    if (max_depth > self->depth)
    {
        goto done;
    }

    // printf("EXTRACT: %u/%u\n", max_depth, self->depth);
    // for (i = 0; i < self->depth; i++)
    // {
    //     printf("  component[%u]: %s\n", i, self->components[i]);
    // }
    

    for (i = 0; i < max_depth; i++)
    {
        if (i > 0)
        {
            ref_len += REF_SEP_LEN;
        }
        ref_len += strlen(self->components[i]);
    }

    // printf("  REF_LEN: %u\n",ref_len);

    RTIOsapiHeap_allocateString(&ref, ref_len);
    if (ref == NULL)
    {
        goto done;
    }

    ref_head = ref;

    for (i = 0; i < max_depth && (ref_head - ref) < ref_len; i++)
    {
        if (i > 0)
        {
            RTIOsapiMemory_copy(ref_head, REF_SEP, REF_SEP_LEN);
            ref_head += REF_SEP_LEN;
        }
        cmp_len = strlen(self->components[i]);
        RTIOsapiMemory_copy(ref_head, self->components[i], cmp_len);
        ref_head += cmp_len;

        // printf("[%u] COPIED: '%s' [remaining: %ld]\n",
        //     i, ref, ref_head - ref);
    }
    ref[ref_len] = '\0';
    
    *ref_out = ref;
    retcode = DDS_BOOLEAN_TRUE;
    
done:
    return retcode;
}

DDS_Boolean
D2S2_EntityName_component(
    const D2S2_EntityName *const self,
    const DDS_UnsignedLong depth,
    const char **const component_out)
{
    DDS_Boolean retcode = DDS_BOOLEAN_FALSE;

    *component_out = NULL;

    if (depth > self->depth)
    {
        goto done;
    }

    *component_out = self->components[depth - 1];

    retcode = DDS_BOOLEAN_TRUE;
    
done:
    return retcode;
}

void
D2S2_EntityName_finalize(D2S2_EntityName *const self)
{
    D2S2_EntityName_free_ref_components(self->components, self->depth);
    RTIOsapiHeap_free(self->components);
    self->components = NULL;
    self->depth = 0;
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */