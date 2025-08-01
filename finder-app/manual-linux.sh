#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

# Convert to absolute path
OUTDIR=$(realpath -m "${OUTDIR}")

mkdir -p ${OUTDIR}

# Check if directory creation was successful
if [ ! -d "${OUTDIR}" ]; then
    echo "ERROR: Failed to create directory ${OUTDIR}"
    exit 1
fi

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux/arch/${ARCH}/boot/Image ]; then
    cd linux
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    echo "Building kernel"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    # Skip modules_install as instructed
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux/arch/${ARCH}/boot/Image ${OUTDIR}/

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    rm -rf ${OUTDIR}/rootfs
fi

# Create necessary base directories
mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    echo "Cloning busybox"
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
else
    cd busybox
    echo "Busybox directory exists, cleaning and rebuilding"
fi

# Configure and build busybox
make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

# Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
echo "Sysroot: ${SYSROOT}"

# Copy program interpreter
cp ${SYSROOT}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib/

# Copy shared libraries
cp ${SYSROOT}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64/
cp ${SYSROOT}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64/
cp ${SYSROOT}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64/

# Make device nodes (may require root privileges in some environments)
cd ${OUTDIR}/rootfs
if ! mknod -m 666 dev/null c 1 3 2>/dev/null; then
    echo "Warning: Could not create /dev/null device node, may need root privileges"
fi
if ! mknod -m 666 dev/console c 5 1 2>/dev/null; then
    echo "Warning: Could not create /dev/console device node, may need root privileges"
fi

# Clean and build the writer utility
cd ${FINDER_APP_DIR}
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer ${OUTDIR}/rootfs/home/
cp writer.sh ${OUTDIR}/rootfs/home/
cp finder.sh ${OUTDIR}/rootfs/home/
cp finder-test.sh ${OUTDIR}/rootfs/home/
cp autorun-qemu.sh ${OUTDIR}/rootfs/home/

# Copy conf files
mkdir -p ${OUTDIR}/rootfs/home/conf
cp ../conf/username.txt ${OUTDIR}/rootfs/home/conf/
cp ../conf/assignment.txt ${OUTDIR}/rootfs/home/conf/

# Modify finder-test.sh to reference conf/assignment.txt instead of ../conf/assignment.txt
sed -i 's|../conf/assignment.txt|conf/assignment.txt|g' ${OUTDIR}/rootfs/home/finder-test.sh

# Make scripts executable
chmod +x ${OUTDIR}/rootfs/home/writer.sh
chmod +x ${OUTDIR}/rootfs/home/finder.sh
chmod +x ${OUTDIR}/rootfs/home/finder-test.sh
chmod +x ${OUTDIR}/rootfs/home/autorun-qemu.sh

# Chown the root directory (may require root privileges in some environments)
cd ${OUTDIR}/rootfs
if ! chown -R root:root * 2>/dev/null; then
    echo "Warning: Could not change ownership to root, may need root privileges"
    echo "Files will be owned by current user: $(whoami)"
fi

# Create initramfs.cpio.gz
find . | cpio -o -H newc | gzip > ${OUTDIR}/initramfs.cpio.gz