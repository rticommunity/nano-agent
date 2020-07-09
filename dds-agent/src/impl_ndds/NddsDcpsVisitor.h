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

#ifndef NddsDcpsIterator_h
#define NddsDcpsIterator_h

#include "NddsInfrastructure.h"

typedef RTIBool
    (*NDDSA_DcpsVisitor_OnParticipantFactoryFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory,
        RTIBool end);

typedef RTIBool
    (*NDDSA_DcpsVisitor_OnParticipantFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory,
        DDS_DomainParticipant *const participant,
        RTIBool end);

typedef RTIBool
    (*NDDSA_DcpsVisitor_OnPublisherFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory,
        DDS_Publisher *const publisher,
        RTIBool end);

typedef RTIBool
    (*NDDSA_DcpsVisitor_OnSubscriberFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory,
        DDS_Subscriber *const subscriber,
        RTIBool end);

typedef RTIBool
    (*NDDSA_DcpsVisitor_OnTopicFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory,
        DDS_Topic *const topic);

typedef RTIBool
    (*NDDSA_DcpsVisitor_OnDataWriterFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory,
        DDS_DataWriter *const writer);

typedef RTIBool
    (*NDDSA_DcpsVisitor_OnDataReaderFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory,
        DDS_DataReader *const reader);


typedef void
    (*NDDSA_DcpsVisitor_OnErrorFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        DDS_DomainParticipantFactory *const factory);

typedef struct NDDSA_DcpsVisitorI
{
    NDDSA_DcpsVisitor_OnParticipantFactoryFn on_participant_factory;
    NDDSA_DcpsVisitor_OnParticipantFn on_participant;
    NDDSA_DcpsVisitor_OnPublisherFn on_publisher;
    NDDSA_DcpsVisitor_OnSubscriberFn on_subscriber;
    NDDSA_DcpsVisitor_OnTopicFn on_topic;
    NDDSA_DcpsVisitor_OnDataWriterFn on_datawriter;
    NDDSA_DcpsVisitor_OnDataReaderFn on_datareader;
    NDDSA_DcpsVisitor_OnErrorFn on_error;
    void *user_data;
} NDDSA_DcpsVisitor;

#define NDDSA_DCPSVISITOR_INITIALIZER \
{\
    NULL, /* on_participant_factory */\
    NULL, /* on_participant */\
    NULL, /* on_publisher */\
    NULL, /* on_subscriber */\
    NULL, /* on_topic */\
    NULL, /* on_datawriter */\
    NULL,  /* on_datareader */\
    NULL,  /* on_error */\
    NULL /* user_data */ \
}

RTIBool
NDDSA_DcpsVisitor_on_participant_factory(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory,
    RTIBool end);

#define NDDSA_DcpsVisitor_on_participant_factory(s_,a_,f_,e_) \
    (s_)->on_participant_factory((s_)->user_data,(a_),(f_),(e_))

RTIBool
NDDSA_DcpsVisitor_on_participant(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory,
    DDS_DomainParticipant *const participant,
    RTIBool end);

#define NDDSA_DcpsVisitor_on_participant(s_,a_,f_,p_,e_) \
    (s_)->on_participant((s_)->user_data,(a_),(f_),(p_),(e_))

RTIBool
NDDSA_DcpsVisitor_on_publisher(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory,
    DDS_Publisher *const publisher,
    RTIBool end);

#define NDDSA_DcpsVisitor_on_publisher(s_,a_,f_,p_,e_) \
    (s_)->on_publisher((s_)->user_data,(a_),(f_),(p_),(e_))

RTIBool
NDDSA_DcpsVisitor_on_subscriber(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory,
    DDS_Subscriber *const subscriber,
    RTIBool end);

#define NDDSA_DcpsVisitor_on_subscriber(s_,a_,f_,sb_,e_) \
    (s_)->on_subscriber((s_)->user_data,(a_),(f_),(sb_),(e_))

RTIBool
NDDSA_DcpsVisitor_on_topic(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory,
    DDS_Topic *const topic);


#define NDDSA_DcpsVisitor_on_topic(s_,a_,f_,t_) \
    (s_)->on_topic((s_)->user_data,(a_),(f_),(t_))

RTIBool
NDDSA_DcpsVisitor_on_datawriter(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory,
    DDS_DataWriter *const writer);

#define NDDSA_DcpsVisitor_on_datawriter(s_,a_,f_,w_) \
    (s_)->on_datawriter((s_)->user_data,(a_),(f_),(w_))

RTIBool
NDDSA_DcpsVisitor_on_datareader(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory,
    DDS_DataReader *const reader);

#define NDDSA_DcpsVisitor_on_datareader(s_,a_,f_,r_) \
    (s_)->on_datareader((s_)->user_data,(a_),(f_),(r_))

void
NDDSA_DcpsVisitor_on_error(
    NDDSA_DcpsVisitor *const self,
    NDDSA_Agent *const agent,
    DDS_DomainParticipantFactory *const factory);

#define NDDSA_DcpsVisitor_on_error(s_,a_,f_) \
    (s_)->on_error((s_)->user_data,(a_),(f_))

RTIBool
NDDSA_Agent_visit_dcps(
    NDDSA_Agent *const self,
    NDDSA_DcpsVisitor *const visitor);

#endif /* NddsDcpsIterator_h */