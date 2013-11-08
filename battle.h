#ifndef _BATTLE_H_
#define _BATTLE_H_

#include "ship.h"
#include "globals.h"
#include "window.h"
#include <string>
#include <vector>

enum Crew_task
{
  CREW_NULL = 0,
  CREW_REPAIR,
  CREW_EMP_REPAIR,
  CREW_ENGINE,
  CREW_WEAPONS,
  NUM_CREW_TASKS
};

enum Engine_task
{
  ENGINE_NULL = 0,
  ENGINE_EVADE,
  ENGINE_FLEE,
  ENGINE_CLOSE,
  ENGINE_JUMP,
  NUM_ENGINE_TASKS
};

const std::string Crew_task_name[NUM_CREW_TASKS] = {
"Idle",
"Repairing",
"Fix EMP",
"Engine",
"Weapons"
};

const std::string Engine_task_name[NUM_ENGINE_TASKS] = {
"NULL",
"Evading",
"Fleeing",
"Closing",
"Jumping"
};

struct Combat_ship
{
  Ship ship;
  Crew_task crew;
  Engine_task engine;
  int range;
// TODO: Morale?  Objective?
};

struct Battle
{
  std::vector<Combat_ship> enemies;
  std::vector<std::string> messages;

  Crew_task crew;
  Engine_task engine;
  int target;

// Constructor/Destructor
  Battle();
  ~Battle();
// In-battle functions
  void main_loop();
  void update_ranges();
  void player_turn();
  void enemy_turn();
  void hit_with_weapon(Ship &ship, Ship_part &weapon);
// Interface functions
  void add_message(std::string message);
  void set_crew_task();
  void set_engine_task();
  void set_target(Window &w_battle);
  void display_weapon_symbols();
// Init functions
  void add_enemy(Ship enemy);
};

#endif
