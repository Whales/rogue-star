#include "planet.h"
#include "rng.h"
#include "goods.h"
#include "globals.h"
#include "window.h"

#define SUPPLY(type, amount) \
  tmp.supply.push_back( Good_value(type, (amount)) )
#define DEMAND(type, amount) \
  tmp.demand.push_back( Good_value(type, (amount)) )

std::string random_name();

std::string planet_name_at(int x, int y)
{
  Planet* tmp = planet_at(x, y);
  if (tmp) {
    return tmp->name;
  }
  return "Deep space";
}

Planet* planet_at(int x, int y)
{
  for (int i = 0; i < PLANETS.size(); i++) {
    if (PLANETS[i].posx == x && PLANETS[i].posy == y) {
      return &(PLANETS[i]);
    }
  }
  return NULL;
}

Planet::Planet()
{
  name = "NULL";
  gravity = 0;
  for (int i = 0; i < NUM_GOODS; i++) {
    supply[i] = 5;
    demand[i] = 5;
  }
}

int Planet::buy_price(Good_id good)
{
  Good_datum *info = GOOD_DATA[good];
  int low  = info->low_value,
      high = info->high_value + WORLD.price_adjustment[good];
  int range = high - low;
  int current_supply = supply[good], current_demand = demand[good];
  int static_adj = 0, percent_adj = 100;

  for (int i = 0; i < WORLD.current_events.size(); i++) {
    Event* ev = &(WORLD.current_events[i]);
    for (int n = 0; n < ev->modifiers.size(); n++) {
      Price_modifier *mod = &(ev->modifiers[n]);
      if (mod->good == good &&
          (mod->planet_id == -1 || mod->planet_id == uid)) {
        static_adj  += mod->static_change;
        percent_adj += mod->percent_change;
        if (current_supply > 0) {
          current_supply += mod->supply_change;
        }
        if (current_demand > 0) {
          current_demand += mod->demand_change;
        }
      }
    }
  }

  if (current_supply <= 0) {
    return -1;
  }
        
  int ret = low + (range * (9 - current_supply)) / 9;
  if (current_demand > 0) {
    ret += (range * current_demand) / 36;
  }

  ret = (ret * percent_adj) / 100;
  ret += static_adj;

  return ret;
}

int Planet::sell_price(Good_id good)
{
  Good_datum *info = GOOD_DATA[good];
  int low  = info->low_value,
      high = info->high_value + WORLD.price_adjustment[good];
  int range = high - low;
  int current_supply = supply[good], current_demand = demand[good];
  int static_adj = 0, percent_adj = 100;
  for (int i = 0; i < WORLD.current_events.size(); i++) {
    Event* ev = &(WORLD.current_events[i]);
    for (int n = 0; n < ev->modifiers.size(); n++) {
      Price_modifier *mod = &(ev->modifiers[n]);
      if (mod->good == good &&
          (mod->planet_id == -1 || mod->planet_id == uid)) {
        static_adj  += mod->static_change;
        percent_adj += mod->percent_change;
        if (current_supply > 0) {
          current_supply += mod->supply_change;
        }
        if (current_demand > 0) {
          current_demand += mod->demand_change;
        }
      }
    }
  }
  if (current_demand <= 0) {
    return -1;
  }
  int ret = low + (range * current_demand) / 9;
  if (current_supply > 0) {
    ret -= (range * current_supply) / 24;
  }
  if (ret >= buy_price(good) && buy_price(good) > 0) {
    ret = buy_price(good) - 1;
  }
  if (ret < 0) {
    return -1;
  }
  ret = (ret * percent_adj) / 100;
  ret += static_adj;
  return ret;
}

int Planet::fuel_price(Good_id good)
{
  return buy_price(good) / 10;
}

