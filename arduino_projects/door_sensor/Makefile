flash-config-pa-door:

	# create the bin directory if it does not exist, mklittlefs will not create it and will fail if it's not there
	mkdir -p bin

	# Create an image of the littlefs filesystem, anything under the config directory will be included in the image,
	# such as the config.json, or any AWS certs
	# -c: Path to your data/ directory.
    # -b: Block size (default: 4096 bytes).
    # -p: Page size (default: 256 bytes).
    # -s: Total size of the LittleFS partition (match your partition size: 128 KB = 131072 bytes).

	mklittlefs -c config/pa_door -b 4096 -p 256 -s 131072 bin/littlefs_image_pa_door.bin
	echo "mklittlefs done\n"

	# Flash the littlefs image to the ESP32
	esptool.py --chip esp32 --port /dev/cu.usbserial-02A9B710 write_flash 0x3E0000 bin/littlefs_image_pa_door.bin

flash-config-big-roller-door:

	# create the bin directory if it does not exist, mklittlefs will not create it and will fail if it's not there
	mkdir -p bin

	# Create an image of the littlefs filesystem, anything under the config directory will be included in the image,
	# such as the config.json, or any AWS certs
	# -c: Path to your data/ directory.
    # -b: Block size (default: 4096 bytes).
    # -p: Page size (default: 256 bytes).
    # -s: Total size of the LittleFS partition (match your partition size: 128 KB = 131072 bytes).

	mklittlefs -c config/big_roller_door -b 4096 -p 256 -s 131072 bin/littlefs_image_big_roller_door.bin
	echo "mklittlefs done\n"

	# Flash the littlefs image to the ESP32
	esptool.py --chip esp32 --port /dev/cu.usbserial-02A98D2F write_flash 0x3E0000 bin/littlefs_image_big_roller_door.bin
