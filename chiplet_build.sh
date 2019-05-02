# Linaro
#export CROSS_COMPILE=aarch64-linux-gnu-
# Yocto
export CROSS_COMPILE=aarch64-poky-linux-
make PLAT=hpsc bl31
#make DEBUG=1 PLAT=hpsc bl31
#make PLAT=zynqmp LOG_LEVEL=99 bl31
aarch64-linux-gnu-objdump -D build/hpsc/debug/bl31/bl31.elf > bl31-debug.list
