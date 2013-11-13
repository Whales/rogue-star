#include "goods.h"
#include "globals.h"
#include "planet.h"
#include "ship.h"
#include "event.h"
#include "window.h"

#define NEW_GOOD(name, min, max, volume, mass) \
  GOOD_DATA.push_back( \
    new Good_datum(name, min, max, volume, mass) )

#define NEW_SP_WEAP(name, hp, weight, cost, crew, sym, fg, damage, fire_rate,\
                    acc, range) \
  PARTS.push_back( \
    new SP_weapon(name, hp, weight, cost, crew, glyph(sym, fg, c_black),\
                  damage, fire_rate, acc, range) )

#define NEW_SP_ARMOR(name, hp, weight, cost, crew, physical, energy )\
  PARTS.push_back( \
    new SP_armor (name, hp, weight, cost, crew, physical, energy ) )

#define NEW_SP_DEVICE(name, hp, weight, cost, crew)\
  PARTS.push_back( \
    new SP_device(name, hp, weight, cost, crew) )

#define NEW_SP_CARGO(name, hp, weight, cost, crew, bunks, volume)\
  PARTS.push_back( \
    new SP_cargo(name, hp, weight, cost, crew, bunks, volume) )

#define NEW_SP_ENGINE(name, hp, weight, cost, crew, fuel, power_output,\
                      mass_breakpoint, interstellar_speed, evasive_speed)\
  PARTS.push_back( \
    new SP_engine(name, hp, weight, cost, crew, fuel, power_output,\
                  mass_breakpoint, interstellar_speed, evasive_speed) )

void define_goods()
{
  GOOD_DATA.push_back( new Good_datum("Null", 0, 0, 0, 0) );

/* Numbers on right side are max possible profit with 100 cargo space; after
 * semi-colon is profit divided by mass
 */

// Manufacturing
  NEW_GOOD("ore",                    500,  3000,  10,  50); //  2500;   50
  NEW_GOOD("metals",                1000,  8000,  10, 100); //  7000;   70
  NEW_GOOD("elec. components",       100,   500,   5,   5); //   800;  160
  NEW_GOOD("consumer electronics",  1000,  2500,   5,   5); //  3000;  600
  NEW_GOOD("machinery",             5000, 30000,  20, 120); // 10000;  188

// Farming
  NEW_GOOD("water",                   20,   200,   5,   5); //   360;   72
  NEW_GOOD("fertilizer",              50,   300,   4,  10); //   625;   63
  NEW_GOOD("crops",                  400,  2000,  10,   8); //  1600;  200
  NEW_GOOD("flour & dried foods",    800,  2000,   5,  10); //  2400;  240
  NEW_GOOD("prepared food",         1500,  5000,   8,  15); //  4375;  292

// Fuel
  NEW_GOOD("raw fuel",                10,    50,   1,   1); //   400;  400
  NEW_GOOD("solid fuel",              40,   100,   1,   2); //   600;  300
  NEW_GOOD("hydrogen fuel",           80,   200,   1,   3); //  1200;  400
  NEW_GOOD("uranium",                150,   400,   1,   5); //  2500;  500
  
}

void define_parts()
{
  PARTS.push_back( new Ship_part_type() ); // Null

//              Name              HP  Mass  Cost Crew
  NEW_SP_WEAP   ("plasma cannon", 10,  60, 12000,  2,
//  Sym  FG     Dam Rate Acc Range
    '^', c_cyan,  5,  1,  3,  1000);

//              Name              HP  Mass  Cost Crew
  NEW_SP_ARMOR  ("armor plating", 50, 100,  8000,  0,
//  Phys energy
      20, 0);

  NEW_SP_ARMOR  ("basic shield",  10,  25, 10000,  2,
//  Phys energy
       0, 10);

//              Name              HP  Mass  Cost Crew
  NEW_SP_DEVICE ("test device",    1,   1,  3000,  2);

//              Name              HP  Mass  Cost Crew Bunks Storage
  NEW_SP_CARGO  ("cargo bay",      5,  40,  4000,  2,  0,   20);

//              Name              HP  Mass  Cost Crew
  NEW_SP_CARGO  ("berth",          5,  20,  4000,  1, 25,    0);

//              Name              HP  Mass  Cost Crew
  NEW_SP_ENGINE ("TX-1 Engine",   15,  75, 14000,  2,
//              FUEL           POWER  MBP Speed Evade
                GOOD_SOLIDFUEL,  20,  500, 50,    100);

}

#define NEW_EVENT(name, chance, color, desc, ...) \
  EVENTS.push_back( new Event(name, desc, chance, color, __VA_ARGS__, 0) )

