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

#include "NddsAgentDb.h"

#if DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT

#define NDDSA_AgentDb_get_worker(s_) \
    DDS_DomainParticipantFactory_get_workerI((s_)->factory)

#define NDDSA_AGENTDB_EA_LVL_DEFAULT_DB                  9
#define NDDSA_AGENTDB_EA_LVL_DEFAULT_RECORD_RESOURCES    8
#define NDDSA_AGENTDB_EA_LVL_DEFAULT_TABLE_RESOURCES     7
#define NDDSA_AGENTDB_EA_LVL_DEFAULT_RECORD_SESSIONS     6
#define NDDSA_AGENTDB_EA_LVL_DEFAULT_TABLE_SESSIONS      5

#define NDDSA_AGENTDB_TABLE_NAME_RESOURCES       "d2s2-resources"
#define NDDSA_AGENTDB_TABLE_NAME_SESSIONS        "d2s2-sessions"

#define NDDSA_STATICRESOURCEID_REF_MAX_LENGTH       512

typedef struct NDDSA_StaticResourceIdI
{
    D2S2_ResourceId base;
    char ref_buffer[NDDSA_STATICRESOURCEID_REF_MAX_LENGTH + 1];
} NDDSA_StaticResourceId;

#define NDDSA_STATICRESOURCEID_INITIALIZER \
{\
    D2S2_RESOURCEIDKIND_NONE, /* base */\
    { 0 } /* ref_buffer */\
}

#define NDDSA_StaticResourceId_from_id(s_,id_,ok_) \
{\
    DDS_UnsignedLong ref_len_ = 0;\
    *(ok_) = RTI_TRUE;\
    (s_)->base = *(id_);\
    if ((id_)->kind == D2S2_RESOURCEIDKIND_REF)\
    {\
        ref_len_ = strlen((id_)->value.ref);\
        if (ref_len_ > NDDSA_STATICRESOURCEID_REF_MAX_LENGTH)\
        {\
            *(ok_) = RTI_FALSE;\
        }\
        else \
        {\
            memcpy(&(s_)->ref_buffer, (id_)->value.ref, ref_len_);\
            (s_)->ref_buffer[ref_len_] = '\0';\
            (s_)->base.value.ref = (char*) &(s_)->ref_buffer;\
        }\
    }\
}

RTI_PRIVATE
int
D2S2_ResourceId_compare_record(
    const void *left,
    const void *right)
{
    D2S2_ResourceId *l = (D2S2_ResourceId*)left,
                   *r = (D2S2_ResourceId*)right;
    
    switch (l->kind)
    {
        case D2S2_RESOURCEIDKIND_GUID:
        {
            switch (r->kind)
            {
                case D2S2_RESOURCEIDKIND_GUID:
                {
                    return RTIOsapiMemory_compare(
                            &l->value.guid,
                            &r->value.guid,
                            sizeof(l->value.guid));
                }
                default:
                {
                    return -1;
                }
            }
        }
        case D2S2_RESOURCEIDKIND_REF:
        {
            switch (r->kind)
            {
                case D2S2_RESOURCEIDKIND_REF:
                {
                    return REDAString_compare(l->value.ref, r->value.ref);
                }
                default:
                {
                    return 1;
                }
            }
            break;
        }
        default:
        {
            return 1;
        }
    }
}


RTI_PRIVATE
int
NDDSA_StaticResourceId_compare_record(
    const void *left,
    const void *right)
{
    NDDSA_StaticResourceId *l = (NDDSA_StaticResourceId*)left,
                   *r = (NDDSA_StaticResourceId*)right;
    
    if (l->base.kind == D2S2_RESOURCEIDKIND_REF)
    {
        l->base.value.ref = (char*)&l->ref_buffer;
    }
    if (r->base.kind == D2S2_RESOURCEIDKIND_REF)
    {
        r->base.value.ref = (char*)&r->ref_buffer;
    }

    return D2S2_ResourceId_compare_record(left, right);
}

RTI_PRIVATE
int
NDDSA_ResourceRecord_compare_record(
    const void *left,
    const void *right)
{
    int res = 0;
    NDDSA_ResourceRecord *l = (NDDSA_ResourceRecord*)left,
                        *r = (NDDSA_ResourceRecord*)right;
    res = D2S2_ResourceId_compare_record(
                &l->resource.base.id, &r->resource.base.id);
    
    return res;
}

RTI_PRIVATE
int
D2S2_ClientKey_compare_record(
    const void *left,
    const void *right)
{
    int res = 0;
    D2S2_ClientKey *l = (D2S2_ClientKey*)left,
                   *r = (D2S2_ClientKey*)right;
    unsigned int ul = 0, ur = 0;

    ul = *l;
    ur = *r;
    res = REDAOrderedDataType_compareUInt(&ul,&ur);
    return res;
}


