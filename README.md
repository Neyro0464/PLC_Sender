# PLC Sender programm
## Description
The program is designed for remote interaction with the device. 
The program calculates the position of a specific satellite for 24 hours from the moment the program is launched and sends the resulting calculations to the remote device. 
Sending occurs via the ssh/sftp protocol. After sending, the program waits for a response from the device in the udp response format.
## How to build
1) Download all files from repository in one directory.
2) Open console and enter this command (you need permissions to launch script):
- $ ./setup.sh
3) To start program launch PLC_Sender execute file:
- $ ./PLC_Sender
