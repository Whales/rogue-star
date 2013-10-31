#include "ship.h"
#include "globals.h"

struct Battle
{
  std::vector<Ship> enemies;

  void enemy_turn();
  void player_turn();
};