RTI_PRIVATE
int
D2S2_ClientSessionKey_compare_record(
    const void *left,
    const void *right)
{
    D2S2_ClientSessionKey 
            *l = (D2S2_ClientSessionKey*)left,
            *r = (D2S2_ClientSessionKey*)right;
    int res = 0;
    unsigned int ul = 0,
                 ur = 0;

    res = D2S2_ClientKey_compare_record(&l->client,&r->client);
    if (res != 0)
    {
        return res;
    }

    ul = l->id;
    ur = r->id;

    res = REDAOrderedDataType_compareUInt(&ul,&ur);
    return res;
}

RTI_PRIVATE
int
NDDSA_ClientSessionRecord_compare_record(
    const void *left,
    const void *right)
{
    int res = 0;
    NDDSA_ClientSessionRecord *l = (NDDSA_ClientSessionRecord*)left,
                             *r = (NDDSA_ClientSessionRecord*)right;
    res = D2S2_ClientSessionKey_compare_record(
                &l->session.base.key, &r->session.base.key);
    return res;
}


void
NDDSA_AgentDb_finalize_session_record(
    void *param, 
    void *key,
    void *readOnlyArea,
    void *readWriteArea,
    struct REDAWorker *worker)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_finalize_session_record)
    NDDSA_AgentDb *const self = (NDDSA_AgentDb*)param;
    NDDSA_ClientSessionRecord *const session_rec =
        (NDDSA_ClientSessionRecord*)readWriteArea;
    D2S2_ClientSessionKey *const session_key =
        (D2S2_ClientSessionKey*)key;

    D2S2Log_fn_exit()

    UNUSED_ARG(session_key);
    UNUSED_ARG(readOnlyArea);
    UNUSED_ARG(worker);

    REDAWorkerFactory_destroyExclusiveArea(self->worker_factory, session_rec->ea);

    D2S2Log_fn_entry()
}

void
NDDSA_AgentDb_finalize_resource_record(
    void *param, 
    void *key,
    void *readOnlyArea,
    void *readWriteArea,
    struct REDAWorker *worker)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_finalize_resource_record)
    NDDSA_AgentDb *const self = (NDDSA_AgentDb*)param;
    NDDSA_ResourceRecord *const resource_rec =
        (NDDSA_ResourceRecord*)readWriteArea;
    D2S2_ResourceId *const resource_id = (D2S2_ResourceId*)key;

    D2S2Log_fn_entry()

    UNUSED_ARG(resource_id);
    UNUSED_ARG(readOnlyArea);
    UNUSED_ARG(worker);

    REDAWorkerFactory_destroyExclusiveArea(self->worker_factory, resource_rec->ea);

    D2S2Log_fn_exit()
}

