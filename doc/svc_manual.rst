.. include:: vars.rst

.. _section-manual:

**************************
``nanoagentd`` User Manual
**************************

|RTI| |NANO_AGENT| provides executable ``nanoagentd`` to deploy instances
of the XRCE Agent service.

``nanoagentd`` accepts multiple command line arguments to control the behavior
of the XRCE Agent, such as the transport to use, and the timeout period for
client sessions.

An optional XML configuration file may also be provided to automatically create
DDS entities (see :ref:`section-manual-xml`).

.. _section-manual-transport:

Transport Configuration
=======================

``nanoagentd`` requires at least one of the available transport plugins to be
selected using the associated command-line options.

Multiple transports may be enabled concurrently.

At the moment, ``nanoagentd`` supports loading:

* up to one instance of the UDPv4 transport.

* up to three instances of the Serial transport.

.. _section-manual-transport-serial:

Serial Transport Configuration
------------------------------

The Serial transport can be enabled with the ``-S`` option.

At a minimum, you must specify the serial device to use (using option ``-Sd DEVICE``).
You will likely also want to configure the serial speed (``-Ss SPEED``) and specify
a finite timeout for reading data from the serial line (``-St TIMEOUT``).

Up to two additional instances of the Serial transport can be created on
different devices using options ``-Sd1 DEVICE -Ss1 SPEED``, and
``-Sd2 DEVICE -Ss2 SPEED``. These instances will share other options with
the first instance.

.. _section-manual-transport-udpv4:

UDPv4 Transport Configuration
-----------------------------

The UDPv4 transport can be enabled with the ``-U`` option.

If no other option is specified, ``nanoagentd`` will create a UDP socket on 
the default XRCE Agent port (7401). This port can be modified with option ``-Up``.

The transport currently only supports listening on all available network
interfaces.

.. _section-manual-session:

Client Session Configuration
============================

.. _section-manual-session-timeout:

Client Session Timeout
----------------------

By default, client sessions will only be disposed when an explicit request
is received from a client.

``nanoagentd`` can be configured with an optional timeout period (``-t``), and
it will dispose a client session if no message is received from a client within
the specified time.

Clients may rely on periodically writing data to assert their presence.
If the application does not write data and doesn't send any other kind of
periodic message (for example a fully best-effort subscriber) it may use other
methods provided by its Client API to assert its presence, for example by
periodically sending an "empty" ``GET_INFO`` message.

.. _section-manual-session-hb:

HEARTBEAT Period
----------------

The XRCE protocol defines a reliability protocol which Clients and Agents can
carry out to guarantee the delivery of XRCE messages sent over an unreliable
transport.

The two sides exchanges ``HEARTBEAT`` and ``ACKNACK`` messages to, respectively,
announce the availability of unacknowledged messages, and request messages that
have not yet been received.

By default, ``nanoagentd`` will announce unacknowledged messages with a period
of 100ms. This period can be configured using the ``-hb`` option.


.. _section-manual-xml:

Pre-create DDS Entities via XML Configuration
=============================================

``nanoagentd`` accepts an optional XML configuration file which specifies 
one or more *DDS DomainParticipants* that will be automatically created,
along with their contained entities, when the XRCE Agent is started.

This configuration file uses the standard :link_omg_ddsxml:`DDS-XML <>`
syntax. You can find more information on this XML grammar in
*RTI XML-Based Application Creation*'s :link_connext_xmlappcreation:`documentation <>`.

The following snippet shows an example configuration file which will create
participant ``Participants::SensorAgent`` when loaded with the ``-c`` option:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
        xsi:noNamespaceSchemaLocation="http://community.rti.com/schema/6.0.0/rti_dds_profiles.xsd"
        version="6.0.0">
        <types>
            <struct name= "SensorData">
                <member name="id" key="true"
                        type="uint8" arrayDimensions="4"/>
                <member name="value" type="uint32"/>
            </struct>
        </types>
        <domain_library name="Domains">
            <domain name="Sensors" domain_id="46">
                <register_type name="SensorData" type_ref="SensorData" />
                <topic name="SensorReadings" register_type_ref="SensorData"/>
            </domain>
        </domain_library>
        <domain_participant_library name="Participants">
            <domain_participant name="SensorAgent"
                                domain_ref="Domains::Sensors">
                <publisher name="Publisher">
                    <data_writer name="Writer" topic_ref="SensorReadings"/>
                </publisher>
                <subscriber name="Subscriber">
                    <data_reader name="Reader" topic_ref="SensorReadings"/>
                </subscriber>
            </domain_participant>
        </domain_participant_library>
    </dds>


