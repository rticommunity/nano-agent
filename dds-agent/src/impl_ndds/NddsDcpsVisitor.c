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

#include "NddsDcpsVisitor.h"

#if 0
RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_topic(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor,
    DDS_DomainParticipantFactory *factory,
    DDS_DomainParticipant *const participant,
    DDS_Topic *const topic)
{
    RTIBool retcode = RTI_FALSE;
    
    UNUSED_ARG(participant);
    
    NDDSA_DcpsVisitor_on_topic(visitor, self, factory, topic);

    retcode = RTI_TRUE;
    
    if (!retcode)
    {
        NDDSA_DcpsVisitor_on_error(visitor, self, factory);
    }
    return retcode;
}

#endif

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_datareader(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor,
    DDS_DomainParticipantFactory *factory,
    DDS_DomainParticipant *const participant,
    DDS_Subscriber *const subscriber,
    DDS_DataReader *const reader)
{
    RTIBool retcode = RTI_FALSE,
            visit_err = RTI_FALSE;
    DDS_TopicDescription *topic_desc = NULL;
    DDS_Topic *topic = NULL;

    UNUSED_ARG(participant);
    UNUSED_ARG(subscriber);

    topic_desc = DDS_DataReader_get_topicdescription(reader);
    if (topic_desc == NULL)
    {
        goto done;
    }
    topic = DDS_Topic_narrow(topic_desc);
    if (topic == NULL)
    {
        goto done;
    }
    
    printf("[VISIT][DATAREADER] visitor=%p, datareader=%p\n", visitor, reader);
    if (!NDDSA_DcpsVisitor_on_datareader(visitor, self, factory, reader))
    {
        visit_err = RTI_TRUE;
        goto done;
    }
    printf("[VISIT][TOPIC] visitor=%p, topic=%p\n", visitor, topic);
    if (!NDDSA_DcpsVisitor_on_topic(visitor, self, factory, topic))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (!visit_err)
        {
            NDDSA_DcpsVisitor_on_error(visitor, self, factory);
        }
    }
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_datawriter(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor,
    DDS_DomainParticipantFactory *factory,
    DDS_DomainParticipant *const participant,
    DDS_Publisher *const publisher,
    DDS_DataWriter *const writer)
{
    RTIBool retcode = RTI_FALSE,
            visit_err = RTI_FALSE;
    DDS_Topic *topic = NULL;

    UNUSED_ARG(participant);
    UNUSED_ARG(publisher);

    topic = DDS_DataWriter_get_topic(writer);
    if (topic == NULL)
    {
        goto done;
    }

    printf("[VISIT][DATAWRITER] visitor=%p, datawriter=%p\n", visitor, writer);
    if (!NDDSA_DcpsVisitor_on_datawriter(visitor, self, factory, writer))
    {
        visit_err = RTI_TRUE;
        goto done;
    }
    printf("[VISIT][TOPIC] visitor=%p, topic=%p\n", visitor, topic);
    if (!NDDSA_DcpsVisitor_on_topic(visitor, self, factory, topic))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    if (!retcode)
    {
        if (!visit_err)
        {
            NDDSA_DcpsVisitor_on_error(visitor, self, factory);
        }
    }
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_subscriber(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor,
    DDS_DomainParticipantFactory *factory,
    DDS_DomainParticipant *const participant,
    DDS_Subscriber *const subscriber)
{
    RTIBool retcode = RTI_FALSE,
            visit_err = RTI_FALSE;
    struct DDS_DataReaderSeq readers = DDS_SEQUENCE_INITIALIZER;
    DDS_UnsignedLong seq_len = 0,
                     i = 0;

    printf("[VISIT][START][SUBSCRIBER] visitor=%p, subscriber=%p\n", visitor, subscriber);
    if (!NDDSA_DcpsVisitor_on_subscriber(
            visitor, self, factory, subscriber, RTI_FALSE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_Subscriber_get_all_datareaders(subscriber, &readers))
    {
        goto done;
    }

    seq_len = DDS_DataReaderSeq_get_length(&readers);

    for (i = 0; i < seq_len; i++)
    {
        DDS_DataReader *dr = *DDS_DataReaderSeq_get_reference(&readers, i);
        
        if (!NDDSA_Agent_visit_datareader(
                self, visitor, factory, participant, subscriber, dr))
        {
            goto done;
        }
    }

    printf("[VISIT][END][SUBSCRIBER] visitor=%p, subscriber=%p\n", visitor, subscriber);
    if (!NDDSA_DcpsVisitor_on_subscriber(
            visitor, self, factory, subscriber, RTI_TRUE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    DDS_DataReaderSeq_finalize(&readers);
    if (!retcode)
    {
        if (!visit_err)
        {
            NDDSA_DcpsVisitor_on_error(visitor, self, factory);
        }
    }
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_publisher(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor,
    DDS_DomainParticipantFactory *factory,
    DDS_DomainParticipant *const participant,
    DDS_Publisher *const publisher)
{
    RTIBool retcode = RTI_FALSE,
            visit_err = RTI_FALSE;
    struct DDS_DataWriterSeq writers = DDS_SEQUENCE_INITIALIZER;
    DDS_UnsignedLong seq_len = 0,
                     i = 0;

    printf("[VISIT][START][PUBLISHER] visitor=%p, publisher=%p\n", visitor, publisher);
    if (!NDDSA_DcpsVisitor_on_publisher(
            visitor, self, factory, publisher, RTI_FALSE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_Publisher_get_all_datawriters(publisher, &writers))
    {
        goto done;
    }

    seq_len = DDS_DataWriterSeq_get_length(&writers);

    for (i = 0; i < seq_len; i++)
    {
        DDS_DataWriter *dw = *DDS_DataWriterSeq_get_reference(&writers, i);
        
        if (!NDDSA_Agent_visit_datawriter(
                self, visitor, factory, participant, publisher, dw))
        {
            goto done;
        }
    }

    printf("[VISIT][END][PUBLISHER] visitor=%p, publisher=%p\n", visitor, publisher);
    if (!NDDSA_DcpsVisitor_on_publisher(
            visitor, self, factory, publisher, RTI_TRUE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    retcode = RTI_TRUE;
    
done:
    DDS_DataWriterSeq_finalize(&writers);
    if (!retcode)
    {
        if (!visit_err)
        {
            NDDSA_DcpsVisitor_on_error(visitor, self, factory);
        }
    }
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_domain_participant(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor,
    DDS_DomainParticipantFactory *factory,
    DDS_DomainParticipant *const participant)
{
    RTIBool retcode = RTI_FALSE,
            visit_err = RTI_FALSE;
    struct DDS_SubscriberSeq subs = DDS_SEQUENCE_INITIALIZER;
    struct DDS_PublisherSeq pubs = DDS_SEQUENCE_INITIALIZER;
    DDS_UnsignedLong seq_len = 0,
                     i = 0;
    
    printf("[VISIT][START][PARTICIPANT] visitor=%p, participant=%p\n", visitor, participant);
    if (!NDDSA_DcpsVisitor_on_participant(
            visitor, self, factory, participant, RTI_FALSE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipant_get_publishers(participant, &pubs))
    {
        goto done;
    }

    seq_len = DDS_PublisherSeq_get_length(&pubs);

    for (i = 0; i < seq_len; i++)
    {
        DDS_Publisher *pub = *DDS_PublisherSeq_get_reference(&pubs, i);
        
        if (!NDDSA_Agent_visit_publisher(
                self, visitor, factory, participant, pub))
        {
            goto done;
        }
    }

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipant_get_subscribers(participant, &subs))
    {
        goto done;
    }

    seq_len = DDS_SubscriberSeq_get_length(&subs);

    for (i = 0; i < seq_len; i++)
    {
        DDS_Subscriber *sub = *DDS_SubscriberSeq_get_reference(&subs, i);
        
        if (!NDDSA_Agent_visit_subscriber(
                self, visitor, factory, participant, sub))
        {
            goto done;
        }
    }

    printf("[VISIT][END][PARTICIPANT] visitor=%p, participant=%p\n", visitor, participant);
    if (!NDDSA_DcpsVisitor_on_participant(
            visitor, self, factory, participant, RTI_TRUE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    DDS_PublisherSeq_finalize(&pubs);
    DDS_SubscriberSeq_finalize(&subs);
    if (!retcode)
    {
        if (!visit_err)
        {
            NDDSA_DcpsVisitor_on_error(visitor, self, factory);
        }
    }
    return retcode;
}

RTI_PRIVATE
RTIBool
NDDSA_Agent_visit_domain_participant_factory(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor,
    DDS_DomainParticipantFactory *factory)
{
    RTIBool retcode = RTI_FALSE,
            visit_err = RTI_FALSE;
    struct DDS_DomainParticipantSeq participants = DDS_SEQUENCE_INITIALIZER;
    DDS_UnsignedLong seq_len = 0,
                     i = 0;

    printf("[VISIT][START][FACTORY] visitor=%p, factory=%p\n", visitor, factory);
    if (!NDDSA_DcpsVisitor_on_participant_factory(
            visitor, self, factory, RTI_FALSE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }

    if (DDS_RETCODE_OK !=
            DDS_DomainParticipantFactory_get_participants(
                factory, &participants))
    {
        goto done;
    }

    seq_len = DDS_DomainParticipantSeq_get_length(&participants);

    for (i = 0; i < seq_len; i++)
    {
        DDS_DomainParticipant *dp = 
            *DDS_DomainParticipantSeq_get_reference(&participants, i);
        
        if (!NDDSA_Agent_visit_domain_participant(self, visitor, factory, dp))
        {
            goto done;
        }
    }
    
    printf("[VISIT][END][FACTORY] visitor=%p, factory=%p\n", visitor, factory);
    if (!NDDSA_DcpsVisitor_on_participant_factory(
            visitor, self, factory, RTI_TRUE))
    {
        visit_err = RTI_TRUE;
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    DDS_DomainParticipantSeq_finalize(&participants);
    if (!retcode)
    {
        if (!visit_err)
        {
            NDDSA_DcpsVisitor_on_error(visitor, self, factory);
        }
    }
    return retcode;
}

RTIBool
NDDSA_Agent_visit_dcps(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor)
{
    RTIBool retcode = RTI_FALSE;
    DDS_DomainParticipantFactory *factory = NULL;

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory == NULL)
    {
        /* TODO log */
        goto done;
    }

    printf("[VISIT][START] visitor=%p, factory=%p\n", visitor, factory);
    if (!NDDSA_Agent_visit_domain_participant_factory(self, visitor, factory))
    {
        goto done;
    }
    
    retcode = RTI_TRUE;
    
done:
    printf("[VISIT][END] visitor=%p, factory=%p, ok=%d\n", visitor, factory, retcode);
    return retcode;
}