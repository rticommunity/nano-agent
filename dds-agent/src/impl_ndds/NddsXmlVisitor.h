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

#ifndef NddsXmlVisitor_h
#define NddsXmlVisitor_h

#include "NddsInfrastructure.h"

typedef RTIBool
    (*NDDSA_XmlVisitor_OnXmlObjectFn)(
        void *const visitor,
        NDDSA_Agent *const agent,
        struct DDS_XMLObject *xml_obj);

typedef struct NDDSA_XmlVisitorI
{
    NDDSA_XmlVisitor_OnXmlObjectFn on_participant_factory;
    NDDSA_XmlVisitor_OnXmlObjectFn on_participant;
    NDDSA_XmlVisitor_OnXmlObjectFn on_publisher;
    NDDSA_XmlVisitor_OnXmlObjectFn on_subscriber;
    NDDSA_XmlVisitor_OnXmlObjectFn on_topic;
    NDDSA_XmlVisitor_OnXmlObjectFn on_datawriter;
    NDDSA_XmlVisitor_OnXmlObjectFn on_datareader;
    NDDSA_XmlVisitor_OnXmlObjectFn on_error;
    void *user_data;
} NDDSA_XmlVisitor;

#define NDDSA_XMLVISITOR_INITIALIZER \
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
NDDSA_XmlVisitor_on_participant_factory(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_participant_factory(s_,a_,x_) \
    (s_)->on_participant_factory((s_)->user_data,(a_),(x_))

RTIBool
NDDSA_XmlVisitor_on_participant(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_participant(s_,a_,x_) \
    (s_)->on_participant((s_)->user_data,(a_),(x_))

RTIBool
NDDSA_XmlVisitor_on_publisher(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_publisher(s_,a_,x_) \
    (s_)->on_publisher((s_)->user_data,(a_),(x_))

RTIBool
NDDSA_XmlVisitor_on_subscriber(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_subscriber(s_,a_,x_) \
    (s_)->on_subscriber((s_)->user_data,(a_),(x_))


RTIBool
NDDSA_XmlVisitor_on_topic(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_topic(s_,a_,x_) \
    (s_)->on_topic((s_)->user_data,(a_),(x_))

RTIBool
NDDSA_XmlVisitor_on_datawriter(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_datawriter(s_,a_,x_) \
    (s_)->on_datawriter((s_)->user_data,(a_),(x_))

RTIBool
NDDSA_XmlVisitor_on_datareader(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_datareader(s_,a_,x_) \
    (s_)->on_datareader((s_)->user_data,(a_),(x_))

RTIBool
NDDSA_XmlVisitor_on_error(
    NDDSA_XmlVisitor *const self,
    NDDSA_Agent *const agent,
    struct DDS_XMLObject *const xml_obj);

#define NDDSA_XmlVisitor_on_error(s_,a_,x_) \
    (s_)->on_error((s_)->user_data,(a_),(x_))

RTIBool
NDDSA_Agent_visit_xml(
    NDDSA_Agent *const self,
    NDDSA_XmlVisitor *const visitor);

#endif /* NddsXmlVisitor_h */