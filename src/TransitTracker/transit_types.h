#ifndef TRANSIT_TYPES_H
#define TRANSIT_TYPE_H

#include <Arduino.h>

struct Arrivals {
  String dir;
  int countdown;
};

struct RouteTracker {
  String lineName;
  String arrivals[10]; // Make an arrival struct direction, name, etc
  Arrivals AA[10]; 
  int arrivalCount = 0;
};

#endif