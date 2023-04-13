# Tools

## Userspace PwnedDevice Module

pwnedDevice is a Python 3 module that has a few functions built-in to allow communication with the device skipping the kernel coding stage for fast prototyping.

### Read data example

```python
from pwnedDevice import pwnedDevice # Import module
b = pwnedDevice.find_first() # Find connected device
b.get_block(3, 33) # Sends READ request to ReportID 3 asking for 33 bytes of data.

```

