################################################################################
#
# (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
# 
# RTI grants Licensee a license to use, modify, compile, and create derivative
# works of the Software solely in combination with RTI Connext DDS. Licensee
# may redistribute copies of the Software provided that all such copies are
# subject to this License. The Software is provided "as is", with no warranty
# of any type, including any warranty for fitness for any purpose. RTI is
# under no obligation to maintain or support the Software. RTI shall not be
# liable for any incidental or consequential damages arising out of the use or
# inability to use the Software. For purposes of clarity, nothing in this
# License prevents Licensee from using alternate versions of DDS, provided
# that Licensee may not combine or link such alternate versions of DDS with
# the Software.
#
################################################################################

CMAKE				?= cmake
BUILD_DIR			?= $(shell pwd)/build
BUILD_DIR_CLIENT	?= $(BUILD_DIR)/client
INSTALL_DIR			?= $(shell pwd)/install
BUILD_TYPE			?= Release
BUILD_SHARED		?= OFF
CMAKE_GENERATOR 	?= Unix Makefiles
ENABLE_EXAMPLES 	?= ON
ENABLE_TESTS    	?= OFF

export CMAKE \
	   BUILD_TYPE \
	   BUILD_SHARED \
	   CMAKE_GENERATOR \
	   ENABLE_EXAMPLES \
	   ENABLE_TESTS \
	   INSTALL_DIR

.PHONY : all \
		 build \
		 cmake \
		 install \
		 clean \
		 clean-deep \
		 client

all: build

install: build
	$(CMAKE) --build $(BUILD_DIR) --target install -- -j4

build: cmake
	$(CMAKE) --build $(BUILD_DIR) -- -j4

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean

clean-deep:
	rm -rf $(BUILD_DIR) $(INSTALL_DIR)

cmake: $(BUILD_DIR).dir
	$(CMAKE) -B$(BUILD_DIR) -H. -G "$(CMAKE_GENERATOR)" \
	                            -DCONNEXTDDS_DIR=$(CONNEXTDDS_DIR) \
								-DCONNEXTDDS_ARCH=$(CONNEXTDDS_ARCH) \
	                            -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	                            -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
	                            -DBUILD_SHARED_LIBS=$(BUILD_SHARED) \
	                            -DENABLE_EXAMPLES=$(ENABLE_EXAMPLES) \
	                            -DENABLE_TESTS=$(ENABLE_TESTS) ${CMAKE_ARGS}

%.dir:
	mkdir -p $*

client: install \
		$(BUILD_DIR_CLIENT).dir
	$(MAKE) -C nano-client \
		BUILD_DIR="$(BUILD_DIR_CLIENT)" \
		install
