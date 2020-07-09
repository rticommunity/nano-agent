.. include:: vars.rst

.. _section-building:

********
Building
********

|RTI| |NANO_AGENT| can be built from source using :link_cmake:`CMake <>`.

.. _section-building-deps:

Build Dependencies
==================

|RTI| |NANO_AGENT| requires :link_connext_home:`RTI Connext DDS <>`
6.0.0 or later to be installed on the build host.

Consult :link_connext_install:`RTI Connext DDS' documentation <>`
for more information on how to do this.

.. _section-building-cmake:

Building with CMake
===================

The CMake script requires the |RTI| |CONNEXT_DDS| installation to be configured
using variables ``CONNEXTDDS_DIR`` and ``CONNEXTDDS_ARCH``.

The following snippet will build ``nanoagentd`` using default build options:

.. code-block:: sh

    # Create a build directory and enter it
    mkdir build && cd build

    # Run cmake to configure build
    cmake /path/to/nano-agent -DCONNEXTDDS_DIR=/path/to/rti_connext_dds \
                              -DCONNEXTDDS_ARCH=x64Linux4gcc7.3.0

    # Call native build tool
    cmake --build . --target install



.. _section-building-xcc:

Cross-compilation with CMake
============================

.. _section-building-xcc-rpi:

Raspberry Pi
------------

.. code-block:: cmake

    #
    # This toolchain file can be used to cross-compile RTI nano-client and
    # nano-agent for Raspberry Pi.
    #
    # Make sure that the following variables are set in your shell's 
    # environment:
    #
    #   - RPI_TOOLS_DIR : clone of https://github.com/raspberrypi/tools
    #
    # If you are building nano-agent, make sure to also set the following variables:
    #
    #   - CONNEXTDDS_DIR: installation of RTI Connext DDS
    #                     (host bundle and target libraries).
    #
    #   - CONNEXTDDS_ARCH: architecture for RaspberryPi target 
    #                      (e.g. armv7Linux4gcc7.3.0).
    #

    if("$ENV{CONNEXTDDS_DIR}" STREQUAL "" OR
       "$ENV{CONNEXTDDS_ARCH}" STREQUAL "")
       message(FATAL_ERROR "Invalid CONNEXTDDS_DIR or CONNEXTDDS_ARCH")
    endif()

    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_VERSION 1)

    # This toolchain can be used for both RPi2 and RPi3.
    # Might need a different toolchain for RPi4/64bit
    set(RPI_TOOLCHAIN_DIR
        $ENV{RPI_TOOLS_DIR}/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian)


    set(CMAKE_C_COMPILER   ${RPI_TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER ${RPI_TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)

    list(APPEND CMAKE_FIND_ROOT_PATH      ${RPI_TOOLCHAIN_DIR})

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    # Explicitly link rt library to nanocore library
    set(NANO_CORE_EXTRA_LIBS            rt)

    # Since the toolchain file restricts CMake's find() functions,
    # We must add the following Connext paths to CMake's search path
    # to help FindRTIConnextDDS.cmake
    list(APPEND CMAKE_FIND_ROOT_PATH
        $ENV{CONNEXTDDS_DIR}
        $ENV{CONNEXTDDS_DIR}/bin
        $ENV{CONNEXTDDS_DIR}/lib/$ENV{CONNEXTDDS_ARCH}
        $ENV{CONNEXTDDS_DIR}/include/ndds)

Save the previous block in a file named ``rpi_toolchain.cmake`` and
build |RTI| |NANO_AGENT| with the following snippet:

.. code-block:: sh

    # Clone RPi build tools and export their location as RPI_TOOLCHAIN_DIR
    git clone https://github.com/raspberrypi/tools
    export RPI_TOOLCHAIN_DIR=$(pwd)/tools

    # Create a build directory and enter it
    mkdir build-rpi && cd build-rpi

    # Configure build using the custom toolchain. Optional: build examples.
    cmake /path/to/nano-agent -DCMAKE_TOOLCHAIN_FILE=rpi_toolchain.cmake \
                              -DENABLE_EXAMPLES=ON
    
    # Compile and copy to install location (./install)
    cmake --build . --target install -- -j8

    # Copy install tree to RPi home directory, e.g. with rsync:
    rsync -ra ./install/nano pi@my-rpi:~/


.. _section-building-cmake-options:

CMake Build Options
===================

This section provides information of variables that can be used to control the
behavior of |RTI| |NANO_AGENT|'s CMake build script.


.. _section-building-cmake-options-buildexecutables:

BUILD_EXECUTABLES
-----------------

Description:
    Build included executables (``nanoagentd``, examples, ...).

Accepted Values:
    ``ON``, ``OFF``

Default Value:
    ``ON``

.. _section-building-cmake-options-buildlibraries:

BUILD_LIBRARIES
---------------

Description:
    Build included libraries.

Accepted Values:
    ``ON``, ``OFF``

Default Value:
    ``ON``

.. _section-building-cmake-options-buildsharedlibs:

BUILD_SHARED_LIBS
-----------------

Description:
    :link_cmake_var:`Standard CMake option <BUILD_SHARED_LIBS>`
    which controls whether libraries and executables will be linked statically
    or dynamically.

Accepted Values:
    ``ON``, ``OFF``

Default Value:
    ``OFF``

.. _section-building-cmake-options-cmakebuildtype:

CMAKE_BUILD_TYPE
----------------

Description:
    :link_cmake_var:`Standard CMake option <CMAKE_BUILD_TYPE>`
    which controls the type of libraries and executables to build.

Accepted Values:
    ``"Release"``, ``"Debug"``, ``"RelWithDebInfo"``, ``"MinSizeRel"``

Default Value:
    ``Release``

.. _section-building-cmake-options-cmakeinstallprefix:

CMAKE_INSTALL_PREFIX
--------------------

Description:
    :link_cmake_var:`Standard CMake option <CMAKE_INSTALL_PREFIX>`
    which controls the type of libraries and executables to build.

Accepted Values:
    A valid path in CMake syntax (i.e. using forward-slashes ``/`` only).

Default Value:
    ``${CMAKE_CURRENT_BUILD_DIR}/install``

.. _section-building-cmake-options-connextdds-arch:

CONNEXTDDS_ARCH
---------------

Description:
    Name of the target architecture for the |RTI| |CONNEXT_DDS| libraries;

Accepted Values:
    A valid RTI architecture name.

Default Value:
    Environment variable ``${CONNEXTDDS_ARCH}``

.. _section-building-cmake-options-connextdds-dir:

CONNEXTDDS_DIR
--------------

Description:
    Directory where |RTI| |CONNEXT_DDS| is installed (a.k.a. ``NDDSHOME``).

Accepted Values:
    A valid path in CMake syntax (i.e. using forward-slashes ``/`` only).

Default Value:
    Environment variable ``${CONNEXTDDS_DIR}``
