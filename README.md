# TraceActivity: PlatformIO Project for Trace Activity Detection Capability

This repository contains ESP32 firmware that enables the interception of probe requests and sending of data to AWS.

The capability is divided among two ESP32s, one that acts as the receiver - intercepting probe requests, and the other as the trasnmitter - sending packets to AWS IoT Core. 

The wifi module of the receiver is put into promiscuous mode, otherwise known as monitor mode, which allows the CPU to intercept all netwrok packets in its proximity. These packets are then filtered down to probe requests which are serialized into json format and sent to the transmitting device via UART pins.

The transmitter wifi module is put into access point mode which allows it to connect to a network. It reads the probe request packets sent to it from the receiver device through UART and publishes it to AWS IoT Core using the MQTT protocol. 

Directions to Flash Devices:
1. In config.h file assign the respective device number
2. Using [PlatformIO](https://platformio.org/install/ide), flash respective device

Resources:
- https://blog.podkalicki.com/esp32-wifi-sniffer/
