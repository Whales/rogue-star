#ifndef _GOODS_H_
#define _GOODS_H_

#include <string>

struct Good_datum
{
  Good_datum(std::string N = "", int LV = 0, int HV = 0, int V = 0, int M = 0) :
    name (N), low_value(LV), high_value(HV), volume(V), mass(M)
  { };
  ~Good_datum() {};
  std::string name;
  int low_value;
  int high_value;
  int volume;
  int mass;
};

enum Good_id
{
  GOOD_NULL,        //  0
  
  GOOD_ORE,         //  1; Raw ores
  GOOD_METALS,      //  2; Processed metals
  GOOD_ELECTRONICS, //  3; "Raw" electronic components
  GOOD_CONSUMER,    //  4; Consumer goods
  GOOD_MACHINERY,   //  5; Machinery for farming, construction, etc.

  GOOD_WATER,       //  6; Potable water
  GOOD_FERTILIZER,  //  7; Fertilizers for crops
  GOOD_CROPS,       //  8; Raw crops
  GOOD_FOODS,       //  9; Processed food components (e.g. flour)
  GOOD_EDIBLES,     // 10; Prepared food

  GOOD_RAWFUEL,     // 11; Raw, unusable fuel
  GOOD_SOLIDFUEL,   // 12; Solid fuel
  GOOD_HYDROGEN,    // 13; Hydrogen fuel
  GOOD_URANIUM,     // 14; Raw uranium fuel

  NUM_GOODS
};

struct Good_value
{
  Good_id id;
  int amount;
};

#endif
