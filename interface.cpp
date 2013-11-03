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

      case '?':
        popup_scrollable("\
Welcome to Rogue Star!  You can close this help screen, and any other popups, \
by pressing Enter or Esc.  You can scroll this text with the \
<c=magenta>up<c=/>/<c=magenta>down<c=/> keys, or <c=magenta>J<c=/>/\
<c=magenta>K<c=/> (i.e. vikeys).  You can scroll down a page with \
<c=magenta>Spacebar<c=/>, and scroll up a page with <c=magenta>B<c=/>.  \
Generally speaking, key commands are NOT case-sensitive.\n\
\n\
Rogue Star is a game of space trading.  You are the proud new owner of a \
freighter capable of traveling much faster than the speed of light, via \
SCIENCE.  Your objective is to make a fortune by buying goods at low prices, \
transporting them to another planet, and selling them at higher prices.\n\
\n\
This screen is the main menu.  On the right, news is listed, which may impact \
the prices of various goods.  Each news item is prefaced by the date range it \
applies to.\n\
A quick note about dates: since we're in outer space, 'year' and 'day' aren't \
really meaningful.  The Galactic Consortium of Dates and Times has set the \
standard 'day' to 25 hours, and the standard 'year' to 100 days.\n\
\n\
From this screen, you can access several other screens which will let you get \
things done:\n\
Press <c=magenta>S<c=/> to view your <c=magenta>Ship<c=/>.  This will list \
your current manifest (i.e. what cargo you are carrying), your location, the \
modules that are currently a part of your ship, your remaining fuel, and \
current speed and mass.  It also displays what your reputation is.\n\
Press <c=magenta>M<c=/> to view the <c=magenta>Starmap<c=/>.  This will allow \
you to view information about other planets, and to travel to them if you have \
enough fuel.\n\
Press <c=magenta>T<c=/> to <c=magenta>Trade<c=/>.  This is where you can buy \
and sell various goods.\n\
Press <c=magenta>D<c=/> to <c=magenta>Dock<c=/> your ship.  This will allow \
you to refuel, repair any damage, and sell or buy parts.\n\
Press <c=magenta>Q<c=/> to <c=magenta>Quit<c=/>.  Except don't!\n\
\n\
Remember, once you get to any other screen you can press <c=magenta>?<c=/> to \
view specific help for that screen.\n\
Press <c=magenta>Enter<c=/> or <c=magenta>Esc<c=/> to close this help.");
      break;

      case 'q':
      case 'Q':
        if (query_yn("Really quit (without saving)?")) {
          return;
        }
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
    if (ch == '?') {
      popup_scrollable("\
This is the <c=magenta>Ship<c=/> screen.  This screen is not interactive; it \
is only there to provide information.  You can return to the main menu by \
pressing <c=magenta>q<c=/> or <c=magenta>Esc<c=/>.\n\
\n\
You can see the following information here:\n\
<c=brown>Location<c=/> is the name of the planet (if any) whose orbit you are \
in.\n\
<c=ltblue>Engine<c=/>   is the name of the engine your ship currently uses.\n\
<c=ltred>Fuel<c=/>     is how much you you have left (and your maximum fuel \
capacity).\n\
<c=ltgreen>Speed<c=/>    is how long it takes you to travel a single parsec.\n\
<c=cyan>Mass<c=/>     is the combined mass of your ship and its cargo, if \
any.\n\
<c=ltcyan>Limit<c=/>    is the maximum mass your engine is designed to \
handle.  Be careful!  You can go over this mass limit, but it will severely \
reduce your speed and fuel efficiency.\n\
<c=green>Manifest<c=/> is the list of cargo you're currently carrying.  This \
includes the amount you paid for this cargo; this helps you determine how much \
of a profit you will make when you sell it.  If you bought a certain type of \
good at two or more different prices, the Cost will be averaged.\n\
<c=ltblue>Equipment<c=/> is the list of parts that make up your ship.\n\
\n\
<c=green>Trader   Reputation<c=/> is your fame as a trader.\n\
  <c=yellow>Positive Implications:<c=/>\n\
    -You are more likely to receive special transport missions.\n\
    -You are more likely to make strong contacts in trade depots.\n\
  <c=ltred>Negative Implications:<c=/>\n\
    -You are more likely to be targeted by pirates.\n\
\n\
<c=ltblue>Combat  Reputation<c=/> is your reputation for being deadly in \
battle.\n\
  <c=yellow>Positive Implications:<c=/>\n\
    -You are less likely to be attacked.\n\
    -Enemies you attack are more likely to surrender.\n\
    -You are more likely to receive special takedown missions.\n\
  <c=ltred>Negative Implications:<c=/>\n\
    -If you are being pursued, your foe is more likely to bring a large \
fleet.\n\
    -Other spacefighters may seek you out, seeking fame as your killer.\n\
\n\
<c=ltred>Criminal Reputation<c=/> is your notoriety in the criminal world.\n\
  <c=yellow>Positive Implications:<c=/>\n\
    -Criminal elements such as the black market, hackers, and hit men will \
only\n\
     associate with you if you have some criminal reputation.\n\
    -You are more likely to receive special criminal missions.\n\
    -Pirates and others who are usually foes may instead seek to trade.\n\
  <c=ltred>Negative Implications:<c=/>\n\
    -Police and bounty hunters will hunt you down more and more persistantly.\n\
    -Your licenses, if any, will be revoked.\n\
    -You face the danger of capture every time you visit public locations \
such\n\
     as public offices or docks.\n\
\n\
You can press <c=magenta>Q<c=/> or <c=magenta>Esc<c=/> to return to the main \
menu.\n\
\n\
Press <c=magenta>Enter<c=/> or <c=magenta>Esc<c=/> to close this help.");
    } else if (ch == 'q' || ch == 'Q' || ch == KEY_ESC) {
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
      transport  += sell_amount[i] * GOOD_DATA[i]->mass;
      transport  += buy_amount[i]  * GOOD_DATA[i]->mass;
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

    if (ch == '?') {
      popup_scrollable("\
This is the trade depot, where you can buy or sell cargo.  On the left is a \
list of all available goods and their price; on the right is a list of all \
available goods, the amount you are carry, and their price.  You can press \
<c=magenta>TAB<c=/> to switch between the two lists.  Press \
<c=magenta>Up<c=/>/<c=magenta>Down<c=/> or <c=magenta>J<c=/>/<c=magenta>K<c=/> \
to scroll through the lists.\n\
You can press <c=magenta>Right<c=/>, <c=magenta>L<c=/> or <c=magenta>+=<c=/> \
to increase the amount you are buying/selling, and <c=magenta>Left<c=/>, \
<c=magenta>H<c=/> or <c=magenta>-<c=/> to decrease.  You can also press \
<c=magenta>M<c=/> to sell as many as your carrying / buy as many as you can \
afford and have storage space for.\n\
\n\
Both lists have a price column.  You'll notice that the sell price for each \
good is different from the buy price, and always lower.  Some goods have \
<c=dkgray>N/A<c=/> as their price; these cannot be bought or sold here at \
all.\n\
Prices in the <c=ltblue>Buying List<c=/> which are displayed in <c=dkgray>dark \
gray<c=/> cannot be sold for a higher price.  Thus, there is little point in \
buying these products as you'll always be selling at a loss.\n\
Prices which are displayed in <c=ltgreen>green<c=/> are the best prices on any \
planet.  In the <c=ltblue>Buying List<c=/> this means the price is the lowest \
currently available; in the <c=ltblue>Selling List<c=/> this means the price \
is the highest currently available.\n\
\n\
At the bottom of the screen you can see your <c=ltgreen>Earnings<c=/>, \
<c=ltred>Cost<c=/>, and <c=ltred>Transport Costs<c=/>.  In addition to the \
price of goods bought, you will have to pay to have goods transported to and \
from the surface of the planet.  This price is affected by the mass of the \
good being bought or sold, and the <c=brown>gravity<c=/> of the planet.\n\
\n\
At the bottom center, you can see the <c=cyan>Prices<c=/> of the currently \
selected good.  This includes the local price, the lowest price you can buy \
it at on any planet, and the highest price you can sell it at on any planet.\n\
\n\
At the bottom right, You can see your <c=ltgreen>Current Cash<c=/> and \
available <c=ltblue>Cargo Space<c=/>.  You'll also see the cash/cargo space \
you'll have after the trade goes through.  If either is negative, then you \
won't be able to make this purchase!\n\
\n\
You can press <c=magenta>Q<c=/> or <c=magenta>Esc<c=/> to return to the main \
menu.\n\
\n\
Press <c=magenta>Enter<c=/> or <c=magenta>Esc<c=/> to close this help.");

    } else if (ch == 'm' || ch == 'M') {
      if (buying) {
        buy_amount[sel] = max_buyable;
      } else {
        sell_amount[sel] = PLR.cargo.amount[sel];
      }
    } else if (ch == '+' || ch == KEY_RIGHT || ch == 'l' || ch == 'L') {
      if (buying) {
        if (market->buy_price( Good_id(sel) ) > 0) {
          buy_amount[sel]++;
        }
      } else if (PLR.cargo.amount[sel] > sell_amount[sel] &&
                 market->sell_price( Good_id(sel) ) > 0) {
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
  i_dock.ref_data("num_plr_cash_2", &(PLR.cash));

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

  int num_buying[NUM_SP];
  for (int i = 0; i < NUM_SP; i++) {
    num_buying[i] = 0;
  }
  int parts_price = 0;
  i_dock.ref_data("num_parts_price", &parts_price);

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

// Populate number of parts buying list
    i_dock.clear_data("list_part_amount");
    for (int i = 1; i < NUM_SP; i++) {
      std::stringstream partnum_text;
      partnum_text << num_buying[i];
      i_dock.add_data("list_part_amount", partnum_text.str() );
    }

    i_dock.draw(&w_dock);

    long ch = getch();

    if (ch == '?') {
      popup_scrollable("\
This is the dock screen.  This is where you can maintain your starship.\n\
On the left you can see your <c=green>Fuel gauge<c=/>.  Fuel is required to \
travel from one planet to another, so you will need to visit the dock \
frequently to fill up!  This section contains a visual gauge, hard numbers on \
your remaining fuel & fuel capactiy, the type of fuel your current engine \
uses, and the price per unit of fuel.  This price is affected by the related \
good's price on the planet you're visiting, so it may be wise to travel to a \
planet where fuel is cheap in order to fill up!\n\
\n\
Press <c=magenta>F<c=/> to fill your fuel tanks to capacity.  If you cannot \
afford to do so, or do not want to, you can press \
<c=magenta>+<c=/>/<c=magenta>-<c=/> to increase/decrease the amount of fuel \
you want to buy, then press <c=magenta>B<c=/> to buy.\n\
\n\
In the center of the screen, your damaged parts are listed.  Parts which are \
in perfect condition will not be included.  Press \
<c=magenta>Up<c=/>/<c=magenta>Down<c=/> or <c=magenta>J<c=/>/<c=magenta>K<c=/> \
to scroll through the list, then <c=magenta>Enter<c=/> to repair the part. \
Alternatively, you can press <c=magenta>R<c=/> to repair ALL damaged parts.\n\
\n\
On the right side of the screen is the parts shop, where you can buy or sell \
parts for your spaceship.  Press <c=magenta>TAB<c=/> to switch between the \
Repairs list and the Parts list.  Press <c=magenta>/<c=/> to switch between \
buying parts and selling parts.\n\
When <c=ltblue>buying<c=/> parts, use <c=magenta>Up<c=/>/<c=magenta>Down<c=/> \
or <c=magenta>J<c=/>/<c=magenta>K<c=/> to scroll through the list, then \
<c=magenta>Left<c=/>/<c=magenta>Right<c=/>, \
<c=magenta>-<c=/>/<c=magenta>+=<c=/> or <c=magenta>H<c=/>/<c=magenta>L<c=/> to \
increase & decrease the number of units you are buying.  Below the list of \
available parts, you can see your current cash, the total price of the parts, \
and your cash after the purchase.  Obviously, if your cash does not cover the \
total price, you may not make a purchase.  Once you're ready to buy, press \
<c=magenta>Enter<c=/> to buy the selected parts.\n\
When <c=ltblue>selling<c=/> parts, use <c=magenta>Up<c=/>/<c=magenta>Down<c=/> \
or <c=magenta>J<c=/>/<c=magenta>K<c=/> to scroll through the list, then press \
<c=magenta>Enter<c=/> to sell the currently-selected part.\n\
<c=ltred>Warning:<c=/> if you sell your engine, you will not be able to travel \
anywhere until you buy a new one!\n\
\n\
You can press <c=magenta>Q<c=/> or <c=magenta>Esc<c=/> to return to the main \
menu.\n\
\n\
Press <c=magenta>Enter<c=/> or <c=magenta>Esc<c=/> to close this help.");

    } else if (ch == 'f' || ch == 'F') { // Fill up
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
    } else if ((ch == 'l' || ch == 'L' || ch == '+' || ch == '=' ||
                ch == KEY_RIGHT) &&
               buying_parts && i_dock.selected()->name == "list_parts_to_buy") {
// We add one since the 0th entry in the interface list is really the 1st
// part in the database.
      int buy_index = i_dock.get_int("list_parts_to_buy") + 1;
      num_buying[buy_index]++;
      parts_price += PARTS[buy_index]->cost;
      i_dock.set_data("num_cash_after_purchase", PLR.cash - parts_price);
    } else if ((ch == 'h' || ch == 'H' || ch == '-' || ch == KEY_LEFT) &&
               buying_parts && i_dock.selected()->name == "list_parts_to_buy") {
// We add one since the 0th entry in the interface list is really the 1st
// part in the database.
      int buy_index = i_dock.get_int("list_parts_to_buy") + 1;
      if (num_buying[buy_index] > 0) {
        num_buying[buy_index]--;
        parts_price -= PARTS[buy_index]->cost;
        i_dock.set_data("num_cash_after_purchase", PLR.cash - parts_price);
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
          if (PLR.cash < parts_price) {
            popup("Can't afford that!  You: $%d, price: $%d", PLR.cash,
                  parts_price);
          } else {
            PLR.cash -= parts_price;
            for (int i = 1; i < NUM_SP; i++) {
              for (int n = 0; n < num_buying[i]; n++) {
                PLR.add_part( PARTS[i] );
              }
              num_buying[i] = 0;
            }
            parts_price = 0;
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
          if (sellp == highest_price[n]) {
            data << "<c=green>";
          } else if (!destination_market) {
            data << "<c=ltblue>";
          }
          destination_market = true;
        } else if (!destination_market && buyp == lowest_price[n]) {
          destination_market = true;
          data << "<c=ltcyan>";
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
    if (ch == '?') {
      popup_scrollable("\
This is the <c=magenta>Starmap<c=/> screen.  This screen allows you to view \
information about all planets, and to travel to them.\n\
\n\
On the left side is a list of planets.  You can scroll through this list by \
pressing <c=magenta>Down<c=/>/<c=magenta>Up<c=/> or \
<c=magenta>J<c=/>/<c=magenta>K<c=/>.  Each planet is listed along with its \
distance from your current position.  The planet names are also color-coded:\n\
<c=green>Green:<c=/> This planet has the highest sell price for one or more of \
the items in your cargo.\n\
<c=ltblue>Blue:<c=/>  You can sell one or more of the items in your cargo at a \
profit at this planet.\n\
<c=ltcyan>Cyan:<c=/>  This planet has the lowest buy price for one or more \
goods.\n\
<c=ltgray>Gray:<c=/>  This planet does not fit into any of the above.\n\
\n\
By default, a starmap will appear in the center of the screen, showing the \
positions of various planets.  They will be color-coded by biome (see below) \
and size coded by gravity; if gravity <= 1g, the planet will be represented by \
<c=brown>o<c=/>, and if gravity > 1g, the planet will be represented by \
<c=brown>O<c=/>.  The planet you are currently orbiting will always be \
represented by <c=brown>@<c=/>.\n\
\n\
You can switch to a display of prices on the currently-selected planet by \
pressing <c=magenta>/<c=/>.  This shows the buy & sell prices of each good.  \
The price will be <c=dkgray>dark gray<c=/> if the product cannot be \
bought/sold at all, or if the buy price is higher than all the sell prices \
(making it impossible to profit on goods bought there).  The price will be \
<c=ltgreen>green<c=/> if it is the best price available.\n\
\n\
At the top-right of the screen, you will see the <c=yellow>Travel Time<c=/> \
and <c=yellow>Travel Cost<c=/> (in fuel units) for traveling to the \
currently-selected planet.  You'll also see your <c=yellow>Current Fuel<c=/>.\n\
\n\
At the top-center of the screen, you will see a few pieces of information \
about the currently-selected statement.  This information can affect the \
supply and demand of various goods.\n\
<c=green>Biome<c=/> is information about the natural state of the world. \
Possible biomes include:\n\
<c=green>Grassy<c=/> - A planet dominated by fertile fields.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Fertilizer            <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltgreen>++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
 <c=red>Demand effects:<c=/>\n\
  Heavy Machinery       <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Solid Fuel            <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltred>-<c=/>\n\
  Prepared Food         <c=ltred>-<c=/>\n\
<c=green>Desert<c=/> - A planet covered in dry sandy deserts.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Raw Fuel              <c=ltgreen>++<c=/>\n\
  Uranium               <c=ltgreen>+<c=/>\n\
  Ore                   <c=ltred>-<c=/>\n\
  Water                 <c=red>----<c=/>\n\
  Fertilizer            <c=ltred>--<c=/>\n\
 <c=red>Demand effects:<c=/>\n\
  Water                 <c=green>++++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
<c=green>Water<c=/> - A planet almost entirely covered in ocean.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Water                 <c=ltgreen>+++<c=/>\n\
  Fertilizer            <c=ltgreen>++<c=/>\n\
  Crops                 <c=ltgreen>+<c=/>\n\
  Raw Fuel              <c=ltgreen>+<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
  Ore                   <c=ltred>----<c=/>\n\
 <c=red>Demand effects:<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
  Uranium               <c=ltgreen>+<c=/>\n\
  Water                 <c=red>None!<c=/>\n\
  Fertilizer            <c=red>-<c=/>\n\
<c=green>Swamp<c=/> - A planet dominated by murky swamps.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Fertilizer            <c=green>++++<c=/>\n\
  Raw Fuel              <c=ltgreen>++<c=/>\n\
  Ore                   <c=red>--<c=/>\n\
  Crops                 <c=red>-<c=/>\n\
 <c=red>Demand effects:<c=/>\n\
  Metals                <c=ltgreen>+<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
<c=green>Ice<c=/> - A frozen planet.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=red>---<c=/>\n\
  Crops                 <c=red>---<c=/>\n\
 <c=red>Demand effects:<c=/>\n\
  Consumer Electronics  <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+++<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
  Solid Fuel            <c=ltgreen>++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
  Uranium               <c=ltgreen>+<c=/>\n\
<c=green>Barren<c=/> - A rocky planet with little natural life.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=ltgreen>+++<c=/>\n\
  Water                 <c=red>---<c=/>\n\
  Fertilizer            <c=red>-<c=/>\n\
  Crops                 <c=red>---<c=/>\n\
  Raw Fuel              <c=red>-<c=/>\n\
 <c=red>Demand effects:<c=/>\n\
  Metals                <c=ltgreen>+<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=red>-<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>++<c=/>\n\
  Uranium               <c=ltgreen>++<c=/>\n\
<c=green>Varied<c=/> - A planet with a wide variety of biomes.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltgreen>+<c=/>\n\
  Raw Fuel              <c=ltgreen>+<c=/>\n\
\n\
<c=ltblue>Population<c=/> is information about who lives on the planet, and \
how many of them there are.  Possible populations include:\n\
<c=ltblue>Sparse<c=/> - The planet is barely settled.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Crops                 <c=ltgreen>+<c=/>\n\
  Raw Fuel              <c=ltgreen>+<c=/>\n\
  Metals                <c=ltred>-<c=/>\n\
  Elec. Components      <c=ltred>--<c=/>\n\
  Consumer Electronics  <c=ltred>--<c=/>\n\
  Flour & Dried Foods   <c=ltred>-<c=/>\n\
  Prepared Food         <c=ltred>--<c=/>\n\
  Solid Fuel            <c=ltred>--<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Ore                   <c=ltred>--<c=/>\n\
  Metals                <c=ltgreen>++<c=/>\n\
  Consumer Electronics  <c=red>----<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=ltgreen>+<c=/>\n\
  Flour & Dried Foods   <c=red>----<c=/>\n\
  Prepared Food         <c=red>----<c=/>\n\
<c=ltblue>Rural<c=/> - The planet lacks major cities.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Crops                 <c=ltgreen>++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Elec. Components      <c=ltred>-<c=/>\n\
  Consumer Electronics  <c=ltred>-<c=/>\n\
  Solid Fuel            <c=ltred>-<c=/>\n\
<c=ltblue>Settled<c=/> - The planet is a mixture of cities and open country.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Metals                <c=ltgreen>+<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Prepared Food         <c=ltgreen>+<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Consumer Electronics  <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Prepared Food         <c=ltgreen>+<c=/>\n\
  Solid Fuel            <c=ltgreen>+<c=/>\n\
<c=ltblue>Urban<c=/> - The vast majority of the population lives in cities.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=ltred>-<c=/>\n\
  Water                 <c=ltred>-<c=/>\n\
  Fertilizer            <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltred>--<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Ore                   <c=ltred>-<c=/>\n\
  Consumer Electronics  <c=ltgreen>++<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
  Solid Fuel            <c=ltgreen>++<c=/>\n\
<c=ltblue>Megacity<c=/> - Nearly the entire surface of the world is one city.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=ltred>--<c=/>\n\
  Water                 <c=ltred>---<c=/>\n\
  Fertilizer            <c=ltgreen>+++<c=/>\n\
  Crops                 <c=red>----<c=/>\n\
  Flour & Dried Foods   <c=ltred>-<c=/>\n\
  Raw Fuel              <c=ltred>-<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Elec. Components      <c=ltgreen>++<c=/>\n\
  Consumer Electronics  <c=green>++++<c=/>\n\
  Machinery             <c=ltgreen>++<c=/>\n\
  Water                 <c=ltgreen>+++<c=/>\n\
  Fertilizer            <c=ltred>--<c=/>\n\
  Crops                 <c=ltred>-<c=/>\n\
  Prepared Food         <c=ltgreen>+++<c=/>\n\
  Solid Fuel            <c=ltgreen>++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>++<c=/>\n\
  Uranium               <c=ltgreen>++<c=/>\n\
<c=ltblue>Robotic<c=/> - The majority of the population is robotic.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=ltgreen>+<c=/>\n\
  Metals                <c=ltgreen>+<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=ltred>-<c=/>\n\
  Prepared Food         <c=ltred>-<c=/>\n\
  Raw Fuel              <c=ltgreen>+<c=/>\n\
  Uranium               <c=ltgreen>+<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Elec. Components      <c=ltgreen>++<c=/>\n\
  Consumer Electronics  <c=ltred>None!<c=/>\n\
  Machinery             <c=ltred>--<c=/>\n\
  Flour & Dried Foods   <c=red>-----<c=/>\n\
  Prepared Food         <c=ltred>None!<c=/>\n\
  Solid Fuel            <c=ltgreen>+<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+++<c=/>\n\
  Uranium               <c=green>++++<c=/>\n\
<c=ltred>Industry<c=/> is the dominant form of industry on the planet.  While \
other industries may exist, this one accounts for the majority of the work.  \
Possible industries include:\n\
<c=ltred>Living space<c=/> - Most denizens commute to other planets for work.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=ltred>--<c=/>\n\
  Metals                <c=ltred>--<c=/>\n\
  Elec. Components      <c=ltred>-<c=/>\n\
  Consumer Electronics  <c=ltred>-<c=/>\n\
  Machinery             <c=ltred>--<c=/>\n\
  Fertilizer            <c=ltgreen>++<c=/>\n\
  Prepared Food         <c=ltgreen>+<c=/>\n\
  Raw Fuel              <c=ltred>---<c=/>\n\
  Solid Fuel            <c=ltred>--<c=/>\n\
  Uranium               <c=ltred>-<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Ore                   <c=ltred>---<c=/>\n\
  Metals                <c=ltgreen>+<c=/>\n\
  Consumer Electronics  <c=green>++++<c=/>\n\
  Water                 <c=ltgreen>+++<c=/>\n\
  Crops                 <c=ltgreen>+<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+++<c=/>\n\
  Prepared Food         <c=ltgreen>+++<c=/>\n\
  Raw Fuel              <c=ltred>--<c=/>\n\
<c=ltred>Mining<c=/> - Extraction of valuable assets from the planet.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=green>++++<c=/>\n\
  Metals                <c=ltgreen>+<c=/>\n\
  Water                 <c=ltred>--<c=/>\n\
  Crops                 <c=red>----<c=/>\n\
  Raw Fuel              <c=green>+++++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
  Uranium               <c=ltgreen>+++<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Metals                <c=ltgreen>++<c=/>\n\
  Elec. Components      <c=ltgreen>++<c=/>\n\
  Consumer Electronics  <c=ltred>--<c=/>\n\
  Machinery             <c=green>++++<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=ltred>---<c=/>\n\
  Crops                 <c=ltred>---<c=/>\n\
  Flour & Dried Foods   <c=ltred>--<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
  Solid Fuel            <c=ltgreen>++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
<c=ltred>Heavy Manufacturing<c=/> - Production of large goods.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Metals                <c=green>++++<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Consumer Electronics  <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>+++<c=/>\n\
  Crops                 <c=ltred>---<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Ore                   <c=ltgreen>+++<c=/>\n\
  Metals                <c=ltgreen>++<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Consumer Electronics  <c=ltred>--<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=red>----<c=/>\n\
  Crops                 <c=ltred>--<c=/>\n\
  Flour & Dried Foods   <c=ltred>-<c=/>\n\
  Prepared Food         <c=ltgreen>+<c=/>\n\
  Raw Fuel              <c=ltgreen>+<c=/>\n\
  Solid Fuel            <c=ltgreen>+++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
<c=ltred>Refining<c=/> - Refinement of raw goods, especially fuel.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Metals                <c=ltgreen>++<c=/>\n\
  Elec. Components      <c=ltred>-<c=/>\n\
  Consumer Electronics  <c=ltred>--<c=/>\n\
  Machinery             <c=ltred>-<c=/>\n\
  Water                 <c=ltred>---<c=/>\n\
  Fertilizer            <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltred>---<c=/>\n\
  Solid Fuel            <c=green>+++++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>++<c=/>\n\
  Uranium               <c=ltgreen>+<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Ore                   <c=ltgreen>+<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>+++<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=red>----<c=/>\n\
  Crops                 <c=ltred>---<c=/>\n\
  Flour & Dried Foods   <c=ltred>-<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
  Raw Fuel              <c=green>+++++<c=/>\n\
  Solid Fuel            <c=ltgreen>+<c=/>\n\
<c=ltred>Light Manufacturing<c=/> - Production of small goods.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Elec. Components      <c=ltgreen>+++<c=/>\n\
  Consumer Electronics  <c=ltgreen>+++<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltred>-<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Metals                <c=ltgreen>++<c=/>\n\
  Elec. Components      <c=ltgreen>+++<c=/>\n\
  Machinery             <c=ltgreen>++<c=/>\n\
  Fertilizer            <c=ltred>-<c=/>\n\
  Crops                 <c=ltgreen>++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>++<c=/>\n\
  Prepared Food         <c=ltgreen>+<c=/>\n\
  Solid Fuel            <c=ltgreen>+<c=/>\n\
<c=ltred>Farming<c=/> - Farming, either traditional or hydroponic.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Metals                <c=ltred>-<c=/>\n\
  Water                 <c=ltred>-<c=/>\n\
  Fertilizer            <c=ltgreen>++<c=/>\n\
  Crops                 <c=green>++++++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>++<c=/>\n\
  Prepared Food         <c=ltgreen>+<c=/>\n\
  Raw Fuel              <c=ltred>---<c=/>\n\
  Solid Fuel            <c=ltred>--<c=/>\n\
  Uranium               <c=ltred>--<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Machinery             <c=ltgreen>++<c=/>\n\
  Water                 <c=ltgreen>++<c=/>\n\
  Fertilizer            <c=green>+++++<c=/>\n\
  Crops                 <c=ltred>-<c=/>\n\
  Solid Fuel            <c=ltgreen>+++<c=/>\n\
<c=ltred>Food Processing<c=/> - Production of food products from raw food.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Fertilizer            <c=ltgreen>++<c=/>\n\
  Flour & Dried Foods   <c=green>++++<c=/>\n\
  Prepared Food         <c=green>++++<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Elec. Components      <c=ltgreen>++<c=/>\n\
  Consumer Electronics  <c=ltgreen>+<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>++<c=/>\n\
  Crops                 <c=green>++++<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
<c=ltred>Business<c=/> - This planet is home to various corporate HQs.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Ore                   <c=ltred>-<c=/>\n\
  Metals                <c=ltred>-<c=/>\n\
  Consumer Electronics  <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltred>-<c=/>\n\
  Raw Fuel              <c=ltred>-<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Elec. Components      <c=ltgreen>+<c=/>\n\
  Consumer Electronics  <c=green>++++<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Fertilizer            <c=ltred>--<c=/>\n\
  Crops                 <c=ltgreen>+<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Prepared Food         <c=ltgreen>++<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+<c=/>\n\
<c=ltred>Research<c=/> - This planet is interesting to researchers.\n\
 <c=yellow>Supply effects:<c=/>\n\
  Elec. Components      <c=ltgreen>++<c=/>\n\
  Consumer Electronics  <c=ltred>-<c=/>\n\
  Fertilizer            <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltgreen>+<c=/>\n\
  Flour & Dried Foods   <c=ltgreen>+<c=/>\n\
  Raw Fuel              <c=ltgreen>+<c=/>\n\
  Hydrogen Fuel         <c=ltgreen>+++<c=/>\n\
 <c=ltred>Demand Effects:<c=/>\n\
  Ore                   <c=ltgreen>+<c=/>\n\
  Elec. Components      <c=ltgreen>++<c=/>\n\
  Machinery             <c=ltgreen>+<c=/>\n\
  Water                 <c=ltgreen>+<c=/>\n\
  Crops                 <c=ltgreen>++<c=/>\n\
  Raw Fuel              <c=ltgreen>++<c=/>\n\
  Uranium               <c=green>++++<c=/>\n\
\n\
\n\
You can press <c=magenta>Q<c=/> or <c=magenta>Esc<c=/> to return to the main \
menu.\n\
\n\
Press <c=magenta>Enter<c=/> or <c=magenta>Esc<c=/> to close this help.");
    } else if (ch == '\n') {
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
