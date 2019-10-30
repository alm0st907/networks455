# Garrett Rudisill Project 3 Networks 455

## How to run
    python3 proj3 recv
    python3 proj3 send <dest_ip> <router_ip> <message>
Interfaces are automatically grabbed programmatically

---

## Dependencies
Run with python 3. Needs scapy, netifaces, and netaddr

---
## Known Issues:
IP checksums are different by 0x0100 when sent to different subnet.