#!/bin/bash
# returns "no device present" if not device could be found
# i.e. jumper reset hasnt worked
echo "Testing if device present"
sudo dfu-programmer atmega32u4 get bootloader-version --debug 1

# flashes atmel to state when it was delivered
echo "Resetting AVR"
sudo dfu-programmer atmega32u4 erase --debug 1

echo "Flashing AVR bootloader"
sudo dfu-programmer atmega32u4 flash ./USBtoSerial.hex --debug 1


