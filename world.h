#ifndef _WORLD_H_
#define _WORLD_H_

#include <string>
#include <vector>
#include "event.h"

struct Event;

struct News_item
{
  News_item(std::string T = "", int S = 0, int E = 0) :
    text (T), start (S), end (E)
    {};
  std::string text;
  int start, end;
};

struct World
{
  World();
  ~World(){};

  std::string get_date_str();
  std::string get_news();

  void advance_days(int amount);
  void update_prices(int total_supply[NUM_GOODS], int total_demand[NUM_GOODS]);
  void add_event   (Event *ev);
  void add_news    (std::string text, std::string planet_name, nc_color color,
                    int start, int end);

  int day;
  std::vector<Event> current_events;
  std::vector<News_item> news;
  int price_adjustment[NUM_GOODS];
};

#endif