void Planet::randomize()
{
  name = random_name();
  gravity = rng(30, 225);
  long sym_char = (gravity > 100 ? 'O' : 'o');
  nc_color sym_col = c_ltgray;
// First, pick the planet's biome
  switch (rng(1, 9)) {
    case 1:
      biome_name = "grassy";
      supply[GOOD_FERTILIZER]   +=  1;
      supply[GOOD_CROPS]        +=  2;
      supply[GOOD_FOODS]        +=  1;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_FERTILIZER]   +=  1;
      demand[GOOD_SOLIDFUEL]    +=  1;
      demand[GOOD_CROPS]        -=  1;
      demand[GOOD_EDIBLES]      -=  1;
      sym_col = c_ltgreen;
      break;

    case 2:
      biome_name = "desert";
      supply[GOOD_RAWFUEL]      +=  2;
      supply[GOOD_URANIUM]      +=  1;
      supply[GOOD_ORE]          -=  1;
      supply[GOOD_WATER]        -=  4;
      supply[GOOD_FERTILIZER]   -=  2;
      supply[GOOD_CROPS]        -=  3;
      demand[GOOD_WATER]        +=  4;
      demand[GOOD_FOODS]        +=  1;
      demand[GOOD_EDIBLES]      +=  2;
      sym_col = c_yellow;
      break;

    case 3:
      biome_name = "water";
      supply[GOOD_WATER]        +=  3;
      supply[GOOD_FERTILIZER]   +=  2; // Seaweed and such
      supply[GOOD_CROPS]        +=  1; // Ditto
      supply[GOOD_RAWFUEL]      +=  1;
      supply[GOOD_HYDROGEN]     +=  1;
      supply[GOOD_ORE]          -=  4;
      demand[GOOD_ELECTRONICS]  +=  1;
      demand[GOOD_MACHINERY]    +=  2;
      demand[GOOD_HYDROGEN]     +=  1;
      demand[GOOD_URANIUM]      +=  1;
      demand[GOOD_WATER]        -= 10;
      demand[GOOD_FERTILIZER]   -=  1;
      sym_col = c_blue;
      break;

    case 4:
      biome_name = "swamp";
      supply[GOOD_FERTILIZER]   +=  4;
      supply[GOOD_RAWFUEL]      +=  2;
      supply[GOOD_ORE]          -=  2;
      supply[GOOD_CROPS]        -=  1;
      demand[GOOD_METALS]       +=  1;
      demand[GOOD_ELECTRONICS]  +=  1;
      demand[GOOD_MACHINERY]    +=  2;
      demand[GOOD_FOODS]        +=  1;
      demand[GOOD_EDIBLES]      +=  2;
      sym_col = c_brown;
      break;

    case 5:
      biome_name = "ice";
      supply[GOOD_ELECTRONICS]  +=  1;
      supply[GOOD_WATER]        +=  1;
      supply[GOOD_FERTILIZER]   -=  3;
      supply[GOOD_CROPS]        -=  3;
      demand[GOOD_CONSUMER]     +=  1;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_FOODS]        +=  3;
      demand[GOOD_EDIBLES]      +=  2;
      demand[GOOD_SOLIDFUEL]    +=  2;
      demand[GOOD_HYDROGEN]     +=  1;
      demand[GOOD_URANIUM]      +=  1;
      sym_col = c_ltcyan;
      break;

    case 6:
      biome_name = "barren";
      supply[GOOD_ORE]          +=  3;
      supply[GOOD_WATER]        -=  3;
      supply[GOOD_FERTILIZER]   -=  1;
      supply[GOOD_CROPS]        -=  3;
      supply[GOOD_RAWFUEL]      -=  1;
      demand[GOOD_METALS]       +=  1;
      demand[GOOD_ELECTRONICS]  +=  1;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_FERTILIZER]   -=  1;
      demand[GOOD_HYDROGEN]     +=  2;
      demand[GOOD_URANIUM]      +=  2;
      break;

    case 7:
    case 8:
    case 9:
      biome_name = "varied";
      supply[GOOD_ORE]          +=  1;
      supply[GOOD_CROPS]        +=  1;
      supply[GOOD_RAWFUEL]      +=  1;
      sym_col = c_ltred;
      break;
  }

