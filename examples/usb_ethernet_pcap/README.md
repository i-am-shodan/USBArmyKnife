# Example - PCAP USB Ethernet traffic

There is lots of great information to see by PCAPing a network interface, this example shows you how.

**NOTE You need to change the USB mode of your device to NCM for it to appear as a USB network interface.** To do this first connect to the device over WiFi, navigate to the setting tab and change the usbDeviceType setting to NCM. This is shown about 30sec into the video below.

https://github.com/user-attachments/assets/0d5b1485-b808-46c6-aaf7-7cf016088b8f



## Set up
1. Plug in the device and set the USB mode to be USB Ethernet/NCM
2. Unplug and copy autorun.ds onto the SD card

## Usage
3. Plug in the device and wait until the message 'PCAP Stopped' appears
4. View the SD card, you should see a file called usbncm_0.pcap
