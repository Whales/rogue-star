#ifndef _EVENT_H_
#define _EVENT_H_

#include <string>
#include "planet.h"
#include "goods.h"
#include "globals.h"

struct Price_modifier
{
  Good_id good;
  int planet_id; // -1 means universal
  int static_change;
  int percent_change;
  int supply_change;
  int demand_change;
  int timeout; // The day on which this modifier runs out
};

struct Event
{
  //Event();
  Event(std::string N, std::string D, int F, nc_color C, ...);
  ~Event();

  int frequency;
  std::string name;
  std::string description;
  nc_color base_color;
  std::vector<Price_modifier> modifiers;
};

#endif
