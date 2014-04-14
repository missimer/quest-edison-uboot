#!/bin/sh

rm u-boot.*
BASE_DIR="/home/vincent/ndg/bootloader/ndg_edison-u-boot"
TOOLS_DIR="/home/vincent/ndg/bootloader/ndg_edison-u-boot/tools"
XFSTK_DIR="${TOOLS_DIR}/xfstk-stitcher/"

make clean ; make edison_config; make -j

BASE_IMAGE="${BASE_DIR}/u-boot.bin"
ENV_FILE="${BASE_DIR}/arch/x86/cpu/tangier/default.env"
ENV0_IMAGE="${BASE_DIR}/env0.bin"
ENV1_IMAGE="${BASE_DIR}/env1.bin"
NO_OSIP_IMAGE="${BASE_IMAGE}.no-osip"
NO_OSIP_IMAGE_TMP="${BASE_IMAGE}.no-osip-tmp"
FINAL_IMAGE="${BASE_IMAGE}.osip"

#env size is the one defined in edison.h
ENV_SIZE=0x10000
${TOOLS_DIR}/mkenvimage -s ${ENV_SIZE} ${ENV_FILE} -o ${ENV0_IMAGE}
${TOOLS_DIR}/mkenvimage -s ${ENV_SIZE} ${ENV_FILE} -r -o ${ENV1_IMAGE}

# FIXME: u-boot backup needs to have its own OSIP header
cat u-boot.bin | dd of=${NO_OSIP_IMAGE_TMP} bs=4096 seek=1
dd if=/dev/zero of=${NO_OSIP_IMAGE} bs=8M count=1
dd if=${NO_OSIP_IMAGE_TMP} of=${NO_OSIP_IMAGE} bs=1M conv=notrunc seek=0
dd if=${ENV0_IMAGE} of=${NO_OSIP_IMAGE} bs=1M conv=notrunc seek=2
dd if=${ENV1_IMAGE} of=${NO_OSIP_IMAGE} bs=1M conv=notrunc seek=5
${XFSTK_DIR}/gen_os --input ${NO_OSIP_IMAGE} --output ${FINAL_IMAGE} --xml ${TOOLS_DIR}/saltbay.XML
