# RTI nano-agent

*RTI nano-agent* is a [standard-compliant][omg-xrce] implementation of the XRCE
Agent service, which can be used to bridge XRCE applications with the DDS Global
Data Space.

*RTI nano-agent* is built on top of the [*RTI Connext DDS*][connext-home]
libraries, and the open-source XRCE protocol implementation provided by
[*RTI nano-client*][nano-client-git].

This project is part of RTI's [Experimental Projects][experimental-faq].

## DDS-XRCE

![DDS-XRCE System Architecture][dds-xrce-architecture]

Thanks to XRCE, limited devices such as microcontrollers and other embedded
targets can become part of a DDS system.

[DDS-XRCE][omg-xrce] (or "DDS for e*X*tremely *R*esource *C*onstrained *E*nvironments") introduces 
an alternative interface to DDS which offloads all management of DDS entities
from applications to an external Agent process.

Applications connect to the XRCE Agent as clients, and they use the
*XRCE Client API* to:

* Create and configure DDS entities on the Agent.
* Write DDS samples using a DataWriter on the Agent.
* Read DDS samples received by a DataReader on the Agent.

XRCE's client/server model significantly reduces the minimal memory
footprint required by an application to use DDS.

Additionally, XRCE is a message-oriented protocol which may be carried out
over any transport with a Maximum Transport Unit of at least 24 bytes, with
standard mappings for TCP/UDP sockets, and Serial lines.

The XRCE standard also defines a custom reliability protocol for reliable
delivery of messages over unreliable transports, and it supports fragmentation
(and reconstruction) of large payloads which exceed the transport's MTU.

## Documentation

Please refer to the [User Manual][nano-agent-docs] for 
information on how to install, build, and use *RTI nano-agent*.

## License

RTI grants Licensee a license to use, modify, compile, and create derivative
works of the Software solely in combination with RTI Connext DDS. Licensee
may redistribute copies of the Software provided that all such copies are
subject to this License. The Software is provided "as is", with no warranty
of any type, including any warranty for fitness for any purpose. RTI is
under no obligation to maintain or support the Software. RTI shall not be
liable for any incidental or consequential damages arising out of the use or
inability to use the Software. For purposes of clarity, nothing in this
License prevents Licensee from using alternate versions of DDS, provided
that Licensee may not combine or link such alternate versions of DDS with
the Software.

```text
(c) 2020 Copyright, Real-Time Innovations, Inc. (RTI)
```

[omg-xrce]: https://www.omg.org/spec/DDS-XRCE/About-DDS-XRCE/ "OMG DDS-XRCE Specification"
[nano-client-git]: https://github.com/rticommunity/nano-client.git "RTI nano-client Git repository"
[nano-agent-docs]: https://community.rti.com/static/documentation/nano/nano-agent/latest "RTI nano-agent User Manual"
[connext-home]: https://www.rti.com/products/connext-dds-professional "RTI Connext DDS Professional Homepage"
[experimental-faq]: https://www.rti.com/developers/rti-labs/experimental-product-faq "RTI Experimental Product FAQ"
[dds-xrce-architecture]: doc/static/dds_xrce_architecture.png "DDS-XRCE System Architecture"
