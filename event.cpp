#include "event.h"
#include "window.h"

/*
Event::Event()
{
  name = "nothing";
  frequency = 9999999;
  description = "Nothing happens.";
}
*/

Event::Event(std::string N, std::string D, int F, nc_color C, ...)
{
  name        = N;
  frequency   = F;
  description = D;
  base_color  = C;

  va_list args;
  va_start(args, C);
  int buff;
  bool done = false;
  do {
    buff = va_arg(args, int);
    if (buff != 0) {
      Price_modifier mod;
      mod.good = Good_id(buff);
      buff = va_arg(args, int);
      mod.planet_id = buff;
      buff = va_arg(args, int);
      mod.static_change = buff;
      buff = va_arg(args, int);
      mod.percent_change = buff;
      buff = va_arg(args, int);
      mod.supply_change = buff;
      buff = va_arg(args, int);
      mod.demand_change = buff;
      buff = va_arg(args, int);
      mod.timeout = buff;
      modifiers.push_back(mod);
    } else {
      done = true;
    }
  } while (!done);
}

Event::~Event()
{
}
