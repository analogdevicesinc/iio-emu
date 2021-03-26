
# Creating your own virtual device

## Define context structure
The first step is defining the structure of your context. Any context can be represented
as an XML file, similar to the one that libiio is extracting from the context. The only
difference between libiio and iio-emu XML file is that iio-emu XML contains the attribute
'value' for all end-nodes. An example of a valid XML file:

```xml
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE context [
	<!ELEMENT context (device | context-attribute)*>
	<!ELEMENT context-attribute EMPTY>
	<!ELEMENT device (channel | attribute | debug-attribute | buffer-attribute)*>
	<!ELEMENT channel (scan-element? , attribute*)>
	<!ELEMENT attribute EMPTY>
	<!ELEMENT scan-element EMPTY>
	<!ELEMENT debug-attribute EMPTY>
	<!ELEMENT buffer-attribute EMPTY>
	<!ATTLIST context name CDATA #REQUIRED>
	<!ATTLIST context description CDATA #IMPLIED>
	<!ATTLIST context-attribute name CDATA #REQUIRED>
	<!ATTLIST context-attribute value CDATA #REQUIRED>
	<!ATTLIST device id CDATA #REQUIRED>
	<!ATTLIST device name CDATA #IMPLIED>
	<!ATTLIST channel id CDATA #REQUIRED>
	<!ATTLIST channel type (input | output) #REQUIRED>
	<!ATTLIST channel name CDATA #IMPLIED>
	<!ATTLIST scan-element index CDATA #REQUIRED>
	<!ATTLIST scan-element format CDATA #REQUIRED>
	<!ATTLIST scan-element scale CDATA #IMPLIED>
	<!ATTLIST attribute name CDATA #REQUIRED>
	<!ATTLIST attribute filename CDATA #IMPLIED>
	<!ATTLIST attribute value CDATA #IMPLIED>
	<!ATTLIST debug-attribute name CDATA #REQUIRED>
	<!ATTLIST debug-attribute value CDATA #IMPLIED>
	<!ATTLIST buffer-attribute name CDATA #REQUIRED>
	<!ATTLIST buffer-attribute value CDATA #IMPLIED>
	]>
<context name="name" description="description">
   <context-attribute name="name" value="value"/>
    <device id="iio:deviceX" name="device_name">
	    <channel id="voltageX" name="ch_name" type="input">
                <attribute name="attr" filename="in_voltageX_ch_name_attr" value="value"/>
        </channel>
	    <attribute name="dev_attr" value="value"/>
	    <buffer-attribute name="buffer_attr" value="value"/>
	    <debug-attribute name="debug_attr" value="value"/>
    </device>
</context>
```

The implementation of the new emulated hardware must be located in iio-emu/iiod/context.
There a directory named after the context should be created and inside it will be added the XML
file, the context class and eventually input/output devices.
The XML file (<file_name>.xml) will be compiled and a header file (<file_name>_xml.h) will be
created, containing an array of bytes (<file_name>_xml). 

## Implement behavior

The simplest way to develop a new context is to extend the GenericXmlContext class.

```cpp
#include "iiod/context/generic_xml/generic_xml_context.hpp"

namespace iio_emu {

class MyCustomContext : public GenericXmlContext
{
public:
        MyCustomContext();
        ~MyCustomContext() override;

}
};
```

The operation inside libtinyiiod should have an implementation assigned, that is why you
should call either assignBasicOps or assignAllOps. Also the ops can be manually assigned.

```cpp
#include "my_custom_device_context.hpp"
#include <custom_device_xml.h>

using namespace iio_emu;

MyCustomContext::MyCustomContext()
: GenericXmlContext(reinterpret_cast<const char*>(custom_device_xml), sizeof(custom_device_xml))
{
        assignBasicOps();
}
```

## Define new devices
A new device should implement one of the following interfaces: AbstractDevice, AbstractDeviceIn,
AbstractDeviceOut, based on its functionality. Simply implementing an interface to work in
the way you intent is sufficient so that a iio-emu context can use that device.

GenericXmlContext searches for a device when a device function (open, close, read, write)
is called. Any device should be added in order to be used:

```cpp
auto adc = new M2kADC("iio:device0", m_doc);
addDevice(adc);
```

## Observations

For a more flexible approach you can implement AbstractOps interface (have a look at
GenericXmlContext implementation).
