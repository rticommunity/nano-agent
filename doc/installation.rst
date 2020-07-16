.. include:: vars.rst

.. _section-install:

************
Installation
************

|RTI| |NANO_AGENT| is distributed in source format. The project's
:link_nano_agent_git:`Git repository <>`
contains all available components in buildable source form.

The repository may be cloned locally and built using one
of the supported methods (see :ref:`section-building`).

.. _section-install-src:

Installation From Source
========================

Clone |RTI| |NANO_AGENT|'s
:link_nano_agent_git:`Git repository <>`:

.. parsed-literal::

    git clone --recurse-submodules |link_nano_agent_git|


When building |RTI| |NANO_AGENT| with CMake, you can take advantage of the generated
``install`` target to copy all build artifacts to your desired location.


.. _section-install-client:

RTI nano-client
===============

|RTI| |NANO_AGENT|'s repository includes a clone of
:link_nano_client_git:`RTI nano-client <>`.

Please refer to |RTI| |NANO_CLIENT|'s :link_nano_client_docs:`documentation <>`
for more information on how to install it, build it, and use it in your projects.


.. _section-install-docs:

Documentation
=============

|RTI| |NANO_AGENT|'s user manual is written using :link_sphinx:`Sphinx <>`,
while API documentation is generated using :link_doxygen:`Doxygen <>`.

Once these tools are installed on your system (and available in your ``PATH``
environment variable), documentation can be generated using the CMake build 
script by enabling option ``BUILD_DOCS``. You can also use the ``BUILD_LIBRARIES``
and ``BUILD_EXECUTABLES`` options to skip compilation of the source code.

.. code-block:: sh

    mkdir build && cd build

    cmake /path/to/nano-client -DBUILD_DOCS=ON \
                               -DBUILD_LIBRARIES=OFF \
                               -DBUILD_EXECUTABLES=OFF
    
    cmake --build . --target install

    # Agent Manual:   ./install/doc/manual/nano-agent/html
    # Client Manual:  ./install/doc/manual/nano-client/html
    # Client API Ref: ./install/doc/api/nano-client/html/

The generation of |RTI| |NANO_CLIENT|'s documentation can be disabled disabling
option ``BUILD_DOCS_CLIENT``.
