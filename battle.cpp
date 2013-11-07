#include "battle.h"
#include "cuss.h"
#include "window.h"
#include <sstream>

Battle::Battle()
{
  crew   = CREW_NULL;
  engine = ENGINE_EVADE;
  target = -1;
}

Battle::~Battle()
{
}

void Battle::main_loop()
{
  cuss::interface i_battle;
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
    i_battle.set_data("text_player_armor", PLR.armor_meter());
    i_battle.set_data("text_player_shields", PLR.shields_meter());
    std::vector<std::string> player_parts, player_parts_hp;
    for (int i = 0; i < PLR.parts.size(); i++) {
      player_parts.push_back( PLR.parts[i].type->name );
      std::stringstream hp_text;
      hp_text << PLR.parts[i].hp_color_tag() << PLR.parts[i].hp << "<c=/>";
      player_parts_hp.push_back( hp_text.str() );
    }
    i_battle.set_data("list_player_parts",    player_parts);
    i_battle.set_data("list_player_parts_hp", player_parts_hp);
    i_battle.set_data("text_crew_status",     Crew_task_name[crew]);
    i_battle.set_data("text_engine_status",   Engine_task_name[engine]);
    i_battle.set_data("text_messages",        messages);

    for (int i = 0; i < enemies.size() && i < 3; i++) {
      std::stringstream enemy_num_ss;
      Ship* ship = &(enemies[i].ship);
      enemy_num_ss << (i + 1);
      std::string num = enemy_num_ss.str();
      std::string ele_enemy   = "text_enemy_"         + num,
                  ele_armor   = "text_enemy_armor_"   + num,
                  ele_shields = "text_enemy_shields_" + num,
                  ele_engines = "text_enemy_engines_" + num,
                  ele_weapons = "text_enemy_weapons_" + num;
      i_battle.set_data(ele_enemy,   ship->name            );
      i_battle.set_data(ele_armor,   ship->armor_meter()   );
      i_battle.set_data(ele_shields, ship->shields_meter() );
      i_battle.set_data(ele_engines, ship->engine_meter()  );
      i_battle.set_data(ele_weapons, ship->weapon_symbols());
    }
    i_battle.draw(&w_battle);
// We do these manual drawing functions after drawing the interface, since they
// are "on top" of the interface
    switch (target) {
      case 0:
        for (int y = 4; y <= 10; y++) {
          w_battle.putch(51, y, c_red, c_black, LINE_XOXO);
        }
        w_battle.putch(51, 3, c_red, c_black, LINE_OXXO);
        for (int x = 52; x <= 54; x++) {
          w_battle.putch(x, 3, c_red, c_black, LINE_OXOX);
        }
        w_battle.putch(55, 3, c_red, c_black, '>');
        break;

      case 1:
        for (int x = 52; x <= 54; x++) {
          w_battle.putch(x, 11, c_red, c_black, LINE_OXOX);
        }
        w_battle.putch(55, 11, c_red, c_black, '>');
        break;

      case 2:
        for (int y = 12; y <= 19; y++) {
          w_battle.putch(51, y, c_red, c_black, LINE_XOXO);
        }
        w_battle.putch(51, 19, c_red, c_black, LINE_XXOO);
        for (int x = 52; x <= 54; x++) {
          w_battle.putch(x, 19, c_red, c_black, LINE_OXOX);
        }
        w_battle.putch(55, 19, c_red, c_black, '>');
        break;
    }
    if (enemies.size() < 3) {
      w_battle.clear_area(56, 16, 79, 22);
    }
    if (enemies.size() < 2) {
      w_battle.clear_area(56,  8, 79, 14);
    }
        
    long ch = getch();
    if (ch == ' ' || ch == '\n') {
// TODO: Confirm if no target, confirm if no crew task, etc.
      update_ranges();
      player_turn();
      enemy_turn();
    } else if (ch == 'c' || ch == 'C') {
      set_crew_task();
    } else if (ch == 'e' || ch == 'E') {
      set_engine_task();
    } else if (ch == 't' || ch == 'T') {
      set_target(w_battle);
    } else if (ch == '!') {
      display_weapon_symbols();
    }
  }
}

