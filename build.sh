#!/bin/sh

rm u-boot.*
BASE_DIR="/home/vincent/ndg/bootloader/ndg_edison-u-boot"
TOOLS_DIR="/home/vincent/ndg/bootloader/ndg_edison-u-boot/tools/xfstk-stitcher/"

make clean ; make edison_config; make -j

BASE_IMAGE="${BASE_DIR}/u-boot.bin"
NO_OSIP_IMAGE="${BASE_IMAGE}.no-osip"
FINAL_IMAGE="${BASE_IMAGE}.osip"

cat u-boot.bin | dd of=${NO_OSIP_IMAGE} bs=4096 seek=1


${TOOLS_DIR}/gen_os --input ${NO_OSIP_IMAGE} --output ${FINAL_IMAGE} --xml ${TOOLS_DIR}/saltbay.XML