void define_events()
{
  NEW_EVENT("gas crisis", 1600, c_ltred,
"A gas crisis has broken out.  Demand increases on all planets.",
//  Good      Planet Static Percent  Supply  Demand  Time
GOOD_RAWFUEL,     -1,     0,     10,     -1,      2,  100,
GOOD_SOLIDFUEL,   -1,     0,     10,      0,      3,  100,
GOOD_HYDROGEN,    -1,     0,     10,      0,      3,  100,
GOOD_URANIUM,     -1,     0,     10,      0,      3,  100
);

  NEW_EVENT("water contamination", 300, c_ltred,
"Contamination on <planet> has caused a drop in the water supply.",
//  Good      Planet Static Percent  Supply  Demand  Time
GOOD_WATER,        0,    10,      0,     -4,      1,   80
);

  NEW_EVENT("motherlode", 20, c_ltgreen,
"Miners on <planet> have found a motherlode of metallic ores!",
GOOD_ORE,          0,     0,    -10,      3,      0,   20
);

  NEW_EVENT("big release", 400, c_ltgreen,
"<company:electronic> just released their new product, <product:consumer>, to its rabid fans.  Demand outstrips supply!",
GOOD_CONSUMER,    -1,   200,      0,      1,      2,   50
);

  NEW_EVENT("bountiful harvest", 1200, c_ltgreen,
"<planet> has experienced a bumper crop this year, increasing output greatly.",
//  Good      Planet Static Percent  Supply  Demand  Time
GOOD_CROPS,        0,     0,    -10,      4,      0,  100
);

  NEW_EVENT("motherlode", 200, c_ltgreen,
"Miners on <planet> have found a motherload of uranium!",
GOOD_URANIUM,      0,   -50,      0,      4,      0,   20
);

  NEW_EVENT("protesters",  300, c_ltred,
"Protesters from <acronym> have shut down uranium production on <planet>.",
GOOD_URANIUM,      0,     0,      0,     -8,      0,   40,
GOOD_URANIUM,     -1,     0,     20,      0,      0,   50
);

  NEW_EVENT("health scare", 400, c_ltred,
"<company:food> has made a recall on a contaminated batch of <product:edibles>.",
GOOD_EDIBLES,     -1,    10,      0,     -1,     -3,   30
);

  NEW_EVENT("civil war", 4000, c_red,
"A civil war on <planet> has decreased production of all goods, and shifted demand towards necessities.",
//  Good      Planet Static Percent  Supply  Demand  Time
GOOD_ORE,          0,     0,      0,     -3,      0,  100,
GOOD_METALS,       0,     0,     10,     -3,      1,  100,
GOOD_ELECTRONICS,  0,     0,      5,     -3,      1,  100,
GOOD_CONSUMER,     0,     0,      0,     -5,     -4,  100,
GOOD_MACHINERY,    0,     0,      0,     -4,      0,  100,
GOOD_WATER,        0,     0,     20,     -1,      2,  100,
GOOD_FERTILIZER,   0,     0,      0,      0,     -1,  100,
GOOD_CROPS,        0,     0,      0,     -4,     -2,  100,
GOOD_FOODS,        0,     0,     10,     -2,      1,  100,
GOOD_EDIBLES,      0,     0,     30,     -1,      5,  100,
GOOD_RAWFUEL,      0,     0,      0,     -3,     -1,  100,
GOOD_SOLIDFUEL,    0,     0,     20,     -1,      4,  100,
GOOD_HYDROGEN,     0,     0,     20,     -2,      3,  100,
GOOD_URANIUM,      0,     0,     20,     -2,      3,  100
);

  NEW_EVENT("processing breakthrough", 1000, c_ltgreen,
"A breakthrough in hydrogen processing technology has caused a temporary dip in prices!  Hydrogen is especially available on <planet>, where the technology was invented.",
//  Good      Planet Static Percent  Supply  Demand  Time
GOOD_HYDROGEN,    -1,     0,    -25,      0,      0,   25,
GOOD_HYDROGEN,     0,     0,      0,      5,      0,   40
);

  NEW_EVENT("factory strikes", 300, c_ltred,
"A series of strikes has reduced production of various goods on <planet>.",
GOOD_METALS,       0,     0,      0,     -2,      0,   30,
GOOD_ELECTRONICS,  0,     0,      0,     -4,      0,   30,
GOOD_CONSUMER,     0,     0,      0,     -6,      0,   30,
GOOD_MACHINERY,    0,     0,      0,     -6,      0,   30,
GOOD_EDIBLES,      0,     0,      0,     -2,      0,   30,
GOOD_SOLIDFUEL,    0,     0,      0,     -2,      0,   30
);

  NEW_EVENT("blight", 600, c_ltred,
"A disease has stricken crops on <planet>, reducing their output this year.",
//  Good      Planet Static Percent  Supply  Demand  Time
GOOD_CROPS,        0,     0,      0,     -7,      0,  100
);

  NEW_EVENT("gardening fad", 600, c_ltblue,
"A home gardening fad has increased demand for fertilizer and reduced crop \
prices slightly.",
GOOD_FERTILIZER,  -1,     0,     10,      0,      1,   50,
GOOD_CROPS,       -1,  -100,      0,      0,      0,   50
);

  NEW_EVENT("home robotics fad", 1200, c_ltblue,
"Home robotics is a popular hobby; <company:electronic> is producing a home \
robotics kit that is quite successful.  Electrical components are in high \
demand, while consumer electronics see their price drop slightly.",
GOOD_ELECTRONICS, -1,     0,     15,      0,      2,   50,
GOOD_CONSUMER,    -1,     0,    -15,      0,      0,   30
);

  NEW_EVENT("population boom",  500, c_ltgreen,
"<planet> is experiencing a population boom, and demand for food, water, and \
building supplies is high.",
//  Good      Planet Static Percent  Supply  Demand  Time
GOOD_METALS,       0,     0,      5,      0,      3,   40,
GOOD_ELECTRONICS,  0,     0,      5,      0,      3,   40,
GOOD_WATER,        0,     0,      0,      0,      5,   60,
GOOD_FOODS,        0,     0,     10,      0,      2,  100,
GOOD_EDIBLES,      0,     0,     10,      0,      4,   80,
GOOD_SOLIDFUEL,    0,     0,      0,      0,      2,   80
);

}

void define_ships()
{
  NPC_SHIPS.push_back( new Ship() );

  Ship* tmpship = new Ship();
  tmpship->name = "Fighter";
  tmpship->add_part(SP_PLASMA);
  tmpship->add_part(SP_ENGINE);
  NPC_SHIPS.push_back( tmpship );
}
