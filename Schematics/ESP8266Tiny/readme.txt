10/19/2014
   Created
   Not sure whether the LM 3.3V voltage regulator can provide enough for the ESP8266
   Added a 3.3V Pinout so that an external 3.3V Power source can be provided.
   Will order after testing out the ESP8266 Rx/Tx lines are connected to Arduino
     Rx/Tx so ESP8266 module should be removed will programming.     
   Could be a useful module for an I2C component.
   
10/21/2014
   Confirmed LM 3.3V regulator does indeed provide enough power for this device.
   Transistors were added so that the 5V arduino can communicate with the 3.3V device
   The ESP8266 will need the RX/TX lines, so an I2C component may be necessary
   Go ahead an order the pcb
   
10/22/2014
   PCB ordered from iteadstudio
   
10/23/2014
   Transistor to amplify power to infra-red led with supporting resistors should be
   placed on the main board.  Board ready.  Wait until first board is returned for 
   evaluation before ordering another.
   
11/10/2014
   D0 was driving ESPTX.  (backwards).   Reordering
   
11/18/2014
   AT+CIOBAUD=19200 sets the baud rate to 19200.  This will allow me to communicate with the 
   ESP8266 using soft serial.  Normal Arduino Serial can be used for debugging (better).
   Reordered pcb.  D11 and D12 are now connected to ESP8266
 
12/12/2014
   Redid the .zip file, but this was unnecessary as the last change has not arrived for evaluation yet. 
   
12/31/2014
   Last change arrived (some problem in transportation).
   pcb checks out ok.  Some standoffs would be nice.
   