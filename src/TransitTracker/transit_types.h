#ifndef TRANSIT_TYPES_H
#define TRANSIT_TYPE_H

#include <Arduino.h>

struct RouteTracker {
  String lineName;
  String arrivals[10]; // Make and arrival struct direction, name, etc
  int arrivalCount = 0;
};

#endif