#ifndef _PLANET_H_
#define _PLANET_H_

#include <string>
#include <vector>
#include "goods.h"
#include "glyph.h"

struct Planet_property
{
  std::string name;
  std::string description;
  std::vector<Good_value> supply;
  std::vector<Good_value> demand;

  void reset() { name = ""; description = ""; supply.clear(); demand.clear(); }
};

struct Planet
{
  Planet();
  void randomize();

  int buy_price(Good_id good);
  int sell_price(Good_id good);

  int fuel_price(Good_id good); // Price is divided by 10

  std::string name;
  std::string biome_name, pop_name, industry_name;
  int uid;
  int gravity;
  int posx, posy;
  glyph sym;

  std::vector<Planet_property> properties;

  int supply[NUM_GOODS]; // High supply drives the price down
  int demand[NUM_GOODS]; // High demand drives the price up
/*
  Supply  Demand  Buy Price Sell Price
  Low     Low     High      Low
  Low     High    High      High
  High    Low     Low       Low
  High    High    Medium    Medium
*/
};

std::string planet_name_at(int x, int y);
Planet* planet_at(int x, int y);

#endif
