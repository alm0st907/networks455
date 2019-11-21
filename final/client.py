from scapy.all import *
import socket
import sys

global debug

if __name__ == "__main__":
    debug = 0
    file = open('tux.txt','r')
    lines = file.readlines()
    if debug:
        for line in lines:
            print(line,end="") #establish no newline on end
    if(len(sys.argv) < 3 and not debug):
        print("ARGS <ip> <filename>")
    