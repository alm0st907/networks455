# Networks Project 4 Garrett Rudisill

## Run arguments
    python3 server.py <filename>
    python3 client.py <ip> <filename>

## Description
This application implements reliability using UDP sockets. This uses a sliding window go-back-n style algorithm.

The file is broken down into a number of 10 packet windows, then we attempt to send through that window until it is succesfully ack'd that the window has recieved. Otherwise a nack is sent with the last in order packet received to write

## Known Issues
I have had issues with duplicate data arriving or occassionally a couple packets dropped, even though the debug prints say that the packets are being ack'd successfully and I have a clause to prevent writing if not properly acked. I was unable to debug, but otherwise everything else is implemented.

The issue may be due to blocking/nonblocking issues with the python sockets which I have not been able to debug, thus duplicate packets get sent