RTI_PRIVATE
RTIBool
NDDSA_AgentDb_create_table(
    NDDSA_AgentDb *const self,
    const char *table_name,
    struct REDAWeakReference *table,
    struct REDACursorPerWorker **cursor_p_worker_ref,
    struct REDAOrderedDataType *const odt_key,
    struct REDAOrderedDataType *const odt_rw,
    struct REDAFastBufferPoolGrowthProperty *table_growth_user,
    struct REDAExclusiveArea *const table_ea,
    REDATableRecordFinalizeFunction finalize_record_fn)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_create_table)
    RTIBool retcode = RTI_FALSE;
    struct REDAFastBufferPoolGrowthProperty def_growth =
      REDA_FAST_BUFFER_POOL_GROWTH_PROPERTY_DEFAULT,
            *table_growth = NULL;
    struct REDAWorker *worker = NULL;
    
    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }
    
    if (table_growth_user == NULL)
    {
        table_growth = &def_growth;
    }
    else
    {
        table_growth = table_growth_user;
    }
    if (!REDADatabase_createTable(
            self->db,
            table,
            table_name,
            odt_key,
            NULL, /* ro_area */
            odt_rw,
            NULL, /* hash */
            table_ea,
            // self->ea_db,
            NULL, NULL, /* table finalize */
            finalize_record_fn, self, /* record finalize */
            table_growth,
            NULL, /* user data */
            worker))
    {
        goto done;
    }

    *cursor_p_worker_ref =
        REDADatabase_createCursorPerWorker(self->db, table);
    if (*cursor_p_worker_ref == NULL)
    {
        goto done;
    }

    retcode = RTI_TRUE;
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_AgentDb_delete_table(
    NDDSA_AgentDb *const self,
    struct REDAWeakReference *table,
    struct REDACursorPerWorker **cursor_p_worker_ref)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_delete_table)
    RTIBool retcode = RTI_FALSE,
            already_removed = RTI_FALSE;
    struct REDACursor *cursor = NULL;
    int fail_reason = 0;
    struct REDAWorker *worker = NULL;
    struct REDACursorPerWorker *cursor_p_worker = *cursor_p_worker_ref;

    D2S2Log_fn_entry()

    UNUSED_ARG(table);
    
    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
            cursor_p_worker, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    if (!REDACursor_start(cursor,&fail_reason))
    {
        goto done;
    }
    
    if (!REDACursor_lockTable(cursor, NULL /* fail reason */ )) 
    {
        REDACursor_finish(cursor);
        goto done;
    }

    if (!REDACursor_removeTable(
            cursor,
            &fail_reason, /* fail reason */
            &already_removed /* already removed */))
    {
        REDACursor_unlockTable(cursor);
        REDACursor_finish(cursor);
        goto done;
    }

    REDACursor_unlockTable(cursor);
    REDACursor_finish(cursor);
    
    REDADatabase_destroyCursorPerWorker(self->db, cursor_p_worker, worker);
    *cursor_p_worker_ref = NULL;

    retcode = RTI_TRUE;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_initialize(
    NDDSA_AgentDb *const self,
    NDDSA_Agent *const agent)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_initialize)
    RTIBool retcode = RTI_FALSE,
            initd_table_resources = RTI_FALSE,
            initd_table_sessions = RTI_FALSE,
            already_created = RTI_FALSE;
    struct REDAOrderedDataType odt_key;
    struct REDAOrderedDataType odt_rw;
    struct REDAWorker *worker = NULL;
    struct DDS_DomainParticipantFactoryQos dpf_qos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;

    D2S2Log_fn_entry()

    self->agent = agent;
    self->factory = DDS_DomainParticipantFactory_get_instance();
    if (self->factory == NULL)
    {
        goto done;
    }

    /* Initialize ParticipantFacotryQos */
    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_get_qos(
                self->factory, &dpf_qos))
    {
        goto done;
    }

    dpf_qos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_set_qos(
                self->factory, &dpf_qos))
    {
        goto done;
    }

    self->worker_factory =
        DDS_DomainParticipantFactory_get_worker_factoryI(
            self->factory, &already_created);
    if (self->worker_factory == NULL)
    {
        goto done;
    }

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    self->ea_db = 
        REDAWorkerFactory_createExclusiveArea(
            self->worker_factory, NDDSA_AGENTDB_EA_LVL_DEFAULT_DB);
    if (self->ea_db == NULL)
    {
        goto done;
    }

    self->db =
        REDADatabase_new(
            self->worker_factory, self->ea_db, NULL /* property */, worker);
    if (self->db == NULL)
    {
        goto done;
    }

    REDAOrderedDataType_defineFromStructure(
        &odt_key,
        D2S2_ResourceId,
        D2S2_ResourceId_compare_record,
        NULL);
    REDAOrderedDataType_defineFromStructure(
        &odt_key,
        NDDSA_StaticResourceId,
        NDDSA_StaticResourceId_compare_record,
        NULL);
        
    REDAOrderedDataType_defineFromStructure(
        &odt_rw, 
        NDDSA_ResourceRecord,
        NDDSA_ResourceRecord_compare_record,
        NULL);
    
    self->ea_table_resources =
        REDAWorkerFactory_createExclusiveArea(
            self->worker_factory,
            NDDSA_AGENTDB_EA_LVL_DEFAULT_TABLE_RESOURCES);
    if (self->ea_table_resources == NULL)
    {
        goto done;
    }

    if (!NDDSA_AgentDb_create_table(
            self,
            NDDSA_AGENTDB_TABLE_NAME_RESOURCES,
            &self->table_resources,
            &self->cursor_p_worker_resources,
            &odt_key,
            &odt_rw,
            NULL,
            self->ea_table_resources,
            NDDSA_AgentDb_finalize_resource_record))
    {
        goto done;
    }
    initd_table_resources = RTI_TRUE;

    REDAOrderedDataType_defineFromStructure(
        &odt_key,
        D2S2_ClientSessionKey,
        D2S2_ClientSessionKey_compare_record,
        NULL);
    REDAOrderedDataType_defineFromStructure(
        &odt_rw, 
        NDDSA_ClientSessionRecord,
        NDDSA_ClientSessionRecord_compare_record,
        NULL);
    
    self->ea_table_sessions =
        REDAWorkerFactory_createExclusiveArea(
            self->worker_factory,
            NDDSA_AGENTDB_EA_LVL_DEFAULT_TABLE_SESSIONS);
    if (self->ea_table_sessions == NULL)
    {
        goto done;
    }

    if (!NDDSA_AgentDb_create_table(
            self,
            NDDSA_AGENTDB_TABLE_NAME_SESSIONS,
            &self->table_sessions,
            &self->cursor_p_worker_sessions,
            &odt_key,
            &odt_rw,
            NULL,
            self->ea_table_sessions,
            NDDSA_AgentDb_finalize_session_record))
    {
        goto done;
    }
    initd_table_sessions = RTI_TRUE;

    retcode = RTI_TRUE;
