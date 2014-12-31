#include "PSTRStrings1.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <MemoryFree.h>

PSTRStrings1 wifiCommands(5);   // WIFI commands
PSTRStrings1 response(5);       // Responses
PSTRStrings1 serialCommands(5); // Serial Commands

SoftwareSerial mySerial (12,11); // RX, TX
  
boolean silentRunning = true;
boolean menu = false;
int state = 0;
int eepromIndex = 0;

void setup() {
  // change "key" to a unique string
  wifiCommands.addString ( PSTR ( "findkey"));   //  0
  wifiCommands.addString ( PSTR ( "cmd1" ));     //  1
  wifiCommands.addString ( PSTR ( "cmd2" ));     //  2
  wifiCommands.addString ( PSTR ( "cmd3" ));     //  3
   
  response.addString ( PSTR ( "OK" ));
  response.addString ( PSTR ( "ready" ));
  response.addString ( PSTR ( "no change" ));
  response.addString ( PSTR ( "busy inet..." ));   
  
  serialCommands.addString ( PSTR ( "launch" ) );
  serialCommands.addString ( PSTR ( "ssid " ) );
  serialCommands.addString ( PSTR ( "password " ) );
  serialCommands.addString ( PSTR ( "key " ) );
  serialCommands.addString ( PSTR ( "AT+" ) ); 
  
  Serial.begin (115200);
  
  Serial.println ( "\nAT+CIOBAUD? for baudrate\nReady" );
  Serial.println ( "User Commands: " );
  serialCommands.showStrings();
  Serial.println ( "Memory free: " );
  Serial.println ( freeMemory() );
  mySerial.begin (9600);
}

void getResponse (boolean waitOK) {
  
  unsigned long startTime;
  char ch;
  int index;
  bool gotResponse = false;
  
  startTime = millis();
  // Wait 4 seconds maximum
  while (1) {
     if (millis() > (startTime + 10000)) 
     {
       Serial.println ( "10 seconds expired" );
       break;
     }
     if (mySerial.available())
     {
       ch = mySerial.read();
       Serial.print ( ch );
       index = response.matchString (ch, false);
       gotResponse = true;
       if (index == 2) { // no change
          //Serial.println ( "Found no change" );
          break;
       }   
          
       if (waitOK) {
         if (index == 0) { 
            //Serial.println ( "Found OK" );
            break;
         }   
       } else { // wait for ready          
         if (index == 1) {
            //Serial.println ( "Found ready" );
            break;
         }   
       }
     } 
   }    
   Serial.println ( "\nResponse Received" );  
}

void sendCommand ( char * cmd,  boolean wait ) {
  
  Serial.print ( "Send command: " );
  Serial.println ( cmd );
  mySerial.println ( cmd );
  getResponse(wait);  
}

void showEeprom (int index ) { 
  
  char ch;
  ch = EEPROM.read ( index );  
  while (int(ch) != 0) {
     Serial.print (  ch );
     index = index + 1;
     ch = EEPROM.read ( index );  
  }
  Serial.println ();
  
}

void sendSsidPassword () {
  
  char cmd[] = "AT+CWJAP=\"                                     ";
  int index = 0;
  int cmdIndex = 10;
  char ch;
  
  while (true) {
     ch = EEPROM.read ( index );  
     index = index + 1;
     if (ch == 0)
        break;
        
     cmd[cmdIndex] = ch;
     cmdIndex = cmdIndex + 1;
  }
  cmd[cmdIndex] = 34; // "
  cmdIndex = cmdIndex + 1;
  cmd[cmdIndex] = ',';
  cmdIndex = cmdIndex + 1;
  cmd[cmdIndex] = 34; // '\"';
  cmdIndex = cmdIndex + 1;
  
  index = 0;  
  while (true) {
    ch = EEPROM.read (index + 20);
    index = index + 1;
    if (ch == 0)
       break;
    cmd[cmdIndex] = ch;
    cmdIndex = cmdIndex + 1;
  }
  cmd[cmdIndex] = 34; // '\"';
  cmdIndex = cmdIndex + 1;
  cmd[cmdIndex] = 0;
  sendCommand ( cmd, true ); 
}

void launch () {  
  Serial.println ( "Initialize the ESP8266" );
  sendCommand ( "AT", true );
  sendCommand ( "AT+RST", false );
  sendCommand ( "AT+CWMODE=3", true);
  sendCommand ( "AT+CWLAP", true );
  // Send the SSID and Password from EEPROM
  sendSsidPassword ();
  sendCommand ( "AT+CIPMUX=1",true);
  sendCommand ( "AT+CIPSERVER=1,3333", true); // Port 3333
  sendCommand ( "AT+CIPSTO=120",true );
  Serial.print ( "Launch complete");  
}

void respond(char * response, boolean force) {
  
  char ch;
  char cmd[]="AT+CIPSEND=0,     ";
  Serial.print ( "Sending response: " );
  Serial.println ( response );
  int len = strlen (response) + 2;
  if (!silentRunning || force) {
    if (len>9) {
      cmd[13] = '1';
      cmd[14] = '0' + len - 10;
      cmd[15] = 0;
    } else {
      cmd[13] = '0' + len;
      cmd[14] = 0;
    }  
    mySerial.println (cmd);
    delay (150);
    mySerial.println (response);
    //mySerial.println (); // necesary?
    Serial.println ( response );
    delay (500);
    mySerial.println ( "AT+CIPCLOSE=1");
    delay (100);
  }    
}

void loop () {  
  char ch;
  int index;
  static boolean lastMenu = false;
  static int state = 0;
  int match = 0;
  int len;
  int i;
  static char serialString [30];
  static int serialIndex = 0;
  
  if (Serial.available()) {
     ch = Serial.read();
     //Serial.print ( "first read: " );
     Serial.print ( ch );
     if ((ch == 13) || (ch == 10)) {
        //Serial.println ( "Got a CR or LF" );
        if (strlen (serialString) > 0) {
          len = strlen (serialString);
          Serial.print ( "serialIndex: " );
          Serial.print ( serialIndex );
          Serial.print ( " len: " );
          Serial.print ( len );
          Serial.print ( " serialString: " );
          Serial.println ( serialString );
          match = -1;
          for (i=0; i<len; i++) {
            ch = serialString[i];
            if (match == -1) {               
               match = serialCommands.matchString (ch,false);
               if ((match == 4) || (match == 0)) 
                  break; // AT+ or launch
               
            } else {
               switch (match) {
                  case 1:
                    EEPROM.write ( eepromIndex,ch );
                    eepromIndex = eepromIndex + 1;
                    EEPROM.write ( eepromIndex, 0 );
                  break;
                  case 2:
                    EEPROM.write (20+eepromIndex,ch);
                    eepromIndex = eepromIndex + 1;
                    EEPROM.write ( 20+eepromIndex,0);
                  break;
                  case 3: 
                    EEPROM.write (40+eepromIndex,ch);
                    eepromIndex = eepromIndex + 1;
                    EEPROM.write (40+eepromIndex,0);
                  break;
                  default:
                    Serial.print ( "Ignored: " );
                    Serial.println ( ch );
                  break;
               }
            }                
          }  
          
          switch (match) {
            case 0: // launch
               Serial.println ( "Calling launch" );
               launch();
            break;
            case 1: // ssid
               Serial.print ( "EEPROM SSID:" );
               showEeprom (0);
               eepromIndex = 0;
               break;
            case 2: // password
               Serial.print ( "EEPROM Password:" );
               showEeprom (20);
               eepromIndex = 0;
               break;
            case 3: // key
               Serial.print ( "EEPROM Key:" );
               showEeprom (40);
               eepromIndex = 0;
               break;
            case 4: // AT+
               sendCommand (serialString, true );
               break;
            default:
               break;
          }     
          
          serialString[0] = 0; // Clear the string 
          serialIndex = 0;
        }  
     } else {
        serialString[serialIndex] = ch;
        serialIndex = serialIndex + 1;
        serialString[serialIndex] = 0;
     }   
  } 

  if (mySerial.available() )
  {
    ch = mySerial.read();
    Serial.print ( ch );
    index = wifiCommands.matchString (ch, false);
    if (index > -1) {        
      Serial.println ();
      switch (index) {
       case 0: // Android device is looking for me
          respond ("CTS", true);   
          break;
       case 1: // Command 1
          respond ("ACK", false);
          break;
        case 2: // Command 2
          respond ("ACK", false );
         break;
        case 3: // Command 3
          respond ("ACK",false);
          break;
        case 4: // menu
          menu = true;
          break;          
        default:
          break; // Do nothing
      }    
    }
  }   
}
