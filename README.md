Programs to exercise Apache Traffic server.

ssl-session-reuse-client creates ssl connection to a server.  Then harvests
that SSL_SESSION and reuses it on subsequent connections to the server. In
the session reuse case, the first data packet is often read with the last 
handshake packet.  There are concerns that ATS is not always handling this
case correctly.

ssl-write is another version of the ssl-session-reuse-client program that 
uses SSL_write/SSL_read during the handshake phase instead of SSL_connect.

In both programs it has be observed via wireshark and by debug messages in 
ATS that the first data packet and the last handshake packet in the SSL
session reuse case are read by ATS in the same read.
