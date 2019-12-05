# Networks Project 4 Garrett Rudisill

## Run arguments
    python3 server.py <filename>
    python3 client.py <ip> <filename>

## Description
This application implements reliability using UDP sockets. This uses a sliding window go-back-n style algorithm.

The file is broken down into a number of 10 packet windows, then we attempt to send through that window until it is succesfully ack'd that the window has recieved. Otherwise a nack is sent with the last in order packet received to write

## Known Issues
if the ack message is dropped, this can cause issues. Restart will usually fix it