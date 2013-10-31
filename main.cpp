#include <ctime>
#include "window.h"
#include "interface.h"
#include "cuss.h"
#include "rng.h"
#include "setup.h"
#include "data.h"
#include "globals.h"

int main()
{
  srand(time(NULL));
  init_display();

  init_planets();
  define_goods();
  define_parts();
  define_events();
  PLR.init();


  main_screen();

  endwin();
  return 0;
}

