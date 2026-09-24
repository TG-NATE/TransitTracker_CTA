#ifndef TRANSIT_TYPES_H
#define TRANSIT_TYPE_H

#include <Arduino.h>

struct RouteTracker {
  const char* lineName;
  String arrivals[5];
};

#endif