done:
    if (!retcode)
    {
        if (initd_table_sessions)
        {
            if (self->cursor_p_worker_sessions != NULL)
            {
                if (!NDDSA_AgentDb_delete_table(
                        self,
                        &self->table_sessions,
                        &self->cursor_p_worker_sessions))
                {
                    /* TODO log */
                }
            }
            else
            {
                /* TODO log */
            }
        }

        if (initd_table_resources)
        {
            if (self->cursor_p_worker_resources != NULL)
            {
                if (!NDDSA_AgentDb_delete_table(
                        self,
                        &self->table_resources,
                        &self->cursor_p_worker_resources))
                {
                    /* TODO log */
                }
            }
            else
            {
                /* TODO log */
            }
        }

        if (self->db != NULL)
        {
            RTIBool can_delete = RTI_FALSE;

            if (!REDADatabase_cleanup(self->db, &can_delete, worker))
            {
                /* TODO log */
            }
            else if (!can_delete || !REDADatabase_delete(self->db, worker))
            {
                /* TODO log */
            }
            self->db = NULL;
        }
        
        if (self->ea_db != NULL)
        {
            REDAWorkerFactory_destroyExclusiveArea(
                self->worker_factory, self->ea_db);
            self->ea_db = NULL;
        }
    }
    DDS_DomainParticipantFactoryQos_finalize(&dpf_qos);
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_finalize(
    NDDSA_AgentDb *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_finalize)
    RTIBool retcode = RTI_FALSE,
            can_delete = RTI_FALSE;
    struct REDAWorker *worker = NULL;

    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_delete_table(
            self,
            &self->table_resources,
            &self->cursor_p_worker_resources))
    {
        goto done;
    }

    if (!NDDSA_AgentDb_delete_table(
            self,
            &self->table_sessions,
            &self->cursor_p_worker_sessions))
    {
        goto done;
    }

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    if (!REDADatabase_cleanup(self->db, &can_delete, worker))
    {
        goto done;
    }
    if (!can_delete || !REDADatabase_delete(self->db, worker))
    {
        goto done;
    }
    
    self->db = NULL;

    REDAWorkerFactory_destroyExclusiveArea(
        self->worker_factory, self->ea_table_resources);
    self->ea_table_resources = NULL;

    REDAWorkerFactory_destroyExclusiveArea(
        self->worker_factory, self->ea_table_sessions);
    self->ea_table_sessions = NULL;

    REDAWorkerFactory_destroyExclusiveArea(self->worker_factory, self->ea_db);
    self->ea_db = NULL;

    retcode = RTI_TRUE;
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_AgentDb_assert_record(
    NDDSA_AgentDb *const self,
    struct REDACursorPerWorker *const cursor_p_worker,
    void *key,
    int ea_lvl,
    const RTIBool lock_and_return,
    RTIBool *const new_record_out,
    void **const record_out,
    struct REDAExclusiveArea **const ea_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_assert_record)
    RTIBool retcode = RTI_FALSE,
            already_exists = RTI_FALSE,
            table_locked = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    struct REDAExclusiveArea *rec_ea = NULL;
    void *record = NULL;

    D2S2Log_fn_entry()
    
    *new_record_out = RTI_FALSE;

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    if (!REDACursorPerWorker_assertAndStartCursorSafe(
            cursor_p_worker,
            &cursor,
            &cursor_fail,
            cursor_stack, &cursor_stack_size,
            worker))
    {
        goto done;
    }
    if (cursor == NULL)
    {
        goto done;
    }

    if (!REDACursor_lockTable(cursor, &cursor_fail))
    {
        goto done;
    }
    table_locked = RTI_TRUE;

    if (!REDACursor_gotoKeyEqual(cursor, &cursor_fail, key))
    {
        *new_record_out = RTI_TRUE;

        /* A record for the specified key doesn't exists, so allocate a new
           EA for it and assert it in the table */
        rec_ea =
            REDAWorkerFactory_createExclusiveArea(self->worker_factory, ea_lvl);
        if (rec_ea == NULL)
        {
            goto done;
        }

        record =
            REDACursor_assertAndModifyReadWriteArea(
                cursor, 
                &cursor_fail,
                &already_exists,
                NULL /* weak_ref */,
                key,
                NULL,
                rec_ea);
        
        *ea_out = rec_ea;

        REDACursor_finishReadWriteArea(cursor);
    }

    table_locked = RTI_FALSE;
    REDACursor_unlockTable(cursor);

    if (lock_and_return)
    {
        /* Record exists by now and cursor is already positioned at it, 
          so just lock it and return it */
        record = REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
        if (record == NULL)
        {
            goto done;
        }
    }

    *record_out = record;

    retcode = RTI_TRUE;

done:
    if (table_locked)
    {
        REDACursor_unlockTable(cursor);
    }

    if (!lock_and_return || !retcode)
    {
        if (cursor_stack_size > 0)
        {
            REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
        }
    }
    if (!retcode)
    {
        if (rec_ea != NULL)
        {
            REDAWorkerFactory_destroyExclusiveArea(
                self->worker_factory, rec_ea);
        }
    }

    D2S2Log_fn_exit()
    return retcode;
}