Once DDS Entities have been created by the Agent, clients may register any
of the available DataReaders and DataWriters to their session using their
"fully qualified name", which can be derived by joining the name of the entity
and all of its parents.

The previous XML configuration file will create the following DDS entities:

* DomainParticipant ``Participants::SensorAgent``
* Publisher ``Participants::SensorAgent::Publisher``
* Subscriber ``Participants::SensorAgent::Subscriber``
* Topic ``Participants::SensorAgent::SensorReadings``
* DataWriter ``Participants::SensorAgent::Publisher::DataWriter``
* DataReader ``Participants::SensorAgent::Subscriber::DataReader``

Alternatively, ``nanoagentd`` may be configure to automatically register these
entities with every new client session. See :ref:`section-manual-auto`.

.. _section-manual-auto:

Automatically Register DDS Entities
===================================

``nanoagentd`` can be configured to register all existing DDS entities to a
newly created client session using automatically generated identifiers.

As specified by the XRCE specification, the agent will generate these IDs from
the two least significant bytes of the MD5 hash of each entity's
"fully qualified name".

``nanoagentd`` will print the automatically generated IDs of every precreated
entity to standard output, when run with options ``-a -c config.xml``.

Additionally, you can run ``nanoagentd`` with option ``-g`` to generate 
an ID for a certain entity reference. For example, to print the automatic id
for DataWriter ``Participants::SensorAgent::Publisher::Writer``,
and DataReader ``Participants::SensorAgent::Subscriber::Reader``:

.. code-block:: sh

    nanoagentd -g "Participants::SensorAgent::Subscriber::Reader" -k dr

    nanoagentd -g "Participants::SensorAgent::Publisher::Writer" -k dw

.. _section-manual-help:

``nanoagentd --help``
=====================

.. code-block:: text

    nanoagentd - RTI Connext DDS XRCE Agent Service

      USAGE: nanoagentd [ -U | -S -Sd DEVICE] [options]

      OPTIONS:

       -a, --auto-attach

          Automatically attach any DDS resource to any client session.

       -c PATH, --config PATH

          Load default DDS resources and configuration from an XML file.

       -g RESOURCE-ID, --generate-id RESOURCE-ID

          Print the automatically generate id for the specified resource
          and exit.

       -hb PERIOD, --heartbeat-period PERIOD

          Interval in millisecond for periodic heartbeat messages.

       -k RESOURCE-KIND, --kind RESOURCE-KIND

          The type of resource for which an id is to be generated. One of:

            - domainparticipant [dp]
            - topic [t]
            - publisher [p]
            - subscriber [s],
            - datawriter [dw].
            - datareader [dr].

       -S, --serial

          Enable the Agent's serial transport.

       -Sd DEVICE, --serial-dev DEVICE

          Serial device used by the Agent's serial transport.

       -Sd1 DEVICE, --serial-dev-1 DEVICE

          Secondary serial device used by the Agent's serial transport.

       -Ss SPEED, --serial-speed SPEED

          Baud rate used to communicate over the serial line.

       -Ss1 SPEED, --serial-speed-1 SPEED

          Baud rate used to communicate over the serial line (secondary device).

       -St AMOUNT, --serial-timeout AMOUNT

          Maximum time in ms to wait for data on the serial line to be      received.

       -t AMOUNT, --timeout AMOUNT

          The maximum allowed inactivity period for a client session in
          milliseconds (default: infinite).

       -U, --udp

          Enable the Agent's UDP transport.

       -Up PORT, --udp-port PORT

          Listening port used by the Agent's UDP transport.

       -v LEVEL, --verbosity LEVEL

          Verbosity of log messages printed to console:

            - 0  (error)
            - 1  (warning)
            - 2  (info)
            - 3  (debug)
            - 4  (trace)
            - 5+ (trace more)

       -h, --help

          Display this help menu.



