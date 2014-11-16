/*
 * Utils file.
 */
#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef ARDUINO_CLIENT_NODE

// Start


/**
 * Cast float to string with two decimal parts.
 * Malloc null return not handled.
 */
char* float_to_string(float num) {
  // Do not handle negative
  bool wasNegative = false;
  if (num<0) {
    num = num * -1;
    wasNegative = true;
  }
  int full = (int) num;
  int decimal = (num - full) * 100;
  int num_digits = floor(log10(abs(full))) + 1;

  char* output;
  int extra_payload = 4;
  char* format = "%d.%02d";
  if (wasNegative) {
    extra_payload = 5;
    format = "-%d.%02d";
  }
  output = (char*) malloc(sizeof(char)*(num_digits+extra_payload));
  if (output != NULL) {
    sprintf(output, format, full, decimal);
  }
  return output;
}

// End

#else
#error This library is only for use on a client node.
#endif // ARDUINO_CLIENT_NODE

#endif // __COMMUNICATIONS_H__

