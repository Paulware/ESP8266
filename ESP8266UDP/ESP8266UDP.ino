#include "PSTRStrings1.h"
PSTRStrings1 commands(5);   // BASIC commands
  
boolean initialized = false;
boolean silentRunning = true;
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
  pinMode (13,OUTPUT);
  pinMode (12,OUTPUT);
  digitalWrite (13,0);
  digitalWrite (12,0);
  sendCommand ( "AT", 1000, false );
  digitalWrite (13,0);
  sendCommand ( "AT+RST", 5000, true );
  digitalWrite (13,1);
  sendCommand ( "AT+CWMODE=3",1000, false );
  digitalWrite (13,1);
  sendCommand ( "AT+CWLAP", 4000, true );
  sendCommand ( "AT+CWJAP=\"SSID\",\"Password\"", 1000, false );
  digitalWrite (13,0);
  sendCommand ( "AT+CIPMUX=1", 1000, true );
  digitalWrite (13,1);
  sendCommand ( "AT+CIPSERVER=1,3333", 1000, true );
  digitalWrite (13,0);
  sendCommand ( "AT+CIPSTO=120", 1000, true );
  digitalWrite (13,1);
  //sendCommand ("ATE");
  //delay (1000);
  initialized = true;
  Serial.println ( "\nReady" );
}

void getResponse (int timeout, boolean wait) {
  unsigned long startTime;
  char ch;
  char lastCh;
  
  startTime = millis();
  lastCh = ' ';
  while (millis() < (startTime + timeout))
     if (Serial2.available() )
     {
       ch = Serial2.read();
       //Serial.print ((int) ch);
       //Serial.print ( " " );
       Serial.print (ch);
       if ((lastCh == 'O') && (ch == 'K')) {
          if (wait) 
            startTime = millis(); 
          else
            break;
       }   
       lastCh = ch;
     }
}

void sendCommand ( char * cmd, int timeout, boolean wait ) {
  Serial.print ( "Sending " );
  Serial.println ( cmd );
  Serial2.println ( cmd );
  getResponse(timeout, wait);  
}

void respond(char * response, boolean force) {
  char ch;
  int len = strlen (response);
  if (!silentRunning || force) {
    Serial2.print ( "AT+CIPSEND=0,");
    Serial2.println ( len+2 );
    delay (50);
    Serial2.println (response);
    Serial2.println ();
    delay (50);
    Serial.println (response);   
    delay (100);
    Serial2.println ( "AT+CIPCLOSE=1");
    delay (100);
  }  
}

void loop () {
  char ch;
  int index;
  boolean debugIt = false;
  char cmd [30];
  static int cmdIndex = 0;
  if (initialized) 
  {     
    if (Serial.available() )
    {
       ch = Serial.read();
       if ((ch == 13) || (ch == 10)) {
         if (ch == 13) {           
           sendCommand (cmd, 1000, true);
           cmd[0] = 0;
           cmdIndex = 0;
         }
       } else {
         cmd[cmdIndex] = ch;
         cmdIndex = cmdIndex + 1;
         cmd[cmdIndex] = 0;
       }         
    } 
 
    if (Serial2.available() )
    {
      ch = Serial2.read();
      if (debugIt)
        Serial.print (ch);
      index = commands.matchString (ch, false);
      if (index > -1) {
        //Serial.println ( index );
        
        switch (index) {
          case 0: // Android device is looking for me
            respond ("CTS", true);   
            digitalWrite (12,1);        
            break;
          case 1: // Command 1
            Serial.println ( "Process Command 1" );
            respond ("ACK", false);
            digitalWrite (13,0);
            break;
          case 2: // Command 2
            Serial.println ( "Process Command 2" );
            respond ("ACK", false );
            digitalWrite (13,1);
            break;
          case 3: // Command 3
            Serial.println ( "Process Command 3" );
            respond ("ACK",false);
            digitalWrite (13,0);
            break;
          default:
            break; // Do nothing
        }    
      }
    }   
  } 
}

