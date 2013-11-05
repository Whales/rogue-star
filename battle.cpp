#include "battle.h"
#include "cuss.h"
#include "window.h"

void Battle::main_loop()
{
  cuss::interfact i_battle;
  if (!i_battle.load_from_file("cuss/i_battle.cuss")) {
    debugmsg("Couldn't load cuss/i_battle.cuss");
  }

  Window w_battle(0, 0, 80, 24);

  i_battle.set_data("text_player", PLR.name);
  i_battle.set_data("num_player_crew_skeleton", PLR.skeleton_crew());
  i_battle.set_data("num_player_crew_full", PLR.full_crew());
  i_battle.set_data("list_player_parts", 

  bool done = false;
  while (!done) {
    long ch = getch();
    if (ch == ' ' || ch == '\n') {
      player_turn();
      enemy_turn();
    
  
