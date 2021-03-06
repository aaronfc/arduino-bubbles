#ifndef __COMMUNICATIONS_H__
#define __COMMUNICATIONS_H__

#ifdef ARDUINO_CLIENT_NODE


// Start

#include "RF24.h"

#define MAX_RETRIES 1

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(8,9);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipe_to_central = 0xE8E8F0F0E1LL;


/**
 * Deliver a character, with up to MAX_RETRIES retries.
 *
 * @param RF24 radio
 * @param const char character
 * @return bool
 */
bool reliable_send_char(RF24 radio, const char c) {
  for (int i=0; i<=MAX_RETRIES; i++) {
    if (radio.write(&c, 1)) return true;
  }
  return false;
}

/**
 * Send to central node.
 *
 * Protocol:
 * Start message: '^'
 * Successful message, end of message: '$'
 * Discard last message: '#'
 *
 * @param RF24 radio
 * @param String message
 * @return bool Whether the message was successfuly delivered or not.
 */
bool send_to_central_node(RF24 radio, String message) {
  bool sentOk;
  int messageSize = message.length();
  char c;

  // send the start-message char
  c = '^';
  sentOk = reliable_send_char(radio, c);

  if (sentOk) {
    Serial.println("Start message!");
    // Send message char by char
    for (int i = 0; i < messageSize; i++) {
      c = message.charAt(i);
      Serial.print(c);
      sentOk = reliable_send_char(radio, c);

      // If we fail sending a char, break the process
      if (!sentOk) {
        break;
      }
    }
  
    // Send success-message or faulty-message char
    if (sentOk) {
      // send the 'end of message' value...  
      c = '$';
      sentOk = reliable_send_char(radio, c);
    } else {
      // send the 'faulty message' value (optional)
      c = '#';
      for (int i = 0; i < 5; i++) {
        if(reliable_send_char(radio, c)) {
          break;
        }
      }
    }
  }
  if (sentOk) {
    Serial.print(" Success!");
  } else {
    Serial.println(" Error!");
  }
  radio.powerDown();
  delay(10);
  radio.powerUp();
  delay(500);
  return sentOk; 
}

// End

#else
#error This library is only for use on a client node.
#endif // ARDUINO_CLIENT_NODE

#endif // __COMMUNICATIONS_H__

