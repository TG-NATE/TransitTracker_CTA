#ifndef TRANSIT_TYPES_H
#define TRANSIT_TYPES_H

#include <Arduino.h>

constexpr int MAX_ARRIVALS = 2;
constexpr int MAX_ROUTES = 12;

struct Arrival {
  bool isDue = false; // If isDue is true, countdown is 0.
  int countdown = 0;
};

struct RouteGroup {
  String route;
  String direction;
  String destination;
  Arrival arrivals[MAX_ARRIVALS];
  int arrivalCount = 0;
};

#endif