# Copyright 2022 Real-Time Innovations, Inc. (RTI)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
cmake_minimum_required(VERSION 3.9)
project(nano-test
    LANGUAGES   C CXX
    VERSION     0.1.0
    DESCRIPTION "Unit tests for nano-client and nano-agent")

option(ENABLE_XRCE_CLIENT "build unit tests for nano-client" ON)
option(ENABLE_XRCE_AGENT "build unit tests for nano-agent" ON)
option(ENABLE_ECHO_SERVER "build echo server to support testing on embedded targets" ON)
option(USE_ECHO_SERVER "build echo server version of tests" OFF)
option(ENABLE_UNIT_TESTS "build unit tests" ON)
option(ENABLE_CTEST "enable support for running unit tests with ctest" ON)
option(INSTALL_UNIT_TESTS "install unit test executables" OFF)

if(ENABLE_CTEST)
  include(CTest)
  enable_testing()
endif()

install(DIRECTORY include/nano
  DESTINATION include
)

function(build_unity)
  include(ExternalProject)

  ExternalProject_add(unity_project
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity
    GIT_TAG v2.5.2
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/unity
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/unity-build
    CONFIGURE_COMMAND cmake ${CMAKE_CURRENT_BINARY_DIR}/unity/src/unity_project
    BUILD_COMMAND cmake --build .
    INSTALL_COMMAND cmake --install . --prefix ${CMAKE_CURRENT_BINARY_DIR}/unity-install
    UPDATE_COMMAND ""
  )

  add_library(unity STATIC IMPORTED)
  set_target_properties(unity
    PROPERTIES
      IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/unity-install/lib/libunity.a
      # INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/unity-install/include/unity
  )
  add_dependencies(unity unity_project)
endfunction()

function(build_nanocoretest)
  cmake_parse_arguments(_nanocoretest
    "AGENT"
    ""
    ""
    ${ARGN}
  )
  if(_nanocoretest_AGENT)
    set(lib_tgt nanocoretestagent)
    set(nano_dep nano::nanocoreagent)
    set(nano_pkg nano-core-agent)
  else()
    set(lib_tgt nanocoretest)
    set(nano_dep nano::nanoclient)
    set(nano_pkg nano-client)
  endif()

  if(NOT TARGET ${nano_dep})
    find_package(${nano_pkg} REQUIRED CONFIG)
  endif()

  add_library(${lib_tgt}
    test/nano_core_test.cpp
    include/nano/nano_core_test.h
  )

  target_include_directories(${lib_tgt}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/unity-install/include/unity>
  )

  if(USE_ECHO_SERVER)
    target_compile_definitions(${lib_tgt}
      PUBLIC NANOTEST_FEAT_UDP_ECHO=1)
  endif()

  target_link_libraries(${lib_tgt}
    ${nano_dep}
    unity
  )

  install(TARGETS ${lib_tgt}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )
endfunction()

function(build_runner exec)
  cmake_parse_arguments(_runner
    "AGENT"
    "HEADER;FUNCTION"
    "SOURCES;INCLUDES"
    ${ARGN}
  )

  set(exec_tgt ${exec})
  set(test_lib nanocoretest)

  if(_runner_AGENT)
    set(exec_tgt ${exec_tgt}_agent)
    set(test_lib nanocoretestagent)
  endif()

  add_executable(${exec_tgt}
    ${_runner_SOURCES}
    src/runner/TestRunner.c
  )

  target_include_directories(${exec_tgt}
    PRIVATE ${_runner_INCLUDES}
  )

  target_link_libraries(${exec_tgt}
    ${test_lib}
  )

  target_compile_definitions(${exec_tgt}
    PRIVATE
      "NANO_TEST_H=\"${_runner_HEADER}\""
      NANO_TEST_FN=${_runner_FUNCTION}
  )

  if(_runner_AGENT)
    target_compile_definitions(${exec_tgt}
      PRIVATE
        NANO_FEAT_AGENT=1
    )
  endif()

  # if(INSTALL_UNIT_TESTS)
  #   install(TARGETS ${exec_tgt}
  #     RUNTIME DESTINATION bin/test
  #   )
  # endif()
  install(TARGETS ${exec_tgt}
    RUNTIME DESTINATION bin/test
  )

  if(ENABLE_CTEST)
    add_test(NAME ${exec_tgt} COMMAND ${exec_tgt})
  endif()
