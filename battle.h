#ifndef _BATTLE_H_
#define _BATTLE_H_

#include "ship.h"
#include "globals.h"

enum Crew_task
{
  CREW_NULL = 0,
  CREW_REPAIR,
  CREW_DEIONIZE,
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
"De-ionize",
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

struct Battle
{
  std::vector<Ship> enemies;

  Crew_task crew;
  Engine_task engine;

  Battle();
  ~Battle();
  void main_loop();
  void enemy_turn();
  void player_turn();

  void add_enemy(Ship enemy);
};

#endif