// Pick population type

  switch(rng(1, 9)) {
    case 1:
      pop_name = "sparse";
      supply[GOOD_CROPS]        +=  1;
      supply[GOOD_RAWFUEL]      +=  1;
      supply[GOOD_METALS]       -=  1;
      supply[GOOD_ELECTRONICS]  -=  2;
      supply[GOOD_CONSUMER]     -=  2;
      supply[GOOD_FOODS]        -=  1;
      supply[GOOD_EDIBLES]      -=  2;
      supply[GOOD_SOLIDFUEL]    -=  2;
      demand[GOOD_ORE]          -=  2;
      demand[GOOD_METALS]       +=  2;
      demand[GOOD_CONSUMER]     -=  4;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_FERTILIZER]   +=  1;
      demand[GOOD_FOODS]        -=  4;
      demand[GOOD_EDIBLES]      -=  4;
      break;

    case 2:
      pop_name = "rural";
      supply[GOOD_CROPS]        +=  2;
      supply[GOOD_FOODS]        +=  1;
      supply[GOOD_ELECTRONICS]  -=  1;
      supply[GOOD_CONSUMER]     -=  1;
      supply[GOOD_SOLIDFUEL]    -=  1;
      break;

    case 3:
    case 4:
      pop_name = "settled";
      supply[GOOD_METALS]       +=  1;
      supply[GOOD_FOODS]        +=  1;
      supply[GOOD_EDIBLES]      +=  1;
      demand[GOOD_CONSUMER]     +=  1;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_FOODS]        +=  1;
      demand[GOOD_EDIBLES]      +=  1;
      demand[GOOD_SOLIDFUEL]    +=  1;
      break;

    case 5:
    case 6:
    case 7:
      pop_name = "urban";
      supply[GOOD_ORE]          -=  1;
      supply[GOOD_WATER]        -=  1;
      supply[GOOD_FERTILIZER]   +=  1;
      supply[GOOD_CROPS]        -=  2;
      demand[GOOD_ORE]          -=  1;
      demand[GOOD_CONSUMER]     +=  2;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_WATER]        +=  2;
      demand[GOOD_FOODS]        +=  1;
      demand[GOOD_EDIBLES]      +=  2;
      demand[GOOD_SOLIDFUEL]    +=  2;
      break;

    case 8:
      pop_name = "megacity";
      supply[GOOD_ORE]          -=  2;
      supply[GOOD_WATER]        -=  3;
      supply[GOOD_FERTILIZER]   +=  3;
      supply[GOOD_CROPS]        -=  4;
      supply[GOOD_FOODS]        -=  1;
      supply[GOOD_RAWFUEL]      -=  1;
      demand[GOOD_ELECTRONICS]  +=  2;
      demand[GOOD_CONSUMER]     +=  4;
      demand[GOOD_MACHINERY]    +=  2;
      demand[GOOD_WATER]        +=  3;
      demand[GOOD_FERTILIZER]   -=  2;
      demand[GOOD_CROPS]        -=  1;
      demand[GOOD_EDIBLES]      +=  3;
      demand[GOOD_SOLIDFUEL]    +=  2;
      demand[GOOD_HYDROGEN]     +=  2;
      demand[GOOD_URANIUM]      +=  2;
      break;

    case 9:
      pop_name = "robotic";
      supply[GOOD_ORE]          +=  1;
      supply[GOOD_METALS]       +=  1;
      supply[GOOD_ELECTRONICS]  +=  1;
      supply[GOOD_MACHINERY]    +=  1;
      supply[GOOD_FERTILIZER]   -=  1;
      supply[GOOD_EDIBLES]      -=  1;
      supply[GOOD_RAWFUEL]      +=  1;
      supply[GOOD_URANIUM]      +=  1;
      demand[GOOD_ELECTRONICS]  +=  2;
      demand[GOOD_CONSUMER]     -= 10;
      demand[GOOD_MACHINERY]    -=  2;
      demand[GOOD_FOODS]        -=  5;
      demand[GOOD_EDIBLES]      -= 10;
      demand[GOOD_SOLIDFUEL]    +=  1;
      demand[GOOD_HYDROGEN]     +=  3;
      demand[GOOD_URANIUM]      +=  4;
      break;
  }

