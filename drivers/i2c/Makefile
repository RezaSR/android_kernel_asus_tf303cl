#
# Makefile for the i2c core.
#

obj-$(CONFIG_I2C_BOARDINFO)	+= i2c-boardinfo.o
obj-$(CONFIG_I2C)		+= i2c-core.o
obj-$(CONFIG_I2C_SMBUS)		+= i2c-smbus.o
obj-$(CONFIG_I2C_CHARDEV)	+= i2c-dev.o
obj-$(CONFIG_I2C_MUX)		+= i2c-mux.o
obj-y				+= algos/ busses/ muxes/
obj-$(CONFIG_I2C_STUB)		+= i2c-stub.o
obj-$(CONFIG_ASUS_I2C_MONITOR)	+= asus-i2c-bus-monitor.o

ccflags-$(CONFIG_I2C_DEBUG_CORE) := -DDEBUG
CFLAGS_i2c-core.o := -Wno-deprecated-declarations
