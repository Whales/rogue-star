#include <sstream>
#include <stdlib.h>
#include <math.h>
#include "cuss.h"
#include "glyph.h"
#include "planet.h"
#include "globals.h"
#include "interface.h"

void prep_repair_data(std::vector<std::string> &parts_repair_names,
                      std::vector<std::string> &parts_repair_dmg,
                      std::vector<std::string> &parts_repair_prices,
                      std::vector<int> &parts_repair_indices,
                      int &price_repair_all);

void main_screen()
{
  cuss::interface i_menu;
  if (!i_menu.load_from_file("cuss/i_menu.cuss")) {
    debugmsg("Couldn't load cuss/i_menu.cuss");
    return;
  }

  Window w_menu(0, 0, 80, 24);

  do {
    i_menu.set_data("text_date", WORLD.get_date_str());
    i_menu.set_data("num_cash", PLR.cash);
    i_menu.set_data("num_debt", PLR.debt);
    i_menu.set_data("text_news", WORLD.get_news());
    i_menu.draw(&w_menu);
    char ch = getch();
    switch (ch) {
      case 's':
      case 'S':
        ship_screen();
        break;

      case 'm':
      case 'M':
        travel_screen();
        break;

      case 't':
      case 'T':
        trade_screen();
        break;

      case 'd':
      case 'D':
        dock_screen();
        break;

      default:
        i_menu.handle_keypress(ch);
        break;
    }
  } while (true);
}

void ship_screen()
{
  cuss::interface i_ship;
  if (!i_ship.load_from_file("cuss/i_ship.cuss")) {
    debugmsg("Couldn't load cuss/i_ship.cuss");
    return;
  }


  Window w_ship(0, 0, 80, 24);

  std::vector<std::string> manifest_list, parts_list, manifest_amount,
                           manifest_cost;

  for (int i = 0; i < PLR.parts.size(); i++) {
    parts_list.push_back(PLR.parts[i].type->name);
  }

  for (int i = 0; i < NUM_GOODS; i++) {
    if (PLR.cargo.amount[i] > 0) {
      std::stringstream amount_text, cost_text;
      amount_text << PLR.cargo.amount[i];
      cost_text << PLR.cargo.price[i];
      manifest_list.push_back(GOOD_DATA[i]->name);
      manifest_amount.push_back(amount_text.str());
      manifest_cost.push_back(cost_text.str());
    }
  }

  i_ship.set_data("list_equipment", parts_list);
  i_ship.set_data("list_manifest", manifest_list);
  i_ship.set_data("list_amount", manifest_amount);
  i_ship.set_data("list_cost", manifest_cost);
  i_ship.set_data("text_shipname", PLR.name);
  i_ship.set_data("text_location", planet_name_at(PLR.posx, PLR.posy));
  i_ship.set_data("text_engine", PLR.engine_name());
  i_ship.set_data("num_fuel", PLR.fuel_remaining());
  i_ship.set_data("num_maxfuel", PLR.fuel_cap());
  i_ship.set_data("num_speed", 100 / PLR.speed());
  int mass = PLR.total_mass(), breakpoint = PLR.mass_breakpoint();
  i_ship.set_data("num_mass", mass);
  i_ship.set_data("num_mass_limit", breakpoint);

  if (PLR.total_mass() > PLR.mass_breakpoint()) {
    std::stringstream overweight_text;
    overweight_text << "<c=red>Fuel efficiency and speed are at " <<
                       int(100 / (1 + mass / breakpoint)) << "%%%%";
    i_ship.set_data("text_overweight", overweight_text.str());
  }

  std::string rep_trader   = "<c=green>",
              rep_combat   = "<c=ltblue>",
              rep_criminal = "<c=ltred>";

  for (int i = 0; i < 200; i += 10) {
    if (i < PLR.rep_trader) {
      rep_trader   += '#';
    } else if (i - PLR.rep_trader < 10) {
      rep_trader   += "<c=dkgray>.";
    } else {
      rep_trader   += '.';
    }
    if (i < PLR.rep_combat) {
      rep_combat   += '#';
    } else if (i - PLR.rep_combat < 10) {
      rep_combat   += "<c=dkgray>.";
    } else {
      rep_combat   += '.';
    }
    if (i < PLR.rep_criminal) {
      rep_criminal += '#';
    } else if (i - PLR.rep_criminal < 10) {
      rep_criminal += "<c=dkgray>.";
    } else {
      rep_criminal += '.';
    }
  }

  rep_trader   += "<c=/>";
  rep_combat   += "<c=/>";
  rep_criminal += "<c=/>";

  i_ship.set_data("text_rep_trader",   rep_trader  );
  i_ship.set_data("text_rep_combat",   rep_combat  );
  i_ship.set_data("text_rep_criminal", rep_criminal);

  do {
    i_ship.draw(&w_ship);
    long ch = getch();
    if (ch == 'q' || ch == 'Q' || ch == KEY_ESC) {
      return;
    } else {
      i_ship.handle_action(ch);
    }
  } while (true);
}

