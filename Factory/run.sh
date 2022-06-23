#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PARENT="$(realpath $DIR/..)"

source "$PARENT/3rd/bash_framework/lib/oo-bootstrap.sh"

import util/type
import util/log

namespace run

Log::AddOutput run DEBUG

if [ ! -e $PARENT/Build ]; then
    Log "Build directory doesn't exist, creating it..."
    mkdir $PARENT/Build
fi


if [ ! -e $PARENT/Resources ]; then
    Log "Resources directory doesn't exist, creating it..."
    mkdir $PARENT/Resources
fi

if [ ! -e $PARENT/Resources/linux ];  then
    if [ ! -e $PARENT/Resources/master.zip ]; then
        Log "Cloning the latest Utopia Linux kernel..."
        wget https://github.com/UtopiaOS/linux/archive/master.zip -O $PARENT/Resources/master.zip
    else
        Log "Found cached Utopia Linux kernel..."
    fi

    if [ -e $PARENT/Resources/linux ]; then
        Log "Kernel has already been extracted..."
    else
        pushd $PARENT/Resources
            Log "Extracting the kernel..."
            unzip master.zip
            # Delete the kernel zip file
            rm master.zip
        popd
        # TODO: Stop hardcoding the archive name
        mv $PARENT/Resources/linux-master $PARENT/Resources/linux 
    fi
fi

# Pushd into the kernel directory
pushd $PARENT/Resources/linux/src
    Log "Building the kernel..."
    mkdir -p $PARENT/Build/kernel
    # TODO: Stop hardcoding the architecture
    mkdir -p $PARENT/Build/kernel/x86_64
    Log "Copying kernel configuration"
    # TODO: Make the configuration... well, configurable
    cp $PARENT/Resources/linux/Configs/BasicUtopia $PARENT/Build/kernel/x86_64/.config
    WORKSPACE="$PARENT/Build/kernel/x86_64"
    make O=$WORKSPACE -j$(nproc)
popd


BUSYBOX_VERSION="1.35.0"
BUSYBOX_NAME="busybox-$BUSYBOX_VERSION"
BUSYBOX_PKG="$BUSYBOX_NAME.tar.bz2"
BUSYBOX_URL="https://busybox.net/downloads/$BUSYBOX_PKG"

# In the future, we don't use busybox :p
if [ ! -e $PARENT/Resources/$BUSYBOX_PKG ]; then
    Log "Downloading busybox..."
    wget $BUSYBOX_URL -O $PARENT/Resources/$BUSYBOX_PKG
else
    Log "Found cached busybox..."
fi

if [ ! -e $PARENT/Resources/$BUSYBOX_NAME ]; then
    Log "Extracting busybox..."
    tar -xjf $PARENT/Resources/$BUSYBOX_PKG -C $PARENT/Resources
    mv $PARENT/Resources/$BUSYBOX_NAME $PARENT/Resources/busybox
fi

# We need glibc-static-devel on OpenSuse...
pushd $PARENT/Resources/busybox
    Log "Making necessary directories..."
    mkdir -p $PARENT/Build/busybox/x86_64
    Log "Copying busybox configuration"
    cp $PARENT/Meta/configs/BusyBoxDefault $PARENT/Build/busybox/x86_64/.config
    WORKSPACE="$PARENT/Build/busybox/x86_64"
    LDFLAGS="--static" make O=$WORKSPACE -j$(nproc)
    make O=$WORKSPACE -j$(nproc) install
popd

# Make the legacy directory structure
if [ ! -e $PARENT/Root/x86_64 ]; then
    mkdir $PARENT/Root/x86_64
    mkdir -p $PARENT/Root/x86_64/{bin,sbin,etc,proc,sys,usr/{bin,sbin}}
fi

# Copy the resulting busybox binaries to the Root
Log "Copying busybox binaries to the Root..."
cp -av $PARENT/Build/busybox/x86_64/_install/* $PARENT/Root/x86_64

# Make our temporary "init"
cat << EOT >> $PARENT/Root/x86_64/init
#!/bin/sh

mount -t proc none /proc
mount -t sysfs none /sys

echo -e "Welcome to Utopia\n"
echo -e "Is not recommend to distribute this\n"

exec /bin/sh
EOT

chmod +x $PARENT/Root/x86_64/init

pushd $PARENT/Root/x86_64
# Create and compress our initramfs
find . -print0 | cpio --null -ov --format=newc | gzip -9 > $PARENT/Build/initramfs.cpio.gz
popd