// Decide on the main industry
  switch (rng(1, 9)) {
    case 1:
      industry_name = "living space";
      supply[GOOD_ORE]          -=  2;
      supply[GOOD_METALS]       -=  2;
      supply[GOOD_ELECTRONICS]  -=  1;
      supply[GOOD_CONSUMER]     -=  1;
      supply[GOOD_MACHINERY]    -=  2;
      supply[GOOD_FERTILIZER]   +=  2;
      supply[GOOD_EDIBLES]      +=  1;
      supply[GOOD_RAWFUEL]      -=  3;
      supply[GOOD_SOLIDFUEL]    -=  2;
      supply[GOOD_URANIUM]      -=  1;
      demand[GOOD_ORE]          -=  3;
      demand[GOOD_METALS]       +=  1; // For construction
      demand[GOOD_CONSUMER]     +=  4;
      demand[GOOD_WATER]        +=  3;
      demand[GOOD_CROPS]        +=  1;
      demand[GOOD_FOODS]        +=  3;
      demand[GOOD_EDIBLES]      +=  3;
      demand[GOOD_RAWFUEL]      -=  2;
      break;

    case 2:
      industry_name = "mining";
      supply[GOOD_ORE]          +=  4;
      supply[GOOD_METALS]       +=  1;
      supply[GOOD_WATER]        -=  2;
      supply[GOOD_CROPS]        -=  4;
      supply[GOOD_RAWFUEL]      +=  5;
      supply[GOOD_HYDROGEN]     +=  1;
      supply[GOOD_URANIUM]      +=  3;
      demand[GOOD_METALS]       +=  2;
      demand[GOOD_ELECTRONICS]  +=  2;
      demand[GOOD_CONSUMER]     -=  2;
      demand[GOOD_MACHINERY]    +=  4;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_FERTILIZER]   -=  3;
      demand[GOOD_CROPS]        -=  3;
      demand[GOOD_FOODS]        -=  2;
      demand[GOOD_EDIBLES]      +=  2;
      demand[GOOD_SOLIDFUEL]    +=  2;
      demand[GOOD_HYDROGEN]     +=  1;
      break;

    case 3:
      industry_name = "heavy manu";
      supply[GOOD_METALS]       +=  4;
      supply[GOOD_ELECTRONICS]  +=  1;
      supply[GOOD_CONSUMER]     +=  1;
      supply[GOOD_MACHINERY]    +=  3;
      supply[GOOD_CROPS]        -=  3;
      demand[GOOD_ORE]          +=  3;
      demand[GOOD_METALS]       +=  2;
      demand[GOOD_ELECTRONICS]  +=  1;
      demand[GOOD_CONSUMER]     -=  2;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_FERTILIZER]   -=  4;
      demand[GOOD_CROPS]        -=  2;
      demand[GOOD_FOODS]        -=  1;
      demand[GOOD_EDIBLES]      +=  1;
      demand[GOOD_RAWFUEL]      +=  1;
      demand[GOOD_SOLIDFUEL]    +=  3;
      demand[GOOD_HYDROGEN]     +=  1;
      break;

    case 4:
      industry_name = "refining";
      supply[GOOD_METALS]       +=  2;
      supply[GOOD_ELECTRONICS]  -=  1;
      supply[GOOD_CONSUMER]     -=  2;
      supply[GOOD_MACHINERY]    -=  1;
      supply[GOOD_WATER]        -=  3;
      supply[GOOD_FERTILIZER]   +=  1;
      supply[GOOD_CROPS]        -=  3;
      supply[GOOD_SOLIDFUEL]    +=  5;
      supply[GOOD_HYDROGEN]     +=  2;
      supply[GOOD_URANIUM]      +=  1;
      demand[GOOD_ORE]          +=  1;
      demand[GOOD_ELECTRONICS]  +=  1;
      demand[GOOD_MACHINERY]    +=  3;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_FERTILIZER]   -=  4;
      demand[GOOD_CROPS]        -=  3;
      demand[GOOD_FOODS]        -=  1;
      demand[GOOD_EDIBLES]      +=  2;
      demand[GOOD_RAWFUEL]      +=  5;
      demand[GOOD_SOLIDFUEL]    +=  1;
      break;

    case 5:
      industry_name = "light manu";
      supply[GOOD_ELECTRONICS]  +=  3;
      supply[GOOD_CONSUMER]     +=  3;
      supply[GOOD_MACHINERY]    +=  1;
      supply[GOOD_CROPS]        -=  1;
      supply[GOOD_FOODS]        +=  1;
      supply[GOOD_EDIBLES]      +=  2;
      demand[GOOD_METALS]       +=  2;
      demand[GOOD_ELECTRONICS]  +=  3;
      demand[GOOD_MACHINERY]    +=  2;
      demand[GOOD_FERTILIZER]   -=  1;
      demand[GOOD_CROPS]        +=  2;
      demand[GOOD_FOODS]        +=  2;
      demand[GOOD_EDIBLES]      +=  1;
      demand[GOOD_SOLIDFUEL]    +=  1;
      break;

    case 6:
      industry_name = "farming";
      supply[GOOD_METALS]       -=  1;
      supply[GOOD_WATER]        -=  1;
      supply[GOOD_FERTILIZER]   +=  2;
      supply[GOOD_CROPS]        +=  6;
      supply[GOOD_FOODS]        +=  2;
      supply[GOOD_EDIBLES]      +=  1;
      supply[GOOD_RAWFUEL]      -=  3;
      supply[GOOD_SOLIDFUEL]    -=  2;
      supply[GOOD_URANIUM]      -=  2;
      demand[GOOD_MACHINERY]    +=  2;
      demand[GOOD_WATER]        +=  2;
      demand[GOOD_FERTILIZER]   +=  5;
      demand[GOOD_CROPS]        -=  1;
      demand[GOOD_SOLIDFUEL]    +=  3;
      break;

    case 7:
      industry_name = "food proc";
      supply[GOOD_FERTILIZER]   +=  2;
      supply[GOOD_FOODS]        +=  4;
      supply[GOOD_EDIBLES]      +=  4;
      demand[GOOD_ELECTRONICS]  +=  2;
      demand[GOOD_CONSUMER]     +=  1;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_WATER]        +=  2;
      demand[GOOD_CROPS]        +=  4;
      demand[GOOD_FOODS]        +=  2;
      demand[GOOD_HYDROGEN]     +=  1;
      break;

    case 8:
      industry_name = "business";
      supply[GOOD_ORE]          -=  1;
      supply[GOOD_METALS]       -=  1;
      supply[GOOD_CONSUMER]     +=  1;
      supply[GOOD_CROPS]        -=  1;
      supply[GOOD_RAWFUEL]      -=  1;
      demand[GOOD_ELECTRONICS]  +=  1;
      demand[GOOD_CONSUMER]     +=  4;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_FERTILIZER]   -=  2;
      demand[GOOD_CROPS]        +=  1;
      demand[GOOD_FOODS]        +=  1;
      demand[GOOD_EDIBLES]      +=  2;
      demand[GOOD_HYDROGEN]     +=  1;
      break;

    case 9:
      industry_name = "research";
      supply[GOOD_ELECTRONICS]  +=  2;
      supply[GOOD_CONSUMER]     -=  1;
      supply[GOOD_FERTILIZER]   +=  1;
      supply[GOOD_CROPS]        +=  1;
      supply[GOOD_FOODS]        +=  1;
      supply[GOOD_RAWFUEL]      +=  1;
      supply[GOOD_HYDROGEN]     +=  3;
      demand[GOOD_ORE]          +=  1;
      demand[GOOD_ELECTRONICS]  +=  2;
      demand[GOOD_MACHINERY]    +=  1;
      demand[GOOD_WATER]        +=  1;
      demand[GOOD_CROPS]        +=  2;
      demand[GOOD_RAWFUEL]      +=  2;
      demand[GOOD_URANIUM]      +=  4;
      break;
  }

  for (int i = GOOD_NULL + 1; i < NUM_GOODS; i++) {
    supply[i] += rng(-2, 2);
    demand[i] += rng(-2, 2);
    if (supply[i] < 0) {
      supply[i] = 0;
    }
    if (supply[i] > 9) {
      supply[i] = 9;
    }
    if (demand[i] < 0) {
      demand[i] = 0;
    }
    if (demand[i] > 9) {
      demand[i] = 9;
    }
  }

  sym = glyph(sym_char, sym_col, c_black);
}


