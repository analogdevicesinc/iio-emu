# iio-emu: tools

## xml_gen < uri >
Generate an XML file of a given IIO context based on its URI. The file is similar to the one inside the iio
context, the only difference between these files is that iio-emu XML contains the attribute
'value' for all end-nodes, in which is stored the value of the attribute at the generation time.

### Dependencies:
 - libxml2
 - libiio

### Build
Build iio-emu with 'BUILD_TOOLS' option on.

```shell
mkdir build
cd build
cmake -DBUILD_TOOLS=ON ..
make -j3
```
### Example:
```shell
xml_gen ip:192.168.2.1
```
