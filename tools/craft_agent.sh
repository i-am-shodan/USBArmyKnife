# Find a free loop device
LOOP_DEVICE=$(losetup -f)

# Create disk image
dd if=/dev/zero bs=1048576 count=501 of=agent.img
losetup "$LOOP_DEVICE" ./agent.img

# Partition the image
parted --script "$LOOP_DEVICE" mktable msdos mkpart primary 2048s 100%
losetup -d "$LOOP_DEVICE"
losetup -P "$LOOP_DEVICE" ./agent.img

# Format the partition
mkfs.vfat "${LOOP_DEVICE}p1"

# Mount and copy files
mkdir -p /tmp/mnt
mount "${LOOP_DEVICE}p1" /tmp/mnt
cp agent/* /tmp/mnt/

# Cleanup
umount /tmp/mnt
losetup -d "$LOOP_DEVICE"
