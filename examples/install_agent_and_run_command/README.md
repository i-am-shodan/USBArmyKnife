# Example - Install Agent and run a command

One of the biggest problems of BadUSB attacks is that they requires access to an unlocked machine. This can be a big problem for WiFi connected BadUSB attacks like the USB Army Knife. To work around this we can deploy our agent when the machine is unlocked. This enables us to run commands whenever we like as well as use services like VNC to scrape the screen.

Unlike other malware our agent doesn't use networking, this makes much hardre to detect on a victims machine.

This example shows you how to install the agent and run commands against it. It requires that the Agent be deployed on a disk image called agent.img. For instructions on how to build this checkout [This README](../../tools/)

## Set up
1. Build the agent, add to a disk image named agent.img and copy to SD card
2. Copy autorun.ds onto the SD card

## Usage
1. Plug in device
2. Wait for the device to reset / USB drive to disappear
3. Connect to the device's WiFi access point
4. Look for the command output in the logs tab