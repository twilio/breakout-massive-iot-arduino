#include "massive-sdk/src/platform/random.h"
#include <Arduino.h>

int owl_random(int max) {
  return random(max);
}
