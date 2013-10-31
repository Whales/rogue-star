#include <sstream>
#include <stdlib.h>
#include "world.h"
#include "window.h"
#include "globals.h"
#include "rng.h"
#include "string_tags.h"

std::string short_date_string(int day);

World::World()
{
  day = 1;
  news.clear();
  for (int i = 0; i < NUM_GOODS; i++) {
    price_adjustment[i] = 0;
  }
}

std::string World::get_date_str()
{
  std::stringstream ret;
  int year = 3000 + day / 100;
  ret << "Year " << year << ", day " << day % 100;
  return ret.str();
}

void World::advance_days(int amount)
{
  if (amount == 0) {
    return;
  }
  int total_supply[NUM_GOODS], total_demand[NUM_GOODS];
  for (int i = 0; i < NUM_GOODS; i++) {
    total_supply[i] = 0;
    total_demand[i] = 0;
  }
  for (int i = 0; i < PLANETS.size(); i++) {
    for (int n = 0; n < NUM_GOODS; n++) {
      total_supply[n] += PLANETS[i].supply[n];
      total_demand[n] += PLANETS[i].supply[n];
    }
  }
    
  day += amount;
// Make minimum debt payments
  for (int i = 0; i < amount; i++) {
    update_prices(total_supply, total_demand);
    PLR.update_rep();
    if (PLR.debt > 0) {
// TODO: Don't hard-code minimum payment.
      int min_payment = 10 + (PLR.debt / 250);
      if (min_payment > PLR.debt) {
        min_payment = PLR.debt;
      }
      if (PLR.cash < min_payment) {
// Increase debt rather than make a payment!
        min_payment -= PLR.cash;
        PLR.cash = 0;
        PLR.debt += min_payment;
      } else {
        PLR.cash -= min_payment;
        PLR.debt -= min_payment;
      }
    }
    for (int n = 0; n < current_events.size(); n++) {
      for (int m = 0; m < current_events[n].modifiers.size(); m++) {
        if (day >= current_events[n].modifiers[m].timeout) {
          current_events[n].modifiers.erase(
            current_events[n].modifiers.begin() + m );
          m--;
        }
      }
      if (current_events[n].modifiers.empty()) {
        current_events.erase( current_events.begin() + n );
        n--;
      }
    }
    for (int n = 0; n < EVENTS.size(); n++) {
      if (one_in(4) && one_in(EVENTS[n]->frequency)) {
        add_event(EVENTS[n]);
      }
    }
  }
}

void World::update_prices(int total_supply[NUM_GOODS],
                          int total_demand[NUM_GOODS])
{
  for (int i = 0; i < NUM_GOODS; i++) {
    int diff = total_supply[i] - total_demand[i];
    int unit = (GOOD_DATA[i]->high_value - GOOD_DATA[i]->low_value) / 10;
    int curr = price_adjustment[i] / unit;
    if (diff < 0) {
      if (diff < curr) {
        price_adjustment[i] -= rng(0, abs(diff));
      }
    } else if (diff > curr) {
      price_adjustment[i] += rng(0, diff);
    }
  }
}

void World::add_event(Event* ev)
{
  Event new_event("Nope", "No way", 0, 0);
  new_event.name = ev->name;
  new_event.description = ev->description;
  int num_planets = PLANETS.size();
  int planet_used[num_planets];
  int end_date = day;
  for (int i = 0; i < num_planets; i++) {
    planet_used[i] = -1;
  }
  for (int i = 0; i < ev->modifiers.size(); i++) {
    Price_modifier tmpmod;
    Price_modifier *ref = &(ev->modifiers[i]);
    int planet = ref->planet_id;
    if (planet != -1) {
      if (planet_used[planet] == -1) {
        planet_used[planet] = rng(0, num_planets - 1);
      }
      tmpmod.planet_id = planet_used[planet];
    } else {
      tmpmod.planet_id = -1;
    }
    tmpmod.good           = ref->good;
    tmpmod.static_change  = ref->static_change;
    tmpmod.percent_change = ref->percent_change;
    tmpmod.supply_change  = ref->supply_change;
    tmpmod.demand_change  = ref->demand_change;
    tmpmod.timeout        = day + ref->timeout;
    if (tmpmod.timeout > end_date) {
      end_date = tmpmod.timeout;
    }
    new_event.modifiers.push_back(tmpmod);
  }
  current_events.push_back(new_event);
  std::string planet_name = "the galaxy";
  if (planet_used[0] != -1) {
    planet_name = PLANETS[ planet_used[0] ].name;
  }
  add_news( new_event.description, planet_name, day, end_date );
}

void World::add_news(std::string text, std::string planet_name, int start,
                     int end)
{
  std::string processed = process_string_tags( text, planet_name );
  News_item tmp(processed, start, end);
  news.push_back(tmp);
}

std::string World::get_news()
{
  if (news.empty()) {
    return "No news.";
  }
  std::string ret;
  for (int i = 0; i < news.size(); i++) {
    if (news[i].end <= day) {
      news.erase( news.begin() + i);
      i--;
    } else {
      ret += "*** " + short_date_string(news[i].start) + " - " +
             short_date_string(news[i].end) + " ***\n";
      ret += news[i].text + "\n";
    }
  }
  return ret;
}

std::string short_date_string(int day)
{
  std::stringstream ret;
  int year = 3000 + day / 100;
  ret << year << "-" << day % 100;
  return ret.str();
}
