ESP8266
=======

All things related to ESP8266 low cost device

ESP8266.ino is a mega sketch to show basic server capability.

Basic theory of UDP operation:
   Android device sends out a UDP broadcast to local network (i.e. 192.168.0.255).  The message is findkey where key 
   is a unique name of the device (i.e. garageDoor).
   
   All units will receive this message but only the unit with the matching key will respond.  The response is: "CTS" (clear to send).
   
   When the android device receives the message it now also has the ip address of the device.  All further communication will
   be directly from the android device to the specific ip address of the desired unit.

   The unit will now respond to commands, like "open" or "readTemp".

   I think it will scale nicely.   