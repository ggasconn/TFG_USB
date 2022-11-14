from ._version import  __version__

import usb.core
import usb.util

VENDOR_ID = 0x20a0
PRODUCT_ID = 0x41e5

def find_first():
    d = usb.core.find(find_all=False, idVendor=VENDOR_ID, idProduct=PRODUCT_ID)
    
    if d:
        return PwnedDevice(device=d)

class PwnedDeviceException(Exception):
    pass

class PwnedDevice(object):

    def __init__(self, device=None, error_reporting=True):
        self.error_reporting = error_reporting

        if device:
            self.device = device
            self.open_device(device)

            self.bs_serial = self.get_serial()
    
    def _usb_get_string(self, device, index):
        try:
            return usb.util.get_string(device, index, 1033)
        except usb.USBError:
            if self._refresh_device():
                return usb.util.get_string(self.device, index, 1033)
            else:
                raise PwnedDeviceException("Could not communicate with PwnedDevice {0} - it may have been removed".format(self.bs_serial))
    
    def open_device(self, d):
        if self.device is None:
            raise PwnedDeviceException("Could not find PwnedDevice...")

        if self.device.is_kernel_driver_active(0):
            try:
                self.device.detach_kernel_driver(0)
            except usb.core.USBError as e:
                raise PwnedDeviceException("Could not detach kernel driver: %s" % str(e))

        return True
    
    def get_serial(self):
        return self._usb_get_string(self.device, 3)

    def _refresh_device(self):
        d = find_first()
        if d:
            self.device = d.device
            return True

    def _usb_ctrl_transfer(self, bmRequestType, bRequest, wValue, wIndex, data_or_wLength):
        try:
            return self.device.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, data_or_wLength)
        except usb.USBError:
            if self._refresh_device():
                return self.device.ctrl_transfer(bmRequestType, bRequest, wValue, wIndex, data_or_wLength)
            else:
                raise PwnedDeviceException("Could not communicate with PwnedDevice {0} - it may have been removed".format(self.bs_serial))

    def get_block(self, reportID, msgSize):
        reportID = reportID + 0x0000
        device_bytes = self._usb_ctrl_transfer(0x80 | 0x20, 0x1, reportID, 0, msgSize)
        result = ""
        
        for i in device_bytes[1:]:
            if i == 0:
                break
            result += chr(i)
        
        return result

    def _data_to_message(self, data):
        bytes = [1]
        
        for c in data:
            bytes.append(ord(c))

        for i in range(32 - len(data)):
            bytes.append(0)

        return bytes

    def set_block(self, data):
        # TOOD
        # self._usb_ctrl_transfer(0x20, 0x9, 0x0003, 0, self._data_to_message(data))
        pass