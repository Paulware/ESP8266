#include "PSTRStrings1.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>

PSTRStrings1 wifiCommands(5);   // WIFI commands
PSTRStrings1 response(5);       // Responses
PSTRStrings1 serialCommands(5); // Serial Commands

SoftwareSerial mySerial (2,4); // RX, TX
  
boolean silentRunning = true;
boolean menu = false;
int state = 0;
byte leftOffset = 0;
byte rightOffset = 0;
char serialString[20];
int serialIndex = 0;
int eepromIndex = 0;

/* This could be a pwm class */
unsigned long pwmTimeout = 0;

#define LEFTFORWARD 9
#define LEFTBACKWARD 10
#define RIGHTFORWARD 6
#define RIGHTBACKWARD 5
#define STEPTIME 300 
#define STRAIGHTSPEED 200
#define TURNSPEED 120
#define TURNTIME 100

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
  serialCommands.addString ( PSTR ( "AT+" ) );
  
  Serial.begin (115200);
  pinMode (13,OUTPUT);
  pinMode (12,OUTPUT);
  digitalWrite (13,1);
  digitalWrite (12,0);
  digitalWrite (13,0);
  
  mySerial.begin (19200);
  leftOffset = EEPROM.read(40);
  if (leftOffset == 255) { // Uninitialized
     EEPROM.write ( 0,leftOffset );
  }
  rightOffset = EEPROM.read (41);
  if (rightOffset == 255) { // Uninitialized 
     EEPROM.write ( 0, rightOffset );
  }
   
  Serial.println ( "\nReady" );
}

void sendSsidPassword () {
  char cmd[] = "AT+CWJAP=\"                                      ";
  int index = 0;
  int cmdIndex = 10;
  char ch;
  
  while (true) {
     ch = EEPROM.read ( index );  
     Serial.println ( (int) ch );
     index = index + 1;
     if (ch == 0)
        break;
        
     cmd[cmdIndex] = ch;
     cmdIndex = cmdIndex + 1;
  }
  cmd[cmdIndex] = '\"';
  cmdIndex = cmdIndex + 1;
  cmd[cmdIndex] = ',';
  cmdIndex = cmdIndex + 1;
  cmd[cmdIndex] = '\"';
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
  cmd[cmdIndex] = '\"';
  cmdIndex = cmdIndex + 1;
  cmd[cmdIndex] = 0;
  Serial.println ( cmd );
  sendCommand ( cmd, true );
}

void launch () {
  sendCommand ( "AT", true );
  digitalWrite (13,0);
  sendCommand ( "AT+RST", false );
  digitalWrite (13,1);
  sendCommand ( "AT+CWMODE=3", true);
  digitalWrite (13,1);
  sendCommand ( "AT+CWLAP", true );
  sendSsidPassword ();
  digitalWrite (13,0);
  sendCommand ( "AT+CIPMUX=1",true);
  digitalWrite (13,1);
  sendCommand ( "AT+CIPSERVER=1,3333", true);
  digitalWrite (13,0);
  sendCommand ( "AT+CIPSTO=120",true );
  digitalWrite (13,1);
  Serial.println ( "\nReady" );  
}

void getResponse (boolean waitOK) {
  unsigned long startTime;
  char ch;
  int index;
  bool gotResponse = false;
  
  startTime = millis();
  // Wait 10 seconds maximum
  while (millis() < (startTime + 10000))
     if (mySerial.available())
     {
       ch = mySerial.read();
       Serial.print ( ch );
       index = response.matchString (ch, false);
       gotResponse = true;
       if (index == 2) // no change
          break;
       if (waitOK) {
         if (index == 0) 
            break;
       } else { // wait for ready          
         if (index == 1)
            break;
       }
     }     
}

void sendCommand ( char * cmd,  boolean wait ) {
  Serial.print ( "Send command: " );
  Serial.println ( cmd );
  mySerial.println ( cmd );
  getResponse(wait);  
}

void respond(char * response, boolean force) {
  char ch;
  char cmd[]="AT+CIPSEND=0,     ";
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
    delay (150);
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
  
  if (Serial.available()) {
     ch = Serial.read();
     if ((ch == 13) || (ch == 10)) {
        if (strlen (serialString) > 0) {
          len = strlen (serialString);
          match = -1;
          for (i=0; i<len; i++) {
            ch = serialString[i];
            if (match == -1) {               
               match = serialCommands.matchString (ch,false);
               if ((match == 3) || (match == 0)) 
                  break; // AT+ or launch
            } else {
               switch (match) {
                  case 1: 
                    Serial.print ( "SSID:" );
                    EEPROM.write ( eepromIndex,ch );
                    eepromIndex = eepromIndex + 1;
                    EEPROM.write ( eepromIndex, 0 );
                    Serial.println ( ch );
                  break;
                  case 2:
                    Serial.print ( "Password:" );
                    EEPROM.write (20+eepromIndex,ch);
                    eepromIndex = eepromIndex + 1;
                    EEPROM.write ( 20+eepromIndex,0);
                    Serial.println ( ch );
                  break;
                  default:
                    Serial.print ( "Ignored: " );
                    Serial.println ( ch );
                  break;
               }
            }                
          }            
          if (match == 0) 
            launch();
          else if ((match == 1) || (match == 2))
            eepromIndex = 0;
          else if (match == 3)
            sendCommand ( serialString, true );
                   
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
      switch (index) {
       case 0: // Android device is looking for me
          respond ("CTS", true);   
          digitalWrite (12,1);        
          break;
       case 1: // Command 1
          respond ("ACK", false);
          digitalWrite (13,0);
          break;
        case 2: // Command 2
          respond ("ACK", false );
          digitalWrite (13,1);
         break;
        case 3: // Command 3
          respond ("ACK",false);
          digitalWrite (13,0);
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

