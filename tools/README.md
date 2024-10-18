# Agent set up

Running the following sets with create you a 16MB USB disk image which contains the serial agent

## Compile the agent

The agent is compiled, by default, into Windows native instructions. Cross compilation is not currently supported by dotnet so you'll need to run these steps on a Windows machine.

1. Check out the code
2. Install [Dotnet 8.0 SDK](https://dotnet.microsoft.com/en-us/download/dotnet/8.0)
3. Open up a Terminal and run


```
cd tools\SerialAgent
dotnet publish -r win-x64
```
Your PATH_TO_AGENT_EXECUTABLE value is `.\bin\Release\net8.0-windows\win-x64\publish\`

## Set up a custom disk image

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