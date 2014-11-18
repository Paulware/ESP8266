#include "PSTRStrings1.h"
#include <EEPROM.h>
PSTRStrings1 commands(5);   // BASIC commands
boolean initialized = false;
boolean silentRunning = true;
boolean menu = false;
int state = 0;
byte leftOffset = 0;
byte rightOffset = 0;

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

int pwms[] = {LEFTFORWARD, LEFTBACKWARD,
              RIGHTFORWARD, RIGHTBACKWARD };

void writePwms ( int leftForward, int leftBackward, int rightForward, int rightBackward) {
  analogWrite (pwms[0], leftForward); // Left forward
  analogWrite (pwms[1], leftBackward); // Left backward
  analogWrite (pwms[2], rightForward); // Right forward
  analogWrite (pwms[3], rightBackward); // Right backward
}

void goForward ( ) {
  writePwms (STRAIGHTSPEED-leftOffset,0,STRAIGHTSPEED-rightOffset,0);
  pwmTimeout = millis() + STEPTIME;
}

void goBack() {
  writePwms (0,STRAIGHTSPEED-leftOffset,0,STRAIGHTSPEED-rightOffset);
  pwmTimeout = millis() + STEPTIME;
}

void goLeft () {
  writePwms (0,TURNSPEED,TURNSPEED,0);
  pwmTimeout = millis() + TURNTIME;
}

void goRight () {
  writePwms (TURNSPEED,0,0,TURNSPEED);
  pwmTimeout = millis() + TURNTIME;
}

/* end pwm class */
  
void setup() {
  // change "key" to a unique string
  commands.addString ( PSTR ( "findkey"));   //  0
  commands.addString ( PSTR ( "cmd1" ));     //  1
  commands.addString ( PSTR ( "cmd2" ));     //  2
  commands.addString ( PSTR ( "cmd3" ));     //  3
  commands.addString ( PSTR ( "!menu!" ));   //  4
  Serial.begin (115200);
  pinMode (13,OUTPUT);
  pinMode (12,OUTPUT);
  digitalWrite (13,1);
  digitalWrite (12,0);
  digitalWrite (13,0);
  
  
  leftOffset = EEPROM.read(0);
  if (leftOffset == 255) { // Uninitialized
     leftOffset = 0;
     EEPROM.write ( 0,leftOffset );
  }
  rightOffset = EEPROM.read (1);
  if (rightOffset == 255) { // Uninitialized 
     rightOffset = 0;
     EEPROM.write ( 0, rightOffset );
  }
  
   
  sendCommand ( "AT" );
  delay (1000);
  digitalWrite (13,1);
  sendCommand ( "AT+RST" );
  delay (8000);
  sendCommand ( "AT+CWMODE=3");
  delay (1000);
  //sendCommand ( "AT+CWLAP" );
  //delay (1000);  digitalWrite (13,0);
  sendCommand ( "AT+CWJAP=\"SSID\",\"Password\"" );
  delay (1000);
  digitalWrite (13,1);
  sendCommand ( "AT+CIPMUX=1" );
  delay (4000);
  digitalWrite (13,0);
  sendCommand ( "AT+CIPSERVER=1,3333" );
  delay (1000);
  digitalWrite (13,1);
  sendCommand ( "AT+CIPSTO=120" );
  delay (1000);
  initialized = true;
  
  goRight();
    
}

void getResponse () {
  unsigned long startTime;
  char ch;
  char lastCh;
  
  startTime = millis();
  lastCh = ' ';
  while (millis() < (startTime + 3000))
     if (Serial.available() )
     {
       ch = Serial.read();
       if ((lastCh == 'O') && (ch == 'K'))
          break;
       lastCh = ch;
     }
     
}

void sendCommand ( char * cmd ) {
  Serial.println ( cmd );
  getResponse();  
}

void respond(char * response, boolean force) {
  char ch;
  int len = strlen (response);
  if (!silentRunning || force) {
    Serial.print ( "AT+CIPSEND=0,");
    Serial.println ( len+2 );
    delay (50);
    Serial.println (response);
    Serial.println ();
    delay (50);
    Serial.println ( "AT+CIPCLOSE=1");
    delay (100);
  }  
}

void loop () {
  char ch;
  int index;
  static boolean lastMenu = false;
  
  if (menu) {
    if (!lastMenu) {
      lastMenu = true;
      Serial.println ("*               MENU              *" );
      Serial.println ("*   Enter R for faster right side *" );
      Serial.println ("*   Enter r for slower right side *" );
      Serial.println ("*   Enter L for faster left side *" );
      Serial.println ("*   Enter l for slower left side *" );
    }
    if (Serial.available() ) {
      ch = Serial.read();
      switch (ch ) {
        case 'r':
        case 'R':
          Serial.print ( "Right offset " );
          if (ch == 'r')
            if (rightOffset == 100) 
               Serial.println ( "Right side can go no slower" );
            else
               rightOffset = rightOffset + 1;
          else
            if (rightOffset == 0)
               Serial.println ( "Right side can go no faster" );
            else
               rightOffset = rightOffset - 1;  
          EEPROM.write(1, rightOffset);          
          Serial.println ( (int)rightOffset );
          break;
        case 'L':
        case 'l':
          Serial.print ( "Left offset " );
          if (ch == 'l')
            if (leftOffset == 100) 
               Serial.println ( "Left side can go no slower" );
            else
               leftOffset = leftOffset + 1;
          else
            if (leftOffset == 0)
               Serial.println ( "Left side can go no faster" );
            else
               leftOffset = leftOffset - 1;  
          EEPROM.write (0,leftOffset );
          Serial.println ( (int)leftOffset );
          break;
        case '!':
          Serial.println ( "Leaving menu mode" );
          menu = false;
          lastMenu = false;
          break;
        case 13:
        case 10:
          break; // Carriage Return / Line Feed 
        default:
          Serial.print ( "Not understood: " );
          Serial.println ( ch );
          break;        
      }
    }
  } else {
    if (pwmTimeout > 0) {
      if (millis() > pwmTimeout) {
         pwmTimeout = 0;
         writePwms (0,0,0,0);       
      }
    }
    if (initialized) 
    {     
      if (Serial.available() )
      {
        ch = Serial.read();
        index = commands.matchString (ch, false);
        if (index > -1) {        
          switch (index) {
            case 0: // Android device is looking for me
              respond ("CTS", true);   
              digitalWrite (12,1);        
              break;
            case 1: // Command 1
              respond ("ACK", false);
              digitalWrite (13,0);
              goLeft();
              break;
            case 2: // Command 2
              respond ("ACK", false );
              digitalWrite (13,1);
              goRight();
              break;
            case 3: // Command 3
              respond ("ACK",false);
              digitalWrite (13,0);
              goForward();
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
  } 
}

