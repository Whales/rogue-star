#include "battle.h"
#include "cuss.h"
#include "window.h"
#include <sstream>

Battle::Battle()
{
  crew = CREW_NULL;
  engine = ENGINE_EVADE;
}

Battle::~Battle()
{
}

void Battle::main_loop()
{
  cuss::interfact i_battle;
  if (!i_battle.load_from_file("cuss/i_battle.cuss")) {
    debugmsg("Couldn't load cuss/i_battle.cuss");
  }

  Window w_battle(0, 0, 80, 24);

  i_battle.set_data("text_player", PLR.name);

  bool done = false;
  while (!done) {
// Start by populating all fields
    i_battle.set_data("num_player_crew", PLR.crew_amount);
    i_battle.set_data("num_player_crew_skeleton", PLR.skeleton_crew());
    i_battle.set_data("num_player_crew_full", PLR.full_crew());
    i_battle.set_data("num_player_speed", PLR.combat_speed());
    std::vector<std::string> player_parts, player_parts_hp;
    for (int i = 0; i < PLR.parts.size(); i++) {
      player_parts.push_back( PLR.parts[i].type->name );
      std::stringstream hp_text;
      hp_text << PLR.parts[i].hp_color_tag() << PLR.parts[i].hp << "<c=/>";
      player_parts_hp.push_back( hp_text.str() );
    }
    i_battle.set_data("list_player_parts", player_parts);
    i_battle.set_data("list_player_parts_hp", player_parts_hp);
    i_battle.set_data("text_
    long ch = getch();
    if (ch == ' ' || ch == '\n') {
      player_turn();
      enemy_turn();
    } else if (ch == 'c' || ch == 'C') {
    }
  }
}
  
