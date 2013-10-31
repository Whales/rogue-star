#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <vector>
#include "goods.h"
#include "planet.h"
#include "ship.h"
#include "world.h"
#include "event.h"

struct Event;
struct World;

extern std::vector<Good_datum*>     GOOD_DATA;
extern std::vector<Planet>          PLANETS;
extern std::vector<Ship_part_type*> PARTS;
extern std::vector<Ship*>           NPC_SHIPS;
extern Ship                         PLR;
extern World                        WORLD;
extern std::vector<Event*>          EVENTS;

extern std::vector<std::string>     COMPANIES_ELECTRONIC;
extern std::vector<std::string>     COMPANIES_FOOD;
extern std::vector<std::string>     ACRONYMS;
extern std::vector<std::string>     PRODUCTS_CONSUMER;
extern std::vector<std::string>     PRODUCTS_EDIBLES;

#endif
