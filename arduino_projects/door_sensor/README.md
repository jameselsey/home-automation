Door Sensor
==========

This is a simple project with an ESP32 and a reed sensor that can be attached to a door or window, and when the door is opened or closed, it will send a notification via discord.

This is a very cheap (sub $10) security system.

How to install
===
1. Deploy via Arduino IDE, making sure you select "Custom" partition type. It will fail to run on the first time, but we need that initial install to setup the `partitions.csv` on the device
2. Set your config in `config.json`
3. Run `make flash-config-pa-door`, this will copy the configuration to non volatile memory on the device.
4. Deploy again via Arduino IDE, this time it will work because the configuration file is there.
