#Download gh cli
(type -p wget >/dev/null || (sudo apt update && sudo apt-get install wget -y)) \
        && sudo mkdir -p -m 755 /etc/apt/keyrings \
        && out=$(mktemp) && wget -nv -O$out https://cli.github.com/packages/githubcli-archive-keyring.gpg \
        && cat $out | sudo tee /etc/apt/keyrings/githubcli-archive-keyring.gpg > /dev/null \
        && sudo chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg \
        && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
        && sudo apt update \
        && sudo apt install gh -y

#Clone Repository
git clone https://github.com/i-am-shodan/USBArmyKnife.git

#Donwload gent binaries
cd USBArmyKnife
gh auth login
gh run download

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
cp PortableApp.dll turbojpeg.dll vcruntime140.dll in1.bat /tmp/mnt/

# Cleanup
umount /tmp/mnt
losetup -d "$LOOP_DEVICE"
