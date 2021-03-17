
# IIO EMU
Server application for libiio clients. IIO-Emu can be extended to simulate different behaviors.

Available options:

| option | arguments | description |
| --------- | ----------- | ----------- |
| generic_xml | XML path | Creates a server that uses a context just for accessing attributes based on the XML file. The file should respect the template |
| adalm2000 | - | Creates a server for emulating the basic behavior of ADALM2000 |


# Build instructions

## Linux

### Dependencies

Basic system setup
```shell
sudo apt-get update
sudo apt-get install build-essential
```

Install Prerequisites
```shell
sudo apt-get install libxml2-dev
```

Build IIOD protocol lib
```shell
git clone https://github.com/analogdevicesinc/libtinyiiod.git
cd libtinyiiod
mkdir build && cd build
cmake -DBUILD_EXAMPLES=OFF ..
make
sudo make install
```

Build iio-emu
```shell
git clone https://github.com/analogdevicesinc/iio-emu.git
cd iio-emu
mkdir build && cd build
cmake ..
make
```
