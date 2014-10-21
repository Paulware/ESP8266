
boolean initialized = false;

void setup() {
  Serial.begin (115200);
  Serial.println ( "Ready2" );
  Serial2.begin (115200);
  sendCommand ( "AT" );
  sendCommand ( "AT+RST" );
  delay (4000);
  sendCommand ( "AT+CWLAP" );
  sendCommand ( "AT+CWJAP=\"SSID\",\"Password\"" );
  sendCommand ( "AT+CIPMUX=0" );
  sendCommand ( "AT+CIFSR" );
  sendCommand ( "AT+CIPMUX=1" );
  sendCommand ( "AT+CIPSERVER=1,80" );
  initialized = true;
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

void httpOutput (char * msg) {
  int len = strlen (msg);
  Serial2.print ( "AT+CIPSEND=0,");
  Serial2.println ( len+2 );
  delay (50);
  Serial2.println ( msg );
  Serial2.println ();
  delay (50);
  Serial.println ( msg );
}

void webserver() {
  char ch;
  httpOutput ( "<html><TITLE>ESP8266 Server</TITLE>" );
  httpOutput ( "<body><H3>Welcome to the ESP8266 Server</H3><hr>");
  httpOutput ( "Here is a list: <ul><li>item 1</li><li>Item 2</li></ul></body></html>" );
  delay (100);
  Serial2.println ( "AT+CIPCLOSE=0");
  while (Serial2.available() ) // Flush the input buffer
  {
    ch = Serial2.read();
    Serial.print (ch);
  }
}

void loop () {
  char ch;
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
      Serial.print (ch);
      if (ch == 71) 
        webserver();
    }   
  } 
}
