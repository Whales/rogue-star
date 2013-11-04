#include "ship.h"
#include "globals.h"

struct Battle
{
  std::vector<Ship> enemies;

  void main_loop();
  void enemy_turn();
  void player_turn();

  void add_enemy(Ship enemy);
};
