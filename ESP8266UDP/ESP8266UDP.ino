#include "PSTRStrings1.h"
PSTRStrings1 commands(5);   // BASIC commands
PSTRStrings1 response(4);   // Responses
  
boolean silentRunning = true;
int state = 0;

void setup() {
  // change "key" to a unique string
  commands.addString ( PSTR ( "findkey"));   //  0
  commands.addString ( PSTR ( "cmd1" ));     //  1
  commands.addString ( PSTR ( "cmd2" ));     //  2
  commands.addString ( PSTR ( "cmd3" ));     //  3
  
  response.addString ( PSTR ( "OK" ));
  response.addString ( PSTR ( "ready" ));
  response.addString ( PSTR ( "no change" ));
  Serial.begin (115200);
  Serial.println ( "Ready2" );
  Serial2.begin (115200);
  pinMode (13,OUTPUT);
  pinMode (12,OUTPUT);
  Serial.println ( "\nEnter \"!\" to setup ESP8266 " );
}

void launch () {
  digitalWrite (13,0);
  digitalWrite (12,0);
  sendCommand ( "AT", true );
  digitalWrite (13,0);
  sendCommand ( "AT+RST", false );
  digitalWrite (13,1);
  sendCommand ( "AT+CWMODE=3",true );
  digitalWrite (13,1);
  sendCommand ( "AT+CWLAP", true );
  sendCommand ( "AT+CWJAP=\"SSID\",\"Password\"", true);
  digitalWrite (13,0);
  sendCommand ( "AT+CIPMUX=1", true );
  digitalWrite (13,1);
  sendCommand ( "AT+CIPSERVER=1,3333", true );
  digitalWrite (13,0);
  sendCommand ( "AT+CIPSTO=120", true );
  digitalWrite (13,1);
  //sendCommand ("AT+CIOBAUD=115200", true);
  Serial.println ( "\nDone" );
}

void getResponse (boolean waitOK) {
  unsigned long startTime;
  char ch;
  int index;
  
  startTime = millis();
  // Wait 10 seconds maximum
  while (millis() < (startTime + 10000))
     if (Serial2.available() )
     {
       ch = Serial2.read();
       Serial.print (ch);
       index = response.matchString (ch, false);
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
   Serial.println ();  
}

void sendCommand ( char * cmd,  boolean wait ) {
  Serial.print ( "Sending " );
  Serial.println ( cmd );
  Serial2.println ( cmd );
  getResponse(wait);  
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
  boolean debugIt = true;
  char cmd [30];
  static int cmdIndex = 0;
  
  if (Serial.available() )  {
     ch = Serial.read();
     if (ch == '!') 
       launch();       
     else if ((ch == 13) || (ch == 10)) {
       if (ch == 13) {           
         if (cmdIndex > 0) 
            sendCommand (cmd, true);
         cmd[0] = 0;
         cmdIndex = 0;
       }
     } else {
       cmd[cmdIndex] = ch;
       cmdIndex = cmdIndex + 1;
       cmd[cmdIndex] = 0;
     }         
  } 
 
  if (Serial2.available() ) {
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

