#ifndef _SHIP_H_
#define _SHIP_H_

#include <string>
#include <vector>
#include "goods.h"

struct Ship_part_type
{
  std::string name;
  int max_hp;
  int mass;
  int cost;
  Ship_part_type(std::string N = "NULL", int MHP = 0, int M = 0, int C = 0) :
    name (N), max_hp (MHP), mass (M), cost (C) {};
  virtual bool is_weapon()  { return false; }
  virtual bool is_armor()   { return false; }
  virtual bool is_device()  { return false; }
  virtual bool is_cargo()   { return false; }
  virtual bool is_engine()  { return false; }
};

struct SP_weapon : public Ship_part_type
{
  SP_weapon(std::string N = "NULL", int MHP = 0, int M = 0, int C = 0,
                                    int D = 0, int FR = 0, int A = 0) :
    Ship_part_type(N, MHP, M, C), 
    damage (D), fire_rate (FR), accuracy (A) {};
  virtual bool is_weapon()  { return true; }
  int damage;
  int fire_rate;
  int accuracy;
};

struct SP_armor : public Ship_part_type
{
  SP_armor(std::string N = "NULL", int MHP = 0, int M = 0, int C = 0,
                                   int P = 0, int E = 0) :
    Ship_part_type(N, MHP, M, C), 
    physical (P), energy (E) {};
  virtual bool is_armor()   { return true; }
  int physical;
  int energy;
};

struct SP_device : public Ship_part_type
{
  SP_device(std::string N = "NULL", int MHP = 0, int M = 0, int C = 0) :
    Ship_part_type(N, MHP, M, C)
    {};
  virtual bool is_device()  { return true; }
};

struct SP_cargo : public Ship_part_type
{
  SP_cargo(std::string N = "NULL", int MHP = 0, int M = 0, int C = 0,
                                   int B = 0, int V = 0) :
    Ship_part_type(N, MHP, M, C),
    bunks (B), volume (V) {};
  virtual bool is_cargo()   { return true; }
  int bunks;
  int volume;
};

struct SP_engine : public Ship_part_type
{
  SP_engine(std::string N = "NULL", int MHP = 0, int M = 0, int C = 0,
            Good_id F = GOOD_NULL, int PO = 0, int MB = 0, int IS = 0,
            int ES = 0) :
    Ship_part_type(N, MHP, M, C),
    fuel (F), power_output (PO), mass_breakpoint (MB),
    interstellar_speed (IS), evasive_speed (ES) {};

  virtual bool is_engine()  { return true; }
  Good_id fuel;
  int power_output; // = (mass * distance) / fuel
  int mass_breakpoint; // power_output and speed are halved above this
  int interstellar_speed;
  int evasive_speed;
};

enum Ship_part_id
{
  SP_NULL = 0,
  SP_PLASMA,
  SP_PLATING,
  SP_SHIELD,
  SP_DEVICE_TEST,
  SP_CARGO_BAY,
  SP_BERTH,
  SP_ENGINE,
  NUM_SP
};

struct Ship_part
{
  Ship_part_type *type;
  int hp;
  int charge;

  Ship_part(Ship_part_type *T = NULL);

  int sell_price();

  void repair();
};

struct Manifest
{
  Manifest();
  ~Manifest(){};

  void add_good(Good_id good, int am, int pr);
  bool rem_good(Good_id good, int am);

  int amount[NUM_GOODS];
  int price [NUM_GOODS];
};

struct Ship
{
  std::string name;
  std::vector<Ship_part> parts;
  Manifest cargo;
  int posx, posy;
  int cash, debt;
  int rep_trader, rep_combat, rep_criminal;

  int trade_volume, rep_trade_accel;

  int crew_count, crew_morale;

  Ship();
  ~Ship(){};

  void init();

  void add_part(Ship_part_id type);
  void add_part(Ship_part_type *type);
  void add_part(Ship_part part);

  void update_rep();

  Ship_part* get_engine();
  std::string engine_name();
  int fuel_remaining();
  int fuel_cap();
  Good_id fuel_type();

  int cargo_remaining();
  int total_mass();
  int mass_breakpoint();

  int speed();
  int fuel_economy();

  int travel_time(int distance);
  int travel_cost(int distance);

  void travel_to(int index);
  void expend_fuel(int amount);
  void add_fuel(int amount);
  void fill_tank();
};

enum NPC_ship_id
{
  NPCSHIP_NULL = 0,
  NPCSHIP_FIGHTER,
  NPCSHIP_MAX
};

#endif