RTI_PRIVATE
RTIBool
NDDSA_AgentDb_lookup_record(
    NDDSA_AgentDb *const self,
    struct REDACursorPerWorker *const cursor_p_worker,
    void *key,
    void **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_lookup_record)
    RTIBool retcode = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    void *record = NULL;

    D2S2Log_fn_entry()
    
    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    if (!REDACursorPerWorker_assertAndStartCursorSafe(
            cursor_p_worker,
            &cursor,
            &cursor_fail,
            cursor_stack, &cursor_stack_size,
            worker))
    {
        goto done;
    }
    if (cursor == NULL)
    {
        goto done;
    }
    
    if (REDACursor_gotoKeyEqual(cursor, &cursor_fail, key))
    {
        record = REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
        if (record == NULL)
        {
            goto done;
        }
    }
    
    *record_out = record;
    retcode = RTI_TRUE;

done:
    if ((!retcode || record == NULL) && cursor_stack_size > 0)
    {
        REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    }
    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_AgentDb_release_record(
    NDDSA_AgentDb *const self,
    struct REDACursorPerWorker *const cursor_p_worker)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_release_record)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(cursor_p_worker, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    REDACursor_finishReadWriteArea(cursor);

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    
    retcode = RTI_TRUE;
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_delete_record(
    NDDSA_AgentDb *const self,
    struct REDACursorPerWorker *const cursor_p_worker,
    const void *const key,
    RTIBool *const already_deleted_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_delete_record)
    RTIBool retcode = RTI_FALSE,
            table_locked = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    
    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }
    
    if (!REDACursorPerWorker_assertAndStartCursorSafe(
            cursor_p_worker,
            &cursor,
            &cursor_fail,
            cursor_stack, &cursor_stack_size,
            worker))
    {
        goto done;
    }
    if (cursor == NULL)
    {
        goto done;
    }

    if (!REDACursor_lockTable(cursor, NULL))
    {
        goto done;
    }
    table_locked = RTI_TRUE;

    if (!REDACursor_gotoKeyEqual(cursor, &cursor_fail, (void*)key))
    {
        goto done;
    }

    if (!REDACursor_removeRecord(cursor, &cursor_fail, already_deleted_out))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
done:

    if (table_locked)
    {
        REDACursor_unlockTable(cursor);
    }

    if (cursor_stack_size > 0)
    {
        REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    }

    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_AgentDb_lock_resources(
    NDDSA_AgentDb *const self,
    const RTIBool start_cursor,
    const RTIBool goto_top)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_lock_resources)
    RTIBool retcode = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;

    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }
    
    if (start_cursor)
    {
        if (!REDACursorPerWorker_assertAndStartCursorSafe(
                self->cursor_p_worker_resources,
                &cursor,
                &cursor_fail,
                cursor_stack, 
                &cursor_stack_size,
                worker))
        {
            goto done;
        }
    }
    else
    {
        REDACursorPerWorker_assertCursor(
            self->cursor_p_worker_resources, &cursor, worker);
        if (cursor == NULL)
        {
            goto done;
        }

        cursor_stack[0] = cursor;
        cursor_stack_size = 1;
    }

    if (cursor == NULL)
    {
        goto done;
    }

    if (!REDACursor_lockTable(cursor, NULL))
    {
        goto done;
    }

    if (goto_top)
    {
        REDACursor_gotoTop(cursor);
    }
    
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (start_cursor && cursor_stack_size > 0)
        {
            REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_unlock_resources(
    NDDSA_AgentDb *const self,
    const RTIBool finish_cursor)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_unlock_resources)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_unlockTable(cursor);

    if (finish_cursor)
    {
        REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    }
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}




RTIBool
NDDSA_AgentDb_lookup_resourceEA(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const resource_id,
    NDDSA_ResourceRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_lookup_resourceEA)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0,
        cursor_fail = 0;
    NDDSA_StaticResourceId sid = NDDSA_STATICRESOURCEID_INITIALIZER;

    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    *record_out = NULL;
    
    NDDSA_StaticResourceId_from_id(&sid, resource_id, &retcode);
    if (!retcode)
    {
        return RTI_FALSE;
    }
    retcode = RTI_FALSE;

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
            self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (REDACursor_gotoKeyEqual(cursor, &cursor_fail, &sid))
    {
        *record_out = 
            (NDDSA_ResourceRecord*)
                REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
        if (*record_out == NULL)
        {
            goto done;
        }
    }
    
    retcode = RTI_TRUE;
    
