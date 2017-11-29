deps_config := \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/app_trace/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/aws_iot/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/bt/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/esp32/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/ethernet/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/fatfs/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/freertos/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/log/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/lwip/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/mbedtls/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/openssl/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/spi_flash/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/tcpip_adapter/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/wear_levelling/Kconfig \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/sferrar0/Lua-Rtos-esp32/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
