Temperature Sensor
==========

This is a simple project with an ESP32 and BME280 sensor that is able to measure temperature and pressure. This project will send the reading every few minutes to my home assistant setup via MQTT.

I have these placed in various parts of my house & shed.

How to install
===
1. Deploy via Arduino IDE, making sure you select "Custom" partition type. It will fail to run on the first time, but we need that initial install to setup the `partitions.csv` on the device
2. Set your config in `config.json`
3. Run `make flash-config-shed-inside`, this will copy the configuration to non volatile memory on the device.
4. Deploy again via Arduino IDE, this time it will work because the configuration file is there.