done:
    
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_find_next_resourceEA(
    NDDSA_AgentDb *const self,
    NDDSA_AgentDb_FilterResourceRecordFn filter_resource_fn,
    void *const param,
    NDDSA_ResourceRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_find_next_resourceEA)
    RTIBool retcode = RTI_FALSE,
            return_record = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0,
        cursor_fail = 0;
    
    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    *record_out = NULL;

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    while (*record_out == NULL && REDACursor_gotoNext(cursor))
    {
        *record_out = REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
        if (*record_out == NULL)
        {
            goto done;
        }

        if (filter_resource_fn != NULL)
        {
            if(!filter_resource_fn(self, *record_out, param, &return_record))
            {
                goto done;
            }
        }
        else
        {
            return_record = RTI_TRUE;
        }

        if (!return_record)
        {
            REDACursor_finishReadWriteArea(cursor);
            *record_out = NULL;
        }
    }
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (*record_out != NULL)
        {
            REDACursor_finishReadWriteArea(cursor);
            *record_out = NULL;
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_release_resourceEA(
    NDDSA_AgentDb *const self,
    NDDSA_ResourceRecord *const record)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_release_resourceEA)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    UNUSED_ARG(record);
    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_finishReadWriteArea(cursor);

    
    retcode = RTI_TRUE;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_insert_resourceEA(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const id,
    NDDSA_ResourceRecord **const record_out,
    RTIBool *const already_exists_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_insert_resourceEA)
    RTIBool retcode = RTI_FALSE,
            already_exists = RTI_FALSE;
    NDDSA_ResourceRecord *record = NULL;
    const NDDSA_ResourceRecord def_record = NDDSA_RESOURCERECORD_INITIALIZER;
    NDDSA_StaticResourceId sid = NDDSA_STATICRESOURCEID_INITIALIZER;
    struct REDAExclusiveArea *ea = NULL;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    
    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    *already_exists_out = RTI_FALSE;

    NDDSA_StaticResourceId_from_id(&sid, id, &retcode);
    if (!retcode)
    {
        return RTI_FALSE;
    }
    retcode = RTI_FALSE;

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (REDACursor_gotoKeyEqual(cursor, &cursor_fail, &sid))
    {
        *already_exists_out = RTI_TRUE;
        
        record = (NDDSA_ResourceRecord*)
                REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
        if (record == NULL)
        {
            goto done;
        }
    }
    else
    {
        /* A record for the specified key doesn't exists, so allocate a new
            EA for it and assert it in the table */
        ea = REDAWorkerFactory_createExclusiveArea(
                self->worker_factory,
                NDDSA_AGENTDB_EA_LVL_DEFAULT_RECORD_RESOURCES);
        if (ea == NULL)
        {
            goto done;
        }
        record = (NDDSA_ResourceRecord*)
            REDACursor_assertAndModifyReadWriteArea(
                cursor, 
                &cursor_fail,
                &already_exists,
                NULL /* weak_ref */,
                &sid,
                NULL,
                ea);
        if (record == NULL)
        {
            goto done;
        }
        *record = def_record;
        record->ea = ea;
        record->resource.agent = self->agent;
    }

    *record_out = record;
    
    retcode = RTI_TRUE;

done:
    
    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_AgentDb_delete_resourceEA(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const record_id)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_delete_resourceEA)
    RTIBool retcode = RTI_FALSE,
            already_deleted = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    NDDSA_StaticResourceId sid = NDDSA_STATICRESOURCEID_INITIALIZER;

    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    NDDSA_StaticResourceId_from_id(&sid, record_id, &retcode);
    if (!retcode)
    {
        return RTI_FALSE;
    }
    retcode = RTI_FALSE;

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (!REDACursor_gotoKeyEqual(cursor, &cursor_fail, &sid))
    {
        goto done;
    }

    if (!REDACursor_removeRecord(
            cursor, &cursor_fail, &already_deleted))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;

done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_lookup_resource(
    NDDSA_AgentDb *const self,
    const D2S2_ResourceId *const id,
    NDDSA_ResourceRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_lookup_resource)
    RTIBool retcode = RTI_FALSE;
    NDDSA_StaticResourceId sid = NDDSA_STATICRESOURCEID_INITIALIZER;
    
    D2S2Log_fn_entry()
    
    NDDSA_StaticResourceId_from_id(&sid, id, &retcode);
    if (!retcode)
    {
        return RTI_FALSE;
    }

    if (!NDDSA_AgentDb_lookup_record(
                self,
                self->cursor_p_worker_resources,
                (void*)&sid,
                (void**)record_out))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_release_resource(
    NDDSA_AgentDb *const self,
    NDDSA_ResourceRecord *const record)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_release_resource)
    DDS_ReturnCode_t rc = DDS_RETCODE_ERROR;
    D2S2Log_fn_entry()

    UNUSED_ARG(record);

    rc = NDDSA_AgentDb_release_record(
                self,
                self->cursor_p_worker_resources);
    D2S2Log_fn_exit()
    return rc;
}

