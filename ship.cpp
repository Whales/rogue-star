#include "ship.h"
#include "globals.h"
#include "window.h"
#include "rng.h"
#include <math.h>

Ship::Ship()
{
  name = "Vindicator";
  posx = 0;
  posy = 0;
  cash = 5000;
  debt = 8000;

  rep_trader = 0;
  rep_combat = 0;
  rep_criminal = 0;

  trade_volume = 0;
  rep_trade_accel = 0;

  crew_amount = 25;
  crew_morale = 80;
}

void Ship::init()
{
  //add_part(SP_PLASMA);
  //add_part(SP_PLATING);
  add_part(SP_CARGO_BAY);
  add_part(SP_BERTH);
  add_part(SP_ENGINE);
  get_engine()->charge = 1000;

  posx = PLANETS[0].posx;
  posy = PLANETS[0].posy;
}

void Ship::add_part(Ship_part_id type)
{
  add_part( PARTS[type] );
}

void Ship::add_part(Ship_part_type *type)
{
  if (!type) {
    return;
  }

  add_part( Ship_part(type) );
}

void Ship::add_part(Ship_part part)
{
// TODO: Limit us to one engine
  parts.push_back(part);
}

void Ship::update_rep()
{
  if (trade_volume > 0) {
    int accel_change = trade_volume / 100;
    rep_trade_accel += accel_change;
    trade_volume = 0;
  }

  if (rep_trade_accel > rep_trader) {
    rep_trader++;
    if (rep_trader > 250) {
      rep_trader = 250;
    }
    rep_trade_accel *= 0.8;
  } else if (rep_trader == 0) {
    rep_trade_accel = 0;
  } else if (rep_trade_accel < 0) {
    if (0 - rng(1, 15) >= rep_trade_accel) {
      rep_trader--;
    }
    if (rep_trade_accel > -10) {
      rep_trade_accel--;
    }
  } else {
    rep_trade_accel -= 1 + rep_trader / 50;
  }

}

Ship_part* Ship::get_engine()
{
  for (int i = 0; i < parts.size(); i++) {
    if (parts[i].type->is_engine()) {
      return &(parts[i]);
    }
  }
  return NULL;
}
  
std::string Ship::engine_name()
{
  Ship_part* engine = get_engine();
  if (engine) {
    return engine->type->name;
  }
  return "<c=red>None<c=/>";
}

int Ship::fuel_remaining()
{
  Ship_part* engine = get_engine();
  if (engine) {
    return engine->charge;
  }
  return 0;
}

int Ship::fuel_cap()
{
// TODO: Chaaaaange.
  if (get_engine()) {
    return 1000;
  }
  return 1;
}

Good_id Ship::fuel_type()
{
  Ship_part* engine = get_engine();
  if (engine) {
    return (static_cast<SP_engine*>(engine->type))->fuel;
  }
  return GOOD_NULL;
}

int Ship::cargo_remaining()
{
  int ret = 0;
  for (int i = 0; i < parts.size(); i++) {
    if (parts[i].type->is_cargo()) {
      ret += (static_cast<SP_cargo*>(parts[i].type))->volume;
    }
  }
  for (int i = 0; i < NUM_GOODS; i++) {
    ret -= cargo.amount[i] * GOOD_DATA[i]->volume;
  }
  return ret;
}

int Ship::total_mass()
{
  int ret = 0;
  for (int i = 0; i < parts.size(); i++) {
    ret += parts[i].type->mass;
  }
  for (int i = 0; i < NUM_GOODS; i++) {
    ret += cargo.amount[i] * GOOD_DATA[i]->mass;
  }

  return ret;
}

int Ship::mass_breakpoint()
{
  Ship_part *engine = get_engine();
  if (!engine) {
    return 0;
  }
  return (static_cast<SP_engine*>(engine->type))->mass_breakpoint;
}

int Ship::speed()
{
  Ship_part* engine = get_engine();
  if (engine) {
    SP_engine* engine_data = static_cast<SP_engine*>(engine->type);
    int ret = engine_data->interstellar_speed;
    ret /= (1 + total_mass() / engine_data->mass_breakpoint);
    return ret;
  }
  return -1;
}

int Ship::fuel_economy()
{
  Ship_part* engine = get_engine();
  if (engine) {
    SP_engine* engine_data = static_cast<SP_engine*>(engine->type);
    int ret = total_mass() / engine_data->power_output; // Fuel per parsec
    ret /= (1 + total_mass() / engine_data->mass_breakpoint);
    return ret;
  }
  return -1;
}
  

int Ship::travel_time(int distance)
{
  int ispeed = speed();
  if (ispeed == -1) {
    return -1;
  }
  return (100 * distance) / ispeed;
}

