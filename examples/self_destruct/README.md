# Example - Self destruct

This is an example of using an auxillary devices connected to the LILYGO T Dongle S3's QWIIC port to perform motion detection. In this example if the device discovers motion close to it then the SELF_DESTRUCT() function is run. In our example this simply prints a graphic and resets the device. You are free to implement your own.


https://github.com/user-attachments/assets/d8c192d9-2c56-4526-8ca4-b630c6875a1a


## Set up
1. Copy autorun.ds and bomb.png onto the SD card
1. In the platform.ini configuration file uncomment the line `-D EXT_SENSOR_MOTION_LD2410` this will enable the sensor module to be built
1. Build and flash the device
1. Connect a LD2410 to the QWIIC connector of the T Dongle S3. You'll need:
   * A QWIIC micro cable AKA a 4-pin micro JST connector (1mm).
   * A [HLK-LD2410C](https://s.click.aliexpress.com/e/_EJ2MAjf) or similar
   * A 3.3v to 5v DC-DC step up converter. The LD2410 needs 5v - potentially you could get this from the USB connecter. Connect the red cable to IN+ and OUT+ to VCC on the LD2410.
   * To connect the blue cable to RX
   * To connect the yellow cable to TX
   * To connect the black cable to IN- and OUT- to the LD2410 GND pin

**NOTE** In the pictures and video a JST connector cable with different colours is in use.

## Usage
1. Plug in device
2. Wait a few seconds for the device to scan the room
3. Put your hand over the sensor
