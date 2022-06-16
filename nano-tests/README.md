# nano-test - Unit tests for nano-client and nano-agent

This repository contains unit tests for
[nano-client](https://github.com/rticommunity/nano-client) and
[nano-agent](https://github.com/rticommunity/nano-agent).

## Native Tests

1. Load RTI Connext DDS, e.g. 6.0.1 on Linux 64-bit:

   ```sh
   source ~/rti_connext_dds.6.0.1/resource/script/rtisetenv_x64Linux4gcc7.3.0.bash
   ```

2. Clone, build and install [nano-agent](https://github.com/rticommunity/nano-agent):

   ```sh
   git clone --recurse-submodules https://github.com/rticommunity/nano-agent
   mkdir nano-agent/build
   cmake -Bnano-agent/build -Hnano-agent \
     -DCMAKE_INSTALL_PREFIX=nano-agent/install
   cmake --build nano-agent/build --target install
   ```

3. Build unit tests:

   ```sh
   mkdir nano-agent/nano-tests/build
   cmake -Bnano-agent/nano-tests/build -Hnano-agent/nano-tests \
     -DCMAKE_PREFIX_PATH=$(pwd)/nano-agent/install \
   cmake --build nano-agent/nano-tests/build
   ```

4. Run the included tests:

  ```sh
  (cd nano-agent/nano-tests/build && ctest)
  ```

