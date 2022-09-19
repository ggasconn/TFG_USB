# Running `custom-class` example from the V-USB package

## Compiling the host side software

A Makefile is provided, so we just run make on the `commandline` directory.

```bash	
make
```

If the `<usb.h>` library cannot be found, just install it running the following command:

```bash	
sudo apt-get install libusb-dev
```

Once the compiling process is finished, the program will be available as`set-led`