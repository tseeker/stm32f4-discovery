#
# Note to self: source this.
#

ARM_TC_PREFIX=/opt/ARM/arm-linaro-eabi-4.6
export PATH=$PATH:$ARM_TC_PREFIX/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ARM_TC_PREFIX/lib

function flash_prog() {
	st-flash write build/$1.bin 0x8000000
}
