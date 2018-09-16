# COMP6320LAB1

1. Part 1.b
   - [x] Change echo sample to use UDP
   - [ ] Implement timeout mechanism on client side (don't want to wait for server response indefinitely)
   - [x] Calculate time in milliseconds / convert to network byte order (htonl is too small to handle 64-bit int)
   - [x] Figure out how to pack data into Lab11-RFC format
   - [ ] Is the server supposed to resend the packet exactly as it received it or with new sequence number & timestamp?
2. Part 1.c
   - [ ] Add a child process to handle recvfrom()
3. Part 2
   - [ ] Modify echo sample to use data packed according to Lab12-RFC
   - [ ] Server needs to perform requested operation and reply accordingly
4. Report
   - [ ] Write report
