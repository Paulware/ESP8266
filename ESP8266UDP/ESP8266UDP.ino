#include "PSTRStrings1.h"
PSTRStrings1 commands(5);   // BASIC commands
  
boolean initialized = false;
int state = 0;

void setup() {
  // change "key" to a unique string
  commands.addString ( PSTR ( "findkey"));   //  0
  commands.addString ( PSTR ( "cmd1" ));     //  1
  commands.addString ( PSTR ( "cmd2" ));     //  2
  commands.addString ( PSTR ( "cmd3" ));     //  3
  Serial.begin (115200);
  Serial.println ( "Ready2" );
  Serial2.begin (115200);
  sendCommand ( "AT" );
  delay (1000);
  sendCommand ( "AT+RST" );
  delay (8000);
  //sendCommand ( "AT+CWLAP" );
  //delay (1000);
  sendCommand ( "AT+CWJAP=\"SSID\",\"Password\"" );
  delay (1000);
  sendCommand ( "AT+CIPMUX=1" );
  delay (4000);
  sendCommand ( "AT+CIPSERVER=1,3333" );
  delay (1000);
  sendCommand ( "AT+CIPSTO=120" );
  delay (1000);
  //sendCommand ("ATE");
  //delay (1000);
  initialized = true;
  Serial.println ( "\nReady" );
}

void getResponse () {
  unsigned long startTime;
  char ch;
  char lastCh;
  
  startTime = millis();
  lastCh = ' ';
  while (millis() < (startTime + 3000))
     if (Serial2.available() )
     {
       ch = Serial2.read();
       Serial.print (ch);
       if ((lastCh == 'O') && (ch == 'K'))
          break;
       lastCh = ch;
     }
}

void sendCommand ( char * cmd ) {
  Serial2.println ( cmd );
  getResponse();  
}

void respond(char * response) {
  char ch;
  int len = strlen (response);
  Serial2.print ( "AT+CIPSEND=0,");
  Serial2.println ( len+2 );
  delay (50);
  Serial2.println (response);
  Serial2.println ();
  delay (50);
  Serial.println (response);   
  delay (100);
  Serial2.println ( "AT+CIPCLOSE=1");
  delay (1000);
}

void loop () {
  char ch;
  int index;
  boolean debugIt = false;
  if (initialized) 
  {     
    if (Serial.available() )
    {
       ch = Serial.read();
       Serial2.print (ch);
    } 
 
    if (Serial2.available() )
    {
      ch = Serial2.read();
      if (debugIt)
        Serial.print (ch);
      index = commands.matchString (ch, false);
      if (index > -1) {
        Serial.print ( "Index: " );
        Serial.println ( index );
        
        switch (index) {
          case 0: // Android device is looking for me
            respond ("CTS");           
            break;
          case 1: // Command 1
            respond ("ACK1" );
            break;
          case 2: // Command 2
            respond ("ACK2" );
            break;
          case 3: // Command 3
            respond ("ACK3" );
            break;
          default:
            break; // Do nothing
        }    
      }
    }   
  } 
}

