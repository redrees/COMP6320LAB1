# COMP6320LAB1

1. Part 1.b
   - [x] Change echo sample to use UDP
   - [x] Implement timeout mechanism on client side (don't want to wait for server response indefinitely)
   - [x] Calculate time in milliseconds / convert to network byte order (htonl is too small to handle 64-bit int)
   - [x] Figure out how to pack data into Lab11-RFC format
   - [x] Is the server supposed to resend the packet exactly as it received it or with new sequence number & timestamp?
   - [x] clean out code (take out fflush from server)
2. Part 1.c
   - [x] Add a child process to handle recvfrom()
   - [x] Tested on TUX machine, client11c is working as intended.
   - [ ] add statistics output
3. Part 2
   - [x] Modify echo sample to use data packed according to Lab12-RFC
   - [x] Server needs to perform requested operation and reply accordingly
   - [x] client12.c -> fix line 75: checks multiplication operator twice instead of subtraction
4. Report
   - [ ] Write report
