#include <ctime>
#include "window.h"
#include "interface.h"
#include "cuss.h"
#include "rng.h"
#include "setup.h"
#include "data.h"
#include "globals.h"
#include "battle.h"

int main()
{
  srand(time(NULL));
  init_display();

  init_planets();
  define_goods();
  define_parts();
  define_events();
  PLR.init();

/*
  PLR.add_part(SP_PLATING);
  PLR.add_part(SP_PLATING);
  PLR.add_part(SP_PLATING);
  PLR.add_part(SP_PLATING);
  PLR.add_part(SP_SHIELD);
  PLR.add_part(SP_SHIELD);
  PLR.add_part(SP_SHIELD);

  Ship enemy;
  enemy.init();

  Battle test;
  test.add_enemy(enemy);
  test.add_enemy(enemy);
  test.init();

  test.main_loop();
*/

  main_screen();

  endwin();
  return 0;
}

