/*
 Simple node.
 */

#define ARDUINO_CLIENT_NODE "AQUARIUM"
#define PRINT_SERIAL true
#define USE_RF true
#define DEBUG true

#define MEASURE_INTERVAL_MS 5000

#include <SPI.h>
#include <stdio.h>
#include <math.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "communications.h"
#include "OneWire.h"
#include "temperature_sensor.h"
#include "utils.h"

void setup(void)
{
  if (PRINT_SERIAL) {
    setup_serial();
  }

  if (USE_RF) {
    setup_radio();
  }

  // Wait a little to get the system ready and then print info.
  delay(3000);
  get_temperature();
  print_node_info();
}

void setup_radio() {
  radio.begin();
  // increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // reduce the payload size (improves reliability)
  radio.setPayloadSize(8);

  // Open pipes to other nodes for communication
  radio.openWritingPipe(pipe_to_central);

  // Dump the configuration of the rf unit for debugging
  if (PRINT_SERIAL) {
    radio.printDetails();
  }
}

void setup_serial() {
  // SETUP SERIAL
  Serial.begin(57600);
  printf_begin();
}

/**
 * Print information about this node.
 */
void print_node_info() {
  Serial.println("======================");
  Serial.print("NODE: "); Serial.println(ARDUINO_CLIENT_NODE);
  Serial.print("PRINT_SERIAL: "); Serial.println(PRINT_SERIAL);
  Serial.print("USE_RF: "); Serial.println(USE_RF);
  Serial.print("DEBUG: "); Serial.println(DEBUG);
  Serial.println("======================");
}

void loop(void)
{
  float temperature = get_temperature();
  char* temperature_str = float_to_string(temperature);

  // TODO Generate this automatically
  String message = "{\"node\": \"";
  message.concat(ARDUINO_CLIENT_NODE);
  message.concat("\", \"temperature\": ");
  message.concat(temperature_str);
  message.concat("}");
  free(temperature_str);
  
  // Print message
  Serial.println(message);
  
  if (USE_RF) {
    // Deliver message
    bool ok = send_to_central_node(radio, message);
    if (ok)
    printf("ok\n\r");
    else
    printf("failed.\n\r");
  }

  // Delay
  delay(MEASURE_INTERVAL_MS);
}