int Ship::travel_cost(int distance)
{
  Ship_part* engine = get_engine();
  if (engine) {
    SP_engine* engine_data = static_cast<SP_engine*>(engine->type);
    int ret = (total_mass() * distance) / engine_data->power_output;
    ret /= (1 + total_mass() / engine_data->mass_breakpoint);
    return ret;
  }
  return -1;
}

void Ship::travel_to(int index)
{
  if (index < 0 || index >= PLANETS.size()) {
    debugmsg("Tried to travel to planet index %d (%d exist)", index,
             PLANETS.size());
    return;
  }

  Planet *dest = &(PLANETS[index]);
  int dx = posx - dest->posx;
  int dy = posy - dest->posy;
  int distance = int( sqrt( dx * dx + dy * dy ) );

  int fuel_cost = travel_cost( distance );
  int time      = travel_time( distance );
  if (fuel_remaining() < fuel_cost) {
    debugmsg("Called travel_to with insufficient fuel");
    return;
  }

  expend_fuel( fuel_cost );
  WORLD.advance_days( time );

  posx = dest->posx;
  posy = dest->posy;
}

void Ship::expend_fuel(int amount)
{
  Ship_part* engine = get_engine();
  if (engine) {
    engine->charge -= amount;
    if (engine->charge < 0) {
      engine->charge = 0;
    }
  }
}

void Ship::add_fuel(int amount)
{
  Ship_part* engine = get_engine();
  if (engine) {
    engine->charge += amount;
    if (engine->charge > fuel_cap()) {
      engine->charge = fuel_cap();
    }
  }
}

void Ship::fill_tank()
{
  Ship_part* engine = get_engine();
  if (engine) {
    engine->charge = fuel_cap();
  }
}

int Ship::skeleton_crew()
{
  int ret = 0;
// How many teams does our crew need to split into?
  int num_teams = 1 + parts.size() / 3;
  std::vector<int> biggest_req;
  
  for (int i = 0; i < parts.size(); i++) {
    for (std::vector<int>::iterator it = biggest_req.begin();
         it != biggest_req.end(); it++) {
      if (parts[i].type->crew_requirement > (*it)) {
        biggest_req.insert(it, parts[i].type->crew_requirement);
      }
    }
  }

  for (int i = 0; i < num_teams && i < biggest_req.size(); i++) {
    ret += biggest_req[i];
  }

  if (ret < 1) {
    return 1; // We at least need a pilot!
  }
  if (ret > crew_requirement()) {
    return crew_requirement();  // Shouldn't happen, but...
  }
  return ret;
}

int Ship::crew_requirement()
{
  int ret = 1;
  for (int i = 0; i < parts.size(); i++) {
    ret += parts[i].type->crew_requirement;
  }
  return ret;
}

std::string Ship::morale_level_name()
{
  if (crew_amount <= 0) {
    return "<c=red>No crew!<c=/>";
  }
  if (crew_morale < 15) {
    return "<c=red>Mutinous!<c=/>";
  }
  if (crew_morale < 30) {
    return "<c=ltred>Discontent<c=/>";
  }
  if (crew_morale < 45) {
    return "<c=yellow>Unhappy<c=/>";
  } 
  if (crew_morale < 60) {
    return "<c=yellow>Fair<c=/>";
  }
  if (crew_morale < 75) {
    return "<c=ltgreen>Good<c=/>";
  }
  if (crew_morale < 90) {
    return "<c=ltgreen>Great<c=/>";
  }
  return "<c=green>Fantastic!<c=/>";
}

Ship_part::Ship_part(Ship_part_type *T)
{
  charge = 0;
  if (T) {
    type = T;
    hp = type->max_hp;
  }
}

int Ship_part::sell_price()
{
  if (!type) {
    return 0;
  }
  int ret = type->cost;
  ret *= hp;
  ret /= type->max_hp;
  ret *= .9; // TODO: Don't hardcode this.
  return ret;
}

void Ship_part::repair()
{
  if (!type) {
    return;
  }
  hp = type->max_hp;
}

Manifest::Manifest()
{
  for (int i = 0; i < NUM_GOODS; i++) {
    amount[i] = 0;
    price[i]  = 0;
  }
}

void Manifest::add_good(Good_id good, int am, int pr)
{
  if (am == 0) {
    return;
  }
  int total_price = amount[good] * price[good];
  total_price += am * pr;
  amount[good] += am;
  price[good] = total_price / amount[good];
}

bool Manifest::rem_good(Good_id good, int am)
{
  if (amount == 0) {
    return false;
  }
  amount[good] -= am;
  if (amount[good] < 0) {
    amount[good] = 0;
    return false;
  }
  return true;
}