RTIBool
NDDSA_AgentDb_iterate_resourcesEA(NDDSA_AgentDb *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_iterate_resourcesEA)
    RTIBool retcode = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }
    
    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (cursor == NULL)
    {
        goto done;
    }

    REDACursor_gotoTop(cursor);
    
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {

    }
    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_AgentDb_finish_iterate_resourcesEA(NDDSA_AgentDb *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_finish_iterate_resourcesEA)
    RTIBool retcode = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_resources, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    
    retcode = RTI_TRUE;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_insert_session(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key,
    RTIBool *const new_record_out,
    NDDSA_ClientSessionRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_insert_session)
    RTIBool retcode = RTI_FALSE;
    NDDSA_ClientSessionRecord *record = NULL,
                    def_record = NDDSA_CLIENTSESSIONRECORD_INITIALIZER;
    struct REDAExclusiveArea *ea = NULL;

    D2S2Log_fn_entry()
    
    if (!NDDSA_AgentDb_assert_record(
                self,
                self->cursor_p_worker_sessions,
                (void*)key,
                NDDSA_AGENTDB_EA_LVL_DEFAULT_RECORD_SESSIONS,
                RTI_TRUE/* lock_and_return */,
                new_record_out,
                (void**)&record,
                &ea))
    {
        goto done;
    }
    
    if (*new_record_out)
    {
        *record = def_record;
        record->ea = ea;
    }
    
    *record_out = record;
    retcode = RTI_TRUE;

done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_lookup_session(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key,
    NDDSA_ClientSessionRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_lookup_session)
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    D2S2Log_fn_entry()
    
    retcode = NDDSA_AgentDb_lookup_record(
                self,
                self->cursor_p_worker_sessions,
                (void*)key,
                (void**)record_out);

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_release_session(
    NDDSA_AgentDb *const self,
    NDDSA_ClientSessionRecord *const record)
{
    UNUSED_ARG(record);
    return NDDSA_AgentDb_release_record(
                self,
                self->cursor_p_worker_sessions);
}

RTIBool
NDDSA_AgentDb_delete_session(
    NDDSA_AgentDb *const self,
    NDDSA_ClientSessionRecord *const record)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_delete_session)
    RTIBool retcode = RTI_FALSE,
            table_locked = RTI_FALSE,
            already_deleted = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    
    D2S2Log_fn_entry()

    UNUSED_ARG(record);

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }
    
    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_sessions, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_finishReadWriteArea(cursor);
    
    if (!REDACursor_lockTable(cursor, NULL))
    {
        goto done;
    }
    table_locked = RTI_TRUE;

    if (!REDACursor_removeRecord(cursor, &cursor_fail, &already_deleted))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
done:

    if (table_locked)
    {
        REDACursor_unlockTable(cursor);
    }

    if (cursor_stack_size > 0)
    {
        REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    }

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_find_next_record(
    NDDSA_AgentDb *const self,
    struct REDACursorPerWorker *const cursor_p_worker,
    NDDSA_AgentDb_FilterRecordFn filter_record_fn,
    void *const filter_param,
    void *const prev,
    void **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_find_next_record)
    RTIBool retcode = RTI_FALSE,
            has_next = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    void *record = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);
    
    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(cursor_p_worker, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (prev != NULL)
    {
        REDACursor_finishReadWriteArea(cursor);
    }

    do
    {
        has_next = REDACursor_gotoNext(cursor);

        if (has_next)
        {
            record = REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
            if (record == NULL)
            {
                goto done;
            }

            if (filter_record_fn != NULL &&
                !filter_record_fn(self, record, filter_param))
            {
                REDACursor_finishReadWriteArea(cursor);
                record = NULL;
            }
        }
    } while (has_next && record == NULL);

    if (record != NULL)
    {
        *record_out = record;
    }
    
    retcode = RTI_TRUE;

done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_iterate_sessions(NDDSA_AgentDb *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_iterate_sessions)
    RTIBool retcode = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    
    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }
    
    if (!REDACursorPerWorker_assertAndStartCursorSafe(
            self->cursor_p_worker_sessions,
            &cursor,
            &cursor_fail,
            cursor_stack, 
            &cursor_stack_size,
            worker))
    {
        goto done;
    }
    if (cursor == NULL)
    {
        goto done;
    }

    REDACursor_gotoTop(cursor);
    
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (cursor_stack_size > 0)
        {
            REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_finish_iterate_sessions(NDDSA_AgentDb *const self)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_finish_iterate_sessions)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_sessions, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_find_next_session(
    NDDSA_AgentDb *const self,
    NDDSA_AgentDb_FilterRecordFn filter_record_fn,
    void *const filter_param,
    NDDSA_ClientSessionRecord *const prev_session,
    NDDSA_ClientSessionRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_find_next_session)
    RTIBool retcode = RTI_FALSE;
    void *record = NULL;

    D2S2Log_fn_entry()

    if (!NDDSA_AgentDb_find_next_record(
                self,
                self->cursor_p_worker_sessions,
                filter_record_fn,
                filter_param,
                prev_session,
                &record))
    {
        goto done;
    }

    *record_out = (NDDSA_ClientSessionRecord*)record;
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_find_next_session_and_delete_previous(
    NDDSA_AgentDb *const self,
    void *const filter_param,
    NDDSA_ClientSessionRecord *const prev_session,
    NDDSA_ClientSessionRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_find_next_session_and_delete_previous)
    RTIBool retcode = RTI_FALSE,
            has_next = RTI_FALSE,
            already_deleted = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    void *record = NULL;

    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    *record_out = NULL;
    
    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_sessions, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (prev_session != NULL)
    {
        REDACursor_finishReadWriteArea(cursor);
        if (!REDACursor_removeRecord(
                cursor, &cursor_fail, &already_deleted))
        {
            goto done;
        }
    }

    do
    {
        has_next = REDACursor_gotoNext(cursor);

        if (has_next)
        {
            record = REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
            if (record == NULL)
            {
                goto done;
            }
        }
    } while (has_next && record == NULL);
    
    if (record != NULL)
    {
        *record_out = (NDDSA_ClientSessionRecord*)record;
    }

    retcode = RTI_TRUE;

