# Debugging Tools

Tools should expose something like the following.

- Listen tools
    + Take in a socket, uds tcp whatever, and a callback. It will call the callback whenever the socket has data.
- Write tools
    + Take in a path or address+port, create the socket, give a method to post data into that socket

