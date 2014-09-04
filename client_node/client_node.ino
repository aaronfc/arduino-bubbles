/*
 Simple node.
 */

 #define ARDUINO_CLIENT_NODE

 #define MEASURE_INTERVAL_MS 30000

 #include <SPI.h>
 #include "nRF24L01.h"
 #include "RF24.h"
 #include "printf.h"
 #include "communications.h"
 #include "temperature_sensor.h"

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

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  Serial.println("ROLE: CLIENT NODE");

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
  radio.openWritingPipe(pipe_to_central);

  //
  // Dump the configuration of the rf unit for debugging
  //
  radio.printDetails();

  //
  // Set external reference for analog input reading
  //
  analogReference(EXTERNAL);
}

void loop(void)
{
  float temperature = get_temperature();

  // TODO Generate this automatically
  String message = "{\"temperature\": ";
  message.concat((int)temperature);
  message.concat(".");
  message.concat(((int)(temperature * 100) % 100));
  message.concat("}");   
  
  // Print message
  Serial.println(message);
  
  // Deliver message
  bool ok = send_to_central_node(radio, message);
  if (ok)
  printf("ok\n\r");
  else
  printf("failed.\n\r");

  // Delay
  delay(MEASURE_INTERVAL_MS);
}