endfunction()

function(build_unit_tests)
  cmake_parse_arguments(_tests
    "AGENT"
    ""
    ""
    ${ARGN}
  )

  if(_tests_AGENT)
    set(_tests_extra_args
      AGENT
    )
  endif()

  build_runner(test_core_osapi_time
    SOURCES
      test/core/osapi/test_time/TestTime.c
      test/core/osapi/test_time/TestTime.h
    INCLUDES
      test/core/osapi/test_time
    HEADER TestTime.h
    FUNCTION test_NANO_Time
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_memory
    SOURCES
      test/core/osapi/test_memory/TestMemory.c
      test/core/osapi/test_memory/TestMemory.h
    INCLUDES
      test/core/osapi/test_memory
    HEADER TestMemory.h
    FUNCTION test_NANO_OSAPI_Memory
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_clock
    SOURCES
      test/core/osapi/test_clock/TestClock.c
      test/core/osapi/test_clock/TestClock.h
    INCLUDES
      test/core/osapi/test_clock
    HEADER TestClock.h
    FUNCTION test_NANO_OSAPI_Clock
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_message_buffer
    SOURCES
      test/core/osapi/test_message_buffer/TestMessageBuffer.c
      test/core/osapi/test_message_buffer/TestMessageBuffer.h
    INCLUDES
      test/core/osapi/test_message_buffer
    HEADER TestMessageBuffer.h
    FUNCTION test_NANO_MessageBuffer
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_message_buffer_queue
    SOURCES
      test/core/osapi/test_message_buffer_queue/TestMessageBufferQueue.c
      test/core/osapi/test_message_buffer_queue/TestMessageBufferQueue.h
    INCLUDES
      test/core/osapi/test_message_buffer_queue
    HEADER TestMessageBufferQueue.h
    FUNCTION test_NANO_MessageBufferQueue
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_ipv4_addr
    SOURCES
      test/core/osapi/test_ipv4_addr/TestIpv4Addr.c
      test/core/osapi/test_ipv4_addr/TestIpv4Addr.h
    INCLUDES
      test/core/osapi/test_ipv4_addr
    HEADER TestIpv4Addr.h
    FUNCTION test_NANO_OSAPI_Ipv4Addr
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_sequence
    SOURCES
      test/core/osapi/test_sequence/TestSequence.c
      test/core/osapi/test_sequence/TestSequence.h
    INCLUDES
      test/core/osapi/test_sequence
    HEADER TestSequence.h
    FUNCTION test_NANO_Sequence
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_sequence_iterator
    SOURCES
      test/core/osapi/test_sequence_iterator/TestSequenceIterator.c
      test/core/osapi/test_sequence_iterator/TestSequenceIterator.h
    INCLUDES
      test/core/osapi/test_sequence_iterator
    HEADER TestSequenceIterator.h
    FUNCTION test_NANO_SequenceIterator
    ${_tests_extra_args}
  )

  build_runner(test_core_osapi_udpv4_socket
    SOURCES
      test/core/osapi/test_udpv4_socket/TestUdpv4Socket.c
      test/core/osapi/test_udpv4_socket/TestUdpv4Socket.h
    INCLUDES
      test/core/osapi/test_udpv4_socket
    HEADER TestUdpv4Socket.h
    FUNCTION test_NANO_OSAPI_Udpv4Socket
    ${_tests_extra_args}
  )  
endfunction()

function(build_echo_server)
  add_executable(echo_server
    src/echo/echo_server.c
  )

  target_link_libraries(echo_server nanocoretest)

  install(TARGETS echo_server
    RUNTIME DESTINATION bin
  )
endfunction()

build_unity()

if(ENABLE_XRCE_CLIENT)
  build_nanocoretest()
endif()
if(ENABLE_XRCE_AGENT)
  build_nanocoretest(AGENT)
endif()

if(ENABLE_ECHO_SERVER)
  build_echo_server()
endif()

if(ENABLE_UNIT_TESTS)
  if(ENABLE_XRCE_CLIENT)
    build_unit_tests()
  endif()
  if(ENABLE_XRCE_AGENT)
    build_unit_tests(AGENT)
  endif()
endif()
