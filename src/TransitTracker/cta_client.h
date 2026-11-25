#ifndef CTA_CLIENT_H
#define CTA_CLIENT_H

#include "transit_types.h"

bool getArrivalTimes(RouteGroup Routes[], const char* busAPIKey, const char* stopIds);

#endif