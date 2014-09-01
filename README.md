labtlm-app
==========

Telemetry output application designed for lab environments using UDP


This is non-flight code intended for use in a prototype or lab environment. This app receives telemetry
packets from the software bus and sends them over a UDP port. This application follows the object-based
application design pattern.

It currently uses the expat XML parser. Using expat is an exploritory exercise first in using text-based
tables that are managed by the applications themselves and second using XML as the format. The labtlm 
table is used to determine which packets are sent out the UDP port.