void Battle::update_ranges()
{
  

void Battle::player_turn()
{
// First, fire guns
  if (target >= 0 && target < enemies.size()) {
    for (int i = 0; i < PLR.parts.size(); i++) {
      if (PLR.parts[i].type->is_weapon()) {
        SP_weapon* weap_data = static_cast<SP_weapon*>(PLR.parts[i].type);
        if (PLR.parts[i].charge < weap_data->fire_rate) {
          PLR.parts[i].charge++;
        } else {
          PLR.parts[i].charge = 0;
          std::stringstream mess_text;
          mess_text << "You fire your " << weap_data->name << " at " <<
                       enemies[target].ship.name << "!";
          add_message(mess_text.str());
      
}

void Battle::enemy_turn()
{
}

void Battle::add_message(std::string message)
{
  messages.push_back(message);
}

void Battle::set_crew_task()
{
  bool need_emp_repair = false, need_repair = false;
  for (int i = 0; i < PLR.parts.size(); i++) {
    if (PLR.parts[i].emp > 0) {
      need_emp_repair = true;
    }
    if (PLR.parts[i].hp < PLR.parts[i].type->max_hp) {
      need_repair = true;
    }
  }

  std::vector<std::string> options;
  for (int i = 0; i < NUM_CREW_TASKS; i++) {
    if ( Crew_task(i) == CREW_REPAIR && !need_repair) {
      std::stringstream opt;
      opt << "<c=dkgray>" << Crew_task_name[i] << "<c=/>";
      options.push_back(opt.str());
    } else if (Crew_task(i) == CREW_EMP_REPAIR && !need_emp_repair) {
      std::stringstream opt;
      opt << "<c=dkgray>" << Crew_task_name[i] << "<c=/>";
      options.push_back(opt.str());
    } else {
      options.push_back( Crew_task_name[i] );
    }
  }

  int choice = menu_vec("Crew task:", options);
  crew = Crew_task(choice);
}

void Battle::set_engine_task()
{
  std::vector<std::string> options;
  for (int i = 1; i < NUM_ENGINE_TASKS; i++) { // Start with 1 since 0 is NULL
    options.push_back( Engine_task_name[i] );
  }
  int choice = menu_vec("Engine task:", options);
  engine = Engine_task(choice);
}

void Battle::display_weapon_symbols()
{
  std::string output;
  for (int i = 0; i < NUM_SP; i++) {
    if (PARTS[i]->is_weapon()) {
      SP_weapon* weap_data = static_cast<SP_weapon*>(PARTS[i]);
      std::stringstream data_ss;
      data_ss << weap_data->symbol.text_formatted() << ' ' <<
                 weap_data->name;
    }
  }
  popup_fullscreen(output.c_str());
}

void Battle::set_target(Window &w_battle)
{
  for (int i = 0; i < enemies.size() && i < 3; i++) {
    std::stringstream letter;
    int y = 3 + 8 * i;
    w_battle.putch(53, y, c_magenta, c_black, 'A' + i);
    w_battle.putch(52, y - 1, c_white,   c_black, LINE_OXXO);
    w_battle.putch(53, y - 1, c_white,   c_black, LINE_OXOX);
    w_battle.putch(54, y - 1, c_white,   c_black, LINE_OOXX);
    w_battle.putch(52, y    , c_white,   c_black, LINE_XOXO);
    w_battle.putch(53, y    , c_magenta, c_black, 'A' + i  );
    w_battle.putch(54, y    , c_white,   c_black, LINE_XOXO);
    w_battle.putch(52, y + 1, c_white,   c_black, LINE_XXOO);
    w_battle.putch(53, y + 1, c_white,   c_black, LINE_OXOX);
    w_battle.putch(54, y + 1, c_white,   c_black, LINE_XOOX);
  }

  w_battle.refresh();
  long ch = getch();
  if (ch == 'a' || ch == 'A') {
    target = 0;
  } else if (ch == 'b' || ch == 'B') {
    target = 1;
  } else if (ch == 'c' || ch == 'C') {
    target = 2;
  } else {
    target = -1;
  }
}