done:
    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_AgentDb_lock_sessions(
    NDDSA_AgentDb *const self,
    const RTIBool start_cursor,
    const RTIBool goto_top)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_lock_sessions)
    RTIBool retcode = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    
    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }
    
    if (start_cursor)
    {
        if (!REDACursorPerWorker_assertAndStartCursorSafe(
                self->cursor_p_worker_sessions,
                &cursor,
                &cursor_fail,
                cursor_stack, 
                &cursor_stack_size,
                worker))
        {
            goto done;
        }
        if (cursor == NULL)
        {
            goto done;
        }

    }
    else
    {
        REDACursorPerWorker_assertCursor(
            self->cursor_p_worker_sessions, &cursor, worker);
        if (cursor == NULL)
        {
            goto done;
        }

        cursor_stack[0] = cursor;
        cursor_stack_size = 1;
    }

    if (!REDACursor_lockTable(cursor, NULL))
    {
        goto done;
    }

    if (goto_top)
    {
        REDACursor_gotoTop(cursor);
    }
    
    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (start_cursor && cursor_stack_size > 0)
        {
            REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
        }
    }
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_unlock_sessions(
    NDDSA_AgentDb *const self,
    const RTIBool finish_cursor)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_unlock_sessions)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_sessions, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_unlockTable(cursor);

    if (finish_cursor)
    {
    
        REDACursor_finishSafe(cursor_stack, &cursor_stack_size);
    }
    
    retcode = RTI_TRUE;
    
done:

    D2S2Log_fn_exit()
    return retcode;
}


RTIBool
NDDSA_AgentDb_lookup_sessionEA(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key,
    NDDSA_ClientSessionRecord **const record_out)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_lookup_sessionEA)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0,
        cursor_fail = 0;
    
    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    *record_out = NULL;

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_sessions, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (REDACursor_gotoKeyEqual(cursor, &cursor_fail, (void*)key))
    {
        *record_out = 
            (NDDSA_ClientSessionRecord*)
                REDACursor_modifyReadWriteArea(cursor, &cursor_fail);
        if (*record_out == NULL)
        {
            goto done;
        }
    }
    
    retcode = RTI_TRUE;
    
done:
    
    D2S2Log_fn_exit()
    return retcode;
}



RTIBool
NDDSA_AgentDb_release_sessionEA(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_release_sessionEA)
    RTIBool retcode = RTI_FALSE;
    struct REDACursor *cursor = NULL,
                      *cursor_stack[1] = { NULL };
    struct REDAWorker *worker = NULL;
    int cursor_stack_size = 0;

    D2S2Log_fn_entry()

    UNUSED_ARG(key);

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_sessions, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    REDACursor_finishReadWriteArea(cursor);

    retcode = RTI_TRUE;
    
done:
    D2S2Log_fn_exit()
    return retcode;
}

RTIBool
NDDSA_AgentDb_delete_sessionEA(
    NDDSA_AgentDb *const self,
    const D2S2_ClientSessionKey *const key)
{
    D2S2Log_METHOD_NAME(NDDSA_AgentDb_delete_sessionEA)
    RTIBool retcode = RTI_FALSE,
            already_deleted = RTI_FALSE;
    struct REDAWorker *worker = NULL;
    struct REDACursor *cursor_stack[1] = { NULL },
                      *cursor = NULL;
    int cursor_fail = 0,
        cursor_stack_size = 0;
    
    D2S2Log_fn_entry()

    UNUSED_ARG(cursor_stack);
    UNUSED_ARG(cursor_stack_size);

    worker = NDDSA_AgentDb_get_worker(self);
    if (worker == NULL)
    {
        goto done;
    }

    REDACursorPerWorker_assertCursor(
        self->cursor_p_worker_sessions, &cursor, worker);
    if (cursor == NULL)
    {
        goto done;
    }

    cursor_stack[0] = cursor;
    cursor_stack_size = 1;

    if (!REDACursor_gotoKeyEqual(cursor, &cursor_fail, (void*)key))
    {
        goto done;
    }

    if (!REDACursor_removeRecord(
            cursor, &cursor_fail, &already_deleted))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;

done:
    
    D2S2Log_fn_exit()
    return retcode;
}

#endif /* DDS_AGENT_DDSAPI == DDS_AGENT_DDSAPI_CONNEXT */
