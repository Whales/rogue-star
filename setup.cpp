#include "setup.h"
#include "rng.h"
#include "planet.h"
#include "globals.h"

void init_planets()
{
  int cur_id = 0;
  for (int x = 0; x <= 18; x += 18) {
    for (int y = 0; y < 18; y += 3) {
      Planet tmp;
      tmp.posx = rng(x, x + 17);
      tmp.posy = rng(y, y + 2);
      tmp.randomize();
      tmp.uid = cur_id;
      cur_id++;
      PLANETS.push_back(tmp);
    }
  }
}
