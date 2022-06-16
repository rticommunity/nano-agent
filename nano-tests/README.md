# nano-tests - Unit tests for nano-client and nano-agent

## Native Tests

1. Load RTI Connext DDS, e.g. 6.0.1 on Linux 64-bit:

   ```sh
   source ~/rti_connext_dds.6.0.1/resource/script/rtisetenv_x64Linux4gcc7.3.0.bash
   ```

2. Clone, build and install [nano-agent](https://github.com/rticommunity/nano-agent)
   with unit tests enabled:

   ```sh
   git clone --recurse-submodules https://github.com/rticommunity/nano-agent
   mkdir nano-agent/build
   cmake -Bnano-agent/build -Hnano-agent \
     -DCMAKE_INSTALL_PREFIX=nano-agent/install \
     -DBUILD_UNIT_TESTS=ON
   cmake --build nano-agent/build --target install
   ```

3. Run the tests with `ctest`:

  ```sh
  (cd nano-agent/build/nano-tests && ctest)
  ```

