# Example - Linux EXT4 panic

This reboots/panics Linux boxes as soon as soon as the device is plugged in by abusing the debug feature of ext filesystems and auto mounting.

An EXT filesystem has a header, this contains a field that tells the OS what to do if an invalid filesystem is detected. One of the options you can set is to get the kernel to panic. As USB devices are often auto mounted this means you can provide a filesystem that will crash a suitably configured Linux machine.

In the right situation you could use this to boot a kiosk into your own OS. First time the device appears as a corrupt USB stick which causes the machine to reboot. Next the USB Army Knife appears as a Linux live image and the UEFI boots it. When loaded this new OS adds malware to the existing HDD and reboots the device. USB Army Knife doesn’t appear as anything allowing, this allows the original OS to load.

**Beware**, some Linux distros add `errors=remount-ro` when drives are automounted. This overrides the debug value in the header making the attack fail. YMMV.

## Set up
1. Copy autorun.ds onto the SD card
2. Copy badfs.img onto the SD card

## Usage
1. Plug in device to a Linux machine