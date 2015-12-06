/*
 Central node Arduino node.
 */

#include <SPI.h>
#include <Ethernet.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
//#include <MemoryFree.h>

// RF -----------------------------------------------

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 8 & 9
RF24 radio(8,9);


// Ethernet ----------------------------------------

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipe_to_central = 0xE8E8F0F0E1LL;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char SERVER[] = "www.eses.es";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,0,177);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  Serial.println(F("ROLE: CENTRAL NODE\n\r"));

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //
  radio.openReadingPipe(1,pipe_to_central);

  //
  // Dump the configuration of the rf unit for debugging
  //
  radio.printDetails();
  
  
  Serial.println(F("Loading Ethernet!"));
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);  
   
  Serial.println(F("Ready ethernet with IP:"));
  Serial.println(Ethernet.localIP());

  //Serial.println(F("Initial memory:"));
  //Serial.println(getFreeMemory());

  // Start listening
  radio.startListening();
  Serial.println(F("Listening..."));
}

char inChar;
char message[255] = "";
int index = 0;
bool receiving = false;
void loop(void){
  if (radio.available()) {
      Serial.println(F("Something available!"));
      inChar = 0;
      radio.read(&inChar, 1);
      Serial.println(F("Read: "));
      Serial.println(inChar);

      if (inChar == 0) { // ignore couldn't read from radio
        Serial.println(F("Ignoring!"));
        return;
      }

      if (inChar == '^') {
        receiving = true;
        message[0] = '\0';
        index = 0;
        return;
      }
      if (inChar == '#') {
        receiving = false;
        message[0] = '\0';
        index = 0;
        Serial.println(F("Faulty message warning received! Resetting radio."));
        resetRadio();
        return;
      }
      if (receiving) {
        if (inChar == '$') {
          // End of message found!
          Serial.println(F("MESSAGE:"));
          Serial.println(message);

          // Shut down radio, we are going to do some heavy stuff. Wait to send the ack.
          delay(100);
          radio.powerDown();

          // Deliver message
          upload(message);
  
          // Reset the local message
          message[0] = '\0';
          index = 0;

          resetRadio();
          return;
        }
  
        // This is executed only if it is a char distinct than ^, $ or #
        message[index] = inChar;
        message[index+1] = '\0';
        index++;
      }
  }
}

void upload(const String theMessage) {
  if (client.connect(SERVER, 80)) {
    Serial.println(F("connected to remote"));

    // Make a HTTP request:
    String query = "GET /domot.php?data=";
    String encoded = URLEncode(theMessage);
    query.concat(encoded);
    query.concat(" HTTP/1.1");
    Serial.println(encoded);
    Serial.println(query);
    
    // Query
    client.println(query);
    client.println("Host: www.eses.es");
    client.println("Connection: close");
    client.println();
    
    // Stop connection
    client.stop();
  } else {
    Serial.println(F("connection failed"));
  }
}

void resetRadio()
{
  delay(25); // Give some time to finish whatever it was doing
  radio.powerDown();
  delay(25); // Give it some time to power down properly
  radio.powerUp();
  delay(1000); // Give it a lot of time to power up
  radio.startListening();
  delay(25); // Give it some time to allow radio to start listening
}

/**
 * URL Encode a string.
 * 
 * Based on http://www.icosaedro.it/apache/urlencode.c
 *
 */
String URLEncode(String message)
{
    int strSize = message.length() + 1;
    char charArray[strSize];
    message.toCharArray(charArray, strSize);

    const char *msg = charArray;
    const char *hex = "0123456789abcdef";
    String encodedMsg = "";

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
                || ('A' <= *msg && *msg <= 'Z')
                || ('0' <= *msg && *msg <= '9') ) {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }
    return encodedMsg;
}

