# Tools

## Serial based agent

Running the following sets with create you a 16MB USB disk image which contains the serial agent

**The agent currently only runs on 64-bit machines**

### Compile the agent

1. Clone the repository
   ```
   git clone https://github.com/i-am-shodan/USBArmyKnife.git
   ```
2. Go into the tools folder and give the craft_agent.sh script permissions:
   ```
   sudo chmod a+x craft_agent.sh
   ```
3. Execute the script:
   ```
   sudo bash craft_agent.sh
   ```
4. This script will install github cli. As soon as this is installd it will ask you to authenticate with your github account, to do this just follow the given instructions.
5. After Authenticating a download menu should open up where you can select several options. You need to choose agent binaries with space and than hit enter to download.
6. Now wait for the script to finish executing and you should have your finished agent.img 

### Notes:
The compilation process does not work on WSL due to WSL no having enough acces. This means that this process only works on a complete Linux install.


## Manual Instructions

The agent is compiled, by default, into Windows native instructions. Cross compilation is not currently supported by dotnet so you'll need to run these steps on a Windows machine.

1. Check out the code
2. Install [Dotnet 8.0 SDK](https://dotnet.microsoft.com/en-us/download/dotnet/8.0)
3. Open up a Terminal and run


```
cd tools\Agent
dotnet publish -r win-x64
```
Your PATH_TO_AGENT_EXECUTABLE value is `.\bin\Release\net8.0-windows\win-x64\publish\`

### Set up a custom disk image

Setting up the disk image is easier to do on Linux/WSL. Run the following commands as root

NOTE: If you get a error about device in use, try another loopback device. You can find the devices in use with `losetup`

```
dd if=/dev/zero bs=1048576 count=500 of=agent.img
losetup /dev/loop0 ./agent.img
parted --script /dev/loop0 mktable msdos mkpart primary 2048s 100%
losetup -d /dev/loop0
losetup -P /dev/loop0 ./agent.img
mkfs.vfat /dev/loop0p1
mkdir /tmp/mnt > /dev/null 2>&1
mount /dev/loop0p1 /tmp/mnt

# It's important to copy all the published files onto the disk image
cp [PATH_TO_AGENT_EXECUTABLE]/* /tmp/mnt/
umount /tmp/mnt
losetup -d /dev/loop0
```

You can now copy agent.img to your SD card.

## Getting debug logs

The agent API enables you to display debug logs from the device. To do this:

1. Edit the powershell script in the `DebugLogs` to point to the COM port of your device. 
2. Run the script `.\get_device_debug_output.ps1` in a PowerShell terminal
3. Connect the device

Sometimes the powershell script might not connect to the device quick enough to collect the debug logs. In that case you need the device to wait until the script has connected. Add the following to your autostart.ds file. This will make the device wait until the PowerShell script has connected.
```
WHILE (AGENT_CONNECTED() == FALSE)
  DELAY 2000
END_WHILE
```