std::string random_name()
{
  std::string ret;
  switch (rng(1, 25)) {
    case  1:  ret = "Ar";   break;
    case  2:  ret = "Bes";  break;
    case  3:  ret = "Cor";  break;
    case  4:  ret = "Dan";  break;
    case  5:  ret = "Da";   break;
    case  6:  ret = "Har";  break;
    case  7:  ret = "Lar";  break;
    case  8:  ret = "Mun";  break;
    case  9:  ret = "Nin";  break;
    case 10:  ret = "Pes";  break;
    case 11:  ret = "Quin"; break;
    case 12:  ret = "Rin";  break;
    case 13:  ret = "Se";   break;
    case 14:  ret = "Ta";   break;
    case 15:  ret = "Van";  break;
    case 16:  ret = "Jin";  break;
    case 17:  ret = "Sol";  break;
    case 18:  ret = "Dor";  break;
    case 19:  ret = "Sin";  break;
    case 20:  ret = "Al";   break;
    case 21:  ret = "Ro";   break;
    case 22:  ret = "Ska";  break;
    case 23:  ret = "Ti";   break;
    case 24:  ret = "Gor";  break;
    case 25:  ret = "War";  break;
  }
  switch (rng(1, 20)) {
    case  1:  ret += "go";  break;
    case  2:  ret += "pa";  break;
    case  3:  ret += "ki";  break;
    case  4:  ret += "too"; break;
    case  5:  ret += "us";  break;
    case  6:  ret += "a";   break;
    case  7:  ret += "pi";  break;
    case  8:  ret += "lo";  break;
    case  9:  ret += "mi";  break;
    case 10:  ret += "u";   break;
    case 11:  ret += "ba";  break;
    case 12:  ret += "an";  break;
    case 13:  ret += "be";  break;
    case 14:  ret += "va";  break;
    case 15:  ret += "go";  break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:  break;
  }
  switch (rng(1, 25)) {
    case  1:  ret += "nee";   break;
    case  2:  ret += "bah";   break;
    case  3:  ret += "cant";  break;
    case  4:  ret += "ban";   break;
    case  5:  ret += "gah";   break;
    case  6:  ret += "no";    break;
    case  7:  ret += "nine";  break;
    case  8:  ret += "par";   break;
    case  9:  ret += "lin";   break;
    case 10:  ret += "sin";   break;
    case 11:  ret += "la";    break;
    case 12:  ret += "ca";    break;
    case 13:  ret += "'ja";   break;
    case 14:  ret += "kis";   break;
    case 15:  ret += "t";     break;
    case 16:  ret += "ten";   break;
    case 17:  ret += "thro";  break;
    case 18:  ret += "gol";   break;
    case 19:  ret += "terre"; break;
    case 20:  ret += "kos";   break;
    case 21:  ret += "dros";  break;
    case 22:  ret += "tar";   break;
    case 23:  ret += "men";   break;
    case 24:  ret += "chre";  break;
    case 25:  ret += "ta";    break;
  }

  return ret;
}
