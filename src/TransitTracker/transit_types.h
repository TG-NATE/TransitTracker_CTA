#ifndef TRANSIT_TYPES_H
#define TRANSIT_TYPES_H

#include <Arduino.h>

constexpr int MAX_ARRIVALS = 3;

struct Arrivals {
  String dir;
  int countdown;
};

struct RouteTracker {
  String lineName;
  String arrivals[MAX_ARRIVALS]; // Make an arrival struct direction, name, etc
  Arrivals AA[MAX_ARRIVALS]; 
  int arrivalCount = 0;
};

#endif