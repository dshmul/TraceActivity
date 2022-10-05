# TraceActivity: PlatformIO project for activity intercepting capability.

Monitor mode firmware in part adapted from https://blog.podkalicki.com/esp32-wifi-sniffer/


DevKit V4 UART 0 pins:
U0RXD   GPIO5       B
U0TXD   GPIO4       W


DevKit V1 UART2 pins:
Rx      GPIO16      W
Tx      GPIO17      B



Idea is to use UART to easily send serialized json from one esp to the next, then read line and serial then output to IoT.