void trade_screen()
{
  Planet* market = planet_at(PLR.posx, PLR.posy);
  if (!market) {
    popup("You are in deep space.  You cannot buy anything here!");
    return;
  }

  cuss::interface i_trade;
  if (!i_trade.load_from_file("cuss/i_trade.cuss")) {
    debugmsg("Couldn't load cuss/i_trade.cuss!");
    return;
  }
  Window w_trade(0, 0, 80, 24);



  int buy_amount[NUM_GOODS],   sell_amount[NUM_GOODS];
  int lowest_price[NUM_GOODS], highest_price[NUM_GOODS];
  std::vector<std::string> buy_text, sell_text, buy_price_text, sell_price_text,
                           buy_amount_text, sell_amount_text;
  for (int i = 1; i < NUM_GOODS; i++) {
// Set buying amounts to 0
    buy_amount[i]  = 0;
    sell_amount[i] = 0;
// Fetch & store names
    std::stringstream name_text;
    name_text << GOOD_DATA[i]->name;
    buy_text.push_back( name_text.str() );
// Add a "quantity owned" suffix for the Selling side
    for (int n = 0; n < 21 - GOOD_DATA[i]->name.length(); n++) {
      name_text << " ";
    }
    name_text << "x";
    int amt = PLR.cargo.amount[i];
    if (amt < 10) {
      name_text << " ";
    }
    if (amt < 100) {
      name_text << " ";
    }
    if (amt < 1000) {
      name_text << " ";
    }
    name_text << amt;
    sell_text.push_back( name_text.str() );
// Find lowest/highest price for this item
    lowest_price[i] = 99999;
    highest_price[i] = 0;
    for (int n = 0; n < PLANETS.size(); n++) {
      int tmp_buy  = PLANETS[n].buy_price(  Good_id(i) ),
          tmp_sell = PLANETS[n].sell_price( Good_id(i) );
      if (tmp_buy < lowest_price[i] && tmp_buy > 0) {
        lowest_price[i] = tmp_buy;
      }
      if (tmp_sell > highest_price[i] && tmp_sell > 0) {
        highest_price[i] = tmp_sell;
      }
    }
// Find and store the prices
    std::stringstream buy_price_ss, sell_price_ss;
    int bp = market->buy_price(  Good_id(i) ),
        sp = market->sell_price( Good_id(i) );
    if (bp > 0) {
      if (bp >= highest_price[i]) {
        buy_price_ss << "<c=dkgray>";
      } else if (bp == lowest_price[i]) {
        buy_price_ss << "<c=ltgreen>";
      } else {
        buy_price_ss << "<c=ltgray>";
      }
      buy_price_ss << bp << "<c=/>";
    } else {
      buy_price_ss << "<c=dkgray>N/A<c=/>";
    }
    if (sp > 0) {
      if (sp == highest_price[i]) {
        sell_price_ss << "<c=ltgreen>";
      } else {
        sell_price_ss << "<c=ltgray>";
      }
      sell_price_ss << sp << "<c=/>";
    } else {
      sell_price_ss << "<c=dkgray>N/A<c=/>";
    }
    buy_price_text.push_back(  buy_price_ss.str()  );
    sell_price_text.push_back( sell_price_ss.str() );
  }

  i_trade.set_data("list_buy_name",  buy_text);
  i_trade.set_data("list_buy_price",  buy_price_text);
  i_trade.set_data("list_sell_name", sell_text);
  i_trade.set_data("list_sell_price", sell_price_text);
  i_trade.set_data("num_plr_cash", PLR.cash);
  i_trade.set_data("num_plr_cargo", PLR.cargo_remaining());

  i_trade.select("list_buy_name");

  do {
    int earnings = 0, cost = 0, transport = 0, cargo_used = 0;
    i_trade.clear_data("list_buy_amount");
    i_trade.clear_data("list_sell_amount");
    for (int i = 1; i < NUM_GOODS; i++) {
      std::stringstream buy_amount_ss, sell_amount_ss;
      buy_amount_ss << buy_amount[i];
      sell_amount_ss << sell_amount[i];
      i_trade.add_data("list_buy_amount", buy_amount_ss.str());
      i_trade.add_data("list_sell_amount", sell_amount_ss.str());
      cost     += buy_amount[i]  * market->buy_price(  Good_id(i) );
      earnings += sell_amount[i] * market->sell_price( Good_id(i) );
/* TODO: Allow for different modes of transport, with different costs
 *       e.g. "Planetary Landing" "Drop Ships" "Space Elevator" "Hire Crew"
 */
      transport += sell_amount[i]  * GOOD_DATA[i]->mass;
      transport += buy_amount[i]   * GOOD_DATA[i]->mass;
      cargo_used += buy_amount[i]  * GOOD_DATA[i]->volume;
      cargo_used -= sell_amount[i] * GOOD_DATA[i]->volume;
    }

    transport = (transport * market->gravity) / 100;

    bool buying = (i_trade.selected()->name == "list_buy_name");
    int sel     = i_trade.selected()->get_int();
    sel++;    // This is because the lists start from 0; but that's really
              // GOOD_DATA[1]!
    int buy_this = market->buy_price( Good_id(sel) ),
        sell_this = market->sell_price( Good_id(sel) );
    if (buy_this == -1) {
      buy_this = 0;
    }
    if (sell_this == -1) {
      sell_this = 0;
    }

    int profit = earnings - cost - transport;
    i_trade.set_data("num_earnings", earnings);
    i_trade.set_data("num_cost", cost);
    i_trade.set_data("num_transport", transport);
    i_trade.set_data("num_total",  profit);
    i_trade.set_data("num_profit", profit);
    i_trade.set_data("num_after", PLR.cash + profit);
    if (profit > 0) {
      i_trade.set_data("num_after", c_green);
    } else if (PLR.cash + profit >= 0) {
      i_trade.set_data("num_after", c_ltgreen);
    } else {
      i_trade.set_data("num_after", c_red);
    }
    i_trade.set_data("num_cargo_after", PLR.cargo_remaining() - cargo_used);
    if (cargo_used < 0) {
      i_trade.set_data("num_cargo_after", c_green);
    } else if (PLR.cargo_remaining() >= cargo_used) {
      i_trade.set_data("num_cargo_after", c_ltgreen);
    } else {
      i_trade.set_data("num_cargo_after", c_red);
    }
    i_trade.set_data("num_price", (buying ? buy_this : sell_this) );
    i_trade.set_data("num_lowest", lowest_price[sel]);
    i_trade.set_data("num_highest", highest_price[sel]);

    i_trade.draw(&w_trade);
    long ch = getch();

    int max_buyable = 0;
    if (buying) {
      max_buyable = (PLR.cash + profit) / market->buy_price( Good_id(sel) );
      int max_vol = (PLR.cargo_remaining() - cargo_used) /
                    GOOD_DATA[sel]->volume;
      if (max_vol < max_buyable) {
        max_buyable = max_vol;
      }
    }
    max_buyable += buy_amount[sel];

    if (ch == 'm' || ch == 'M') {
      if (buying) {
        buy_amount[sel] = max_buyable;
      } else {
        sell_amount[sel] = PLR.cargo.amount[sel];
      }
    } else if (ch == '+' || ch == KEY_RIGHT || ch == 'l' || ch == 'L') {
      if (buying) {
        buy_amount[sel]++;
      } else if (PLR.cargo.amount[sel] > sell_amount[sel]) {
        sell_amount[sel]++;
      }
    } else if (ch == '-' || ch == KEY_LEFT || ch == 'h' || ch == 'H') {
      if (buying) {
        if (buy_amount[sel] > 0) {
          buy_amount[sel]--;
        }
      } else if (sell_amount[sel] > 0) {
        sell_amount[sel]--;
      }
    } else if (ch == '\n') {
      if (PLR.cash + profit < 0) {
        popup("You cannot afford this purchase!");
      } else if (PLR.cargo_remaining() < cargo_used) {
        popup("You do not have cargo space for this purchase!");
      } else {
        PLR.cash += profit;
        for (int i = 1; i < NUM_GOODS; i++) {
          PLR.cargo.rem_good( Good_id(i), sell_amount[i] );
          PLR.cargo.add_good( Good_id(i),  buy_amount[i],
                              market->buy_price( Good_id(i) ) );
          PLR.trade_volume += sell_amount[i] * market->sell_price( Good_id(i) );
        }
        return;
      }
    } else if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      return; // TODO: Prompt for confirmation of cancel
    } else {
      i_trade.handle_action(ch);
    }
  } while (true);
}

void dock_screen()
{
  cuss::interface i_dock;
  if (!i_dock.load_from_file("cuss/i_dock.cuss")) {
    debugmsg("Couldn't load cuss/i_dock.cuss");
    return;
  }

  Window w_dock(0, 0, 80, 24);

  Planet* market = planet_at(PLR.posx, PLR.posy);
  if (!market) {
    debugmsg("Somehow got to the dock while not at a planet!");
    return;
  }

  bool buying_parts = true; // Are we buying, or selling parts?
  std::vector<std::string> parts_for_sale;
  std::vector<std::string> part_prices;
  for (int i = 1; i < PARTS.size(); i++) {
    parts_for_sale.push_back( PARTS[i]->name );
    std::stringstream ppss;
    ppss << PARTS[i]->cost;
    part_prices.push_back( ppss.str() );
  }

  i_dock.set_data("list_parts_to_buy",      parts_for_sale);
  i_dock.set_data("list_parts_to_buy_cost", part_prices);

  i_dock.set_data("text_parts_status",
                  "You are <c=ltblue>buying<c=/> parts.");
  i_dock.set_data("text_parts_instructions",
                  "Press <c=magenta>/<c=/> to sell parts.");

// Fuel
  int fuel_units = 0;
  i_dock.ref_data("num_fuel_buy_units", &fuel_units);
  i_dock.ref_data("num_plr_cash", &(PLR.cash));

// Parts to repair
  std::vector<std::string> parts_repair_names, parts_repair_dmg,
                           parts_repair_prices;
  std::vector<int> parts_repair_indices;
  int price_repair_all = 0;

  prep_repair_data(parts_repair_names, parts_repair_dmg, parts_repair_prices,
                   parts_repair_indices, price_repair_all);
// Populate repair-related fields

  i_dock.set_data("list_parts_to_repair", parts_repair_names);
  i_dock.set_data("list_part_damage",     parts_repair_dmg);
  i_dock.set_data("list_repair_cost",     parts_repair_prices);
  i_dock.set_data("num_price_repair_all", price_repair_all);

  i_dock.select("list_parts_to_repair");

  do {
    Good_id fuel_type = PLR.fuel_type();
    int fuel_amt = PLR.fuel_remaining(), fuel_cap = PLR.fuel_cap(),
        fuel_price = market->fuel_price(fuel_type);

// Check fuel_units for inbounds
    if (fuel_units < 0) {
      fuel_units = 0;
    } else if (fuel_units > fuel_cap - fuel_amt) {
      fuel_units = fuel_cap - fuel_amt;
      fuel_units -= fuel_units % 10; // Make sure it's a multiple of 10
    }

    int fuel_unit_price = fuel_units * fuel_price,
        fuel_fill_price = (fuel_cap - fuel_amt) * fuel_price;

    int fuel_gauge = (20 * fuel_amt) / fuel_cap;
    int fuel_gauge_plus = (20 * (fuel_amt + fuel_units)) / fuel_cap;

    i_dock.clear_data("draw_fuel_gauge");
    for (int i = 0; i < fuel_gauge_plus; i++) {
      glyph tile('.', c_red, c_red);
      if (i < fuel_gauge) {
        if (i < 5) {
          tile.bg = c_red;
        } else if (i < 10) {
          tile.bg = c_brown;
        } else {
          tile.bg = c_green;
        }
      } else {
        tile.bg = c_ltgray;
      }
      tile.fg = tile.bg;
      i_dock.set_data("draw_fuel_gauge", tile, 0, 19 - i);
      i_dock.set_data("draw_fuel_gauge", tile, 1, 19 - i);
    }

// Populate fuel-related fields
    i_dock.set_data("num_fuel", fuel_amt);
    i_dock.set_data("num_fuel_max", fuel_cap);
    i_dock.set_data("num_fuel_price", fuel_price);

    if (fuel_type == GOOD_NULL) {
      i_dock.set_data("text_fuel_name", "<c=red>NO ENGINE<c=/>");
    } else {
      std::string fuel_name_truncated = GOOD_DATA[fuel_type]->name;
      size_t space_loc = fuel_name_truncated.find(' ');
      if (space_loc != std::string::npos) {
        fuel_name_truncated = fuel_name_truncated.substr(0, space_loc);
      }
      i_dock.set_data("text_fuel_name", fuel_name_truncated);
    }

    i_dock.set_data("num_price_fill", fuel_fill_price);
    i_dock.set_data("num_price_fuel_units", fuel_unit_price);

    i_dock.draw(&w_dock);

    long ch = getch();

    if (ch == 'f' || ch == 'F') { // Fill up
      if (PLR.cash < fuel_fill_price) {
        popup("Cannot afford to fill the tank! You: $%d, price: $%d", PLR.cash,
              fuel_fill_price);
      } else {
        PLR.cash -= fuel_fill_price;
        PLR.fill_tank();
      }
    } else if (ch == 'b' || ch == 'B') { // Buy units of fuel
      if (PLR.cash < fuel_unit_price) {
        popup("Cannot afford that! You: $%d, price: $%d", PLR.cash,
              fuel_unit_price);
      } else {
        PLR.cash -= fuel_unit_price;
        PLR.add_fuel(fuel_units);
        fuel_units = 0;
      }
    } else if (ch == 'r' || ch == 'R') { // Repair all parts
      if (PLR.cash < price_repair_all) {
        popup("Cannot afford that! You: $%d, price: $%d", PLR.cash,
              price_repair_all);
      } else {
        PLR.cash -= price_repair_all;
        for (int i = 0; i < PLR.parts.size(); i++) {
          PLR.parts[i].repair();
        }
      }
    } else if (ch == 'u' || ch == 'U') { // Upgrade part
// TODO: This.
      debugmsg("Not implemented.");
    } else if (ch == '/') { // Toggle buy/sell parts
      buying_parts = !buying_parts;
      if (buying_parts) {
// parts_for_sale and part_prices are defined outside of the loop, as they are
// static
        i_dock.set_data("list_parts_to_buy",      parts_for_sale);
        i_dock.set_data("list_parts_to_buy_cost", part_prices);
        i_dock.set_data("text_parts_status",
                        "You are <c=ltblue>buying<c=/> parts.");
        i_dock.set_data("text_parts_instructions",
                        "Press <c=magenta>/<c=/> to sell parts.");
      } else {
        std::vector<std::string> parts_owned_names, parts_owned_prices;
        for (int i = 0; i < PLR.parts.size(); i++) {
          parts_owned_names.push_back(PLR.parts[i].type->name);
          std::stringstream popss;
          popss << PLR.parts[i].sell_price();
          parts_owned_prices.push_back( popss.str() );
        }
        i_dock.set_data("list_parts_to_buy", parts_owned_names);
        i_dock.set_data("list_parts_to_buy_cost", parts_owned_prices);
        i_dock.set_data("text_parts_status",
                        "You are <c=ltgreen>selling<c=/> parts.");
        i_dock.set_data("text_parts_instructions",
                        "Press <c=magenta>/<c=/> to buy parts.");
      }
    } else if (ch == '\n') {
      if (i_dock.selected()->name == "list_parts_to_repair") {
        if (parts_repair_indices.empty()) {
          popup("Nothing to repair.  Press TAB to switch to parts shop.");
        } else {
          int interface_index = i_dock.get_int("list_parts_to_repair");
          int index = parts_repair_indices[ interface_index ];
          Ship_part* part_repaired = &(PLR.parts[index]);
          int repair_cost = 20 *
                            (part_repaired->type->max_hp - part_repaired->hp);
  
          if (PLR.cash < repair_cost) {
            popup("Can't afford to repair that!  You: $%d, cost: $%d", PLR.cash,
                  repair_cost);
          } else {
            PLR.cash -= repair_cost;
            part_repaired->repair();
            prep_repair_data(parts_repair_names, parts_repair_dmg,
                             parts_repair_prices, parts_repair_indices,
                             price_repair_all);
          }
        }
      } else { // We're on the parts buy/sell list!
        if (buying_parts) {
// We add one since the 0th entry in the interface list is really the 1st
// part in the database.
          int buy_index = i_dock.get_int("list_parts_to_buy") + 1;
          if (PLR.cash < PARTS[buy_index]->cost) {
            popup("Can't afford that!  You: $%d, price: $%d", PLR.cash,
                  PARTS[buy_index]->cost);
          } else {
            PLR.cash -= PARTS[buy_index]->cost;
            PLR.add_part( PARTS[buy_index] );
          }
        } else { // We're SELLING parts.
// No need to add 1 here.
          int sell_index = i_dock.get_int("list_parts_to_buy");
          Ship_part *sold = &(PLR.parts[sell_index]);
          if (query_yn("Sell your %s for $%d?", sold->type->name.c_str(),
                       sold->sell_price())) {
            PLR.cash += sold->sell_price();
            PLR.parts.erase( PLR.parts.begin() + sell_index );
            std::vector<std::string> parts_owned_names, parts_owned_prices;
            for (int i = 0; i < PLR.parts.size(); i++) {
              parts_owned_names.push_back(PLR.parts[i].type->name);
              std::stringstream popss;
              popss << PLR.parts[i].sell_price();
              parts_owned_prices.push_back( popss.str() );
            }
            i_dock.set_data("list_parts_to_buy", parts_owned_names);
            i_dock.set_data("list_parts_to_buy_cost", parts_owned_prices);
          }
        }
      } // Done checking whether we're repairing or trading
    } else if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      return;
    } else {
      i_dock.handle_action(ch);
    }
  } while (true);
}

void travel_screen()
{
  cuss::interface i_planets;
  if (!i_planets.load_from_file("cuss/i_planets.cuss")) {
    debugmsg("Couldn't load cuss/i_planets.cuss");
    return;
  }
  cuss::interface i_prices;
  if (!i_prices.load_from_file("cuss/i_prices.cuss")) {
    debugmsg("Couldn't load cuss/i_prices.cuss");
    return;
  }

// Populate lowest/highest prices
  int lowest_price[NUM_GOODS], highest_price[NUM_GOODS];
  for (int i = 1; i < NUM_GOODS; i++) {
    lowest_price[i] = 99999;
    highest_price[i] = 0;
    for (int n = 0; n < PLANETS.size(); n++) {
      int tmp_buy  = PLANETS[n].buy_price(  Good_id(i) ),
          tmp_sell = PLANETS[n].sell_price( Good_id(i) );
      if (tmp_buy < lowest_price[i] && tmp_buy > 0) {
        lowest_price[i] = tmp_buy;
      }
      if (tmp_sell > highest_price[i] && tmp_sell > 0) {
        highest_price[i] = tmp_sell;
      }
    }
  }

  Window w_planets(0, 0, 80, 24);
  Window w_prices(28, 5, 36, 18);

  std::vector<int>  planet_distance;
  std::vector<std::string> planet_text;
  bool draw_prices = false;
  int selection_start = 0;
  for (int i = 0; i < PLANETS.size(); i++) {
    std::stringstream data;
    int dx = PLR.posx - PLANETS[i].posx;
    int dy = PLR.posy - PLANETS[i].posy;
    int distance = int( sqrt( dx * dx + dy * dy ) );
    if (distance == 0) {
      selection_start = i;
    }
    if (PLR.travel_cost(distance) > PLR.fuel_remaining()) {
      data << "<c=dkgray>";
    } else {
      bool destination_market = false;
      for (int n = 1; n < NUM_GOODS; n++) {
        int sellp = PLANETS[i].sell_price( Good_id(n) );
        int buyp  = PLANETS[i].buy_price(  Good_id(n) );
        if (PLR.cargo.amount[n] > 0 && PLR.cargo.price[n] < sellp) {
          destination_market = true;
          if (sellp == highest_price[n]) {
            data << "<c=green>";
          } else {
            data << "<c=ltblue>";
          }
        } else if (!destination_market && buyp == lowest_price[n]) {
          destination_market = true;
          data << "<c=white>";
        }
      }
      if (!destination_market) {
        data << "<c=ltgray>";
      }
    }
          
    data << PLANETS[i].name << " ";
    for (int j = 0; j < 12 - PLANETS[i].name.length(); j++) {
      data << " ";
    }
    if (distance < 10) {
      data << " ";
    }
    data << distance << " parsecs";
    data << "<c=/>";
    planet_distance.push_back(distance);
    planet_text.push_back(data.str());
  }

  i_planets.set_data("list_planets", planet_text);
// Select the planet we're on
  i_planets.set_data("list_planets", selection_start);
  i_planets.select("list_planets");

  do {
    int sel = i_planets.get_int("list_planets");
    Planet* dest = &(PLANETS[sel]);
    i_planets.set_data("text_biome", dest->biome_name);
    i_planets.set_data("text_pop", dest->pop_name);
    i_planets.set_data("text_industry", dest->industry_name);
    int ttime = PLR.travel_time(planet_distance[sel]),
        tcost = PLR.travel_cost(planet_distance[sel]);
    std::stringstream ssttime, sstcost;
    if (ttime == -1) {
      ssttime << "<c=red>N/A<c=/>";
    } else {
      ssttime << ttime;
    }
    if (tcost == -1) {
      sstcost << "<c=red>N/A<c=/>";
    } else {
      if (tcost > PLR.fuel_remaining()) {
        sstcost << "<c=red>";
      } else if (tcost == PLR.fuel_remaining()) {
        sstcost << "<c=ltred>";
      }
      sstcost << tcost << "<c=/>";
    }
    i_planets.set_data("text_distance",   ssttime.str());
    i_planets.set_data("text_travelcost", sstcost.str());
    i_planets.set_data("num_fuel_remaining", PLR.fuel_remaining());
    std::stringstream gravtext;
// thus, 78 becomes 0.78, 100 becomes 1.00, 120 becomes 1.20
    gravtext << int(dest->gravity / 100) << ".";
    if (dest->gravity % 100 < 10) {
      gravtext << "0"; // 108 becomes 1.08, not 1.8
    }
    gravtext << dest->gravity % 100;
    i_planets.set_data("text_gravity", gravtext.str());
    i_planets.clear_data("draw_starmap");
// Draw the starmap, OR draw prices
    if (draw_prices) {
      std::vector<std::string> good_text, buy_text, sell_text;
      for (int i = GOOD_NULL + 1; i < NUM_GOODS; i++) {
        std::string good_name = GOOD_DATA[i]->name;
        good_name += "....................";
        good_text.push_back(good_name);
        std::stringstream buy, sell;
        int buyp  = dest->buy_price(  Good_id(i) );
        int sellp = dest->sell_price( Good_id(i) );
        if (buyp == -1) {
          buy << "<c=dkgray>N/A<c=/>";
        } else {
          if (buyp >= highest_price[i]) {
            buy << "<c=dkgray>";
          } else if (buyp == lowest_price[i]) {
            buy << "<c=green>";
          } else {
            buy << "<c=ltgray>";
          }
          buy << buyp << "<c=/>";
        }
        if (sellp == -1) {
          sell << "<c=dkgray>N/A<c=/>";
        } else {
          if (sellp == highest_price[i]) {
            sell << "<c=green>";
          } else {
            sell << "<c=ltgray>";
          }
          sell << sellp << "<c=/>";
        }
        buy_text.push_back ( buy.str());
        sell_text.push_back(sell.str());
      }
      i_prices.set_data("list_prices", good_text);
      i_prices.set_data("list_buy",    buy_text );
      i_prices.set_data("list_sell",   sell_text);
    } else {
      for (int i = 0; i < PLANETS.size(); i++) {
        glyph planet_sym = PLANETS[i].sym;
        int px = PLANETS[i].posx, py = PLANETS[i].posy;
        if (i == sel) {
          planet_sym.bg = c_magenta;
        }
        if (px == PLR.posx && py == PLR.posy) {
          planet_sym.symbol = '@';
        }
        i_planets.set_data("draw_starmap", planet_sym, px, py);
      }
    }

    i_planets.draw(&w_planets);
    if (draw_prices) {
      i_prices.draw(&w_prices);
    }
    long ch = getch();
    if (ch == '\n') {
// Travel to selected planet
      if (PLR.fuel_remaining() < tcost) {
        popup("You do not have sufficient fuel (<c=red>%d<c=/>/%d)!",
              PLR.fuel_remaining(), tcost);
      } else {
        PLR.travel_to(sel);
        return;
      }
    } else if (ch == 'q' || ch == 'Q' || ch == KEY_ESC) {
      return;
    } else if (ch == '/') {
      draw_prices = !draw_prices;
    } else {
      i_planets.handle_action(ch);
    }
  } while (true);
}

void prep_repair_data(std::vector<std::string> &parts_repair_names,
                      std::vector<std::string> &parts_repair_dmg,
                      std::vector<std::string> &parts_repair_prices,
                      std::vector<int> &parts_repair_indices,
                      int &price_repair_all)
{
  for (int i = 0; i < PLR.parts.size(); i++) {
    int part_dmg = PLR.parts[i].type->max_hp - PLR.parts[i].hp;
    if (part_dmg > 0) {
      parts_repair_names.push_back( PLR.parts[i].type->name );
      parts_repair_indices.push_back(i);
      std::stringstream prpss, prdss;
      prdss << part_dmg;
// TODO: Don't hard-code this.
      prpss << 20 * part_dmg;
      price_repair_all += 20 * part_dmg;
      parts_repair_dmg.push_back(prdss.str());
      parts_repair_prices.push_back(prpss.str());
    }
  }
}
