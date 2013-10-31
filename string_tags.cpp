#include "string_tags.h"
#include "rng.h"
#include "globals.h"
#include "window.h"

std::string process_string_tags(const std::string &text,
                                const std::string &planet_name)
{
  std::string ret;
  size_t tag = 0, lasttag = 0;
  while ( (tag = text.find("<", tag)) != std::string::npos) {
    size_t tagend = text.find(">", tag);
    if (tagend == std::string::npos) {
      debugmsg("Unterminated text flag: '%s'", text.c_str());
      return ret;
    }
    ret += text.substr(lasttag, tag - lasttag);

    std::string tagstr = text.substr(tag + 1, tagend - tag - 1);
    ret += get_tagstr( tagstr, planet_name );

    tag     = tagend;
    lasttag = tagend + 1;
  }

  if (lasttag < text.size()) {
    ret += text.substr(lasttag);
  }

  return ret;
}

std::string get_tagstr(const std::string &tag, const std::string &planet_name)
{
  std::vector<std::string> *pregenerated = NULL;
  std::string (*gen_func)() = NULL;
  if (tag == "planet") {
    return planet_name;
  } else if (tag == "company:electronic" ) {
    pregenerated = &COMPANIES_ELECTRONIC;
    gen_func = &generate_company_name;
  } else if (tag == "company:food" ) {
    pregenerated = &COMPANIES_FOOD;
    gen_func = &generate_company_food_name;
  } else if (tag == "acronym") {
    pregenerated = &ACRONYMS;
    gen_func = &generate_acronym;
  } else if (tag == "product:consumer") {
    pregenerated = &PRODUCTS_CONSUMER;
    gen_func = &generate_product_consumer;
  } else if (tag == "product:edibles") {
    pregenerated = &PRODUCTS_EDIBLES;
    gen_func = &generate_product_edibles;
  } else {
    debugmsg("Unknown tag <%s>", tag.c_str());
    return "huh";
  }
  int index = rng(0, 9);
  if (index < pregenerated->size()) {
    return (*pregenerated)[index];
  }
  std::string ret = gen_func();
  pregenerated->push_back(ret);
  return ret;
}

std::string generate_company_name()
{
  std::string ret;
  switch ( rng(1, 10) ) {
    case  1:  ret = "Globo";      break;
    case  2:  ret = "Infini";     break;
    case  3:  ret = "Infra";      break;
    case  4:  ret = "Omni";       break;
    case  5:  ret = "Para";       break;
    case  6:  ret = "Visi";       break;
    case  7:  ret = "Proto";      break;
    case  8:  ret = "Nitro";      break;
    case  9:  ret = "Data";       break;
    case 10:  ret = "Lumi";       break;
  }

  switch ( rng(1, 10) ) {
    case  1:  ret += "tech";      break;
    case  2:  ret += "corp";      break;
    case  3:  ret += "prog";      break;
    case  4:  ret += "net";       break;
    case  5:  ret += "zine";      break;
    case  6:  ret += "tron";      break;
    case  7:  ret += "dyne";      break;
    case  8:  ret += "kline";     break;
    case  9:  ret += "max";       break;
    case 10:  ret += "tac";       break;
  }

  switch ( rng(1, 10) ) {
    case  1:  ret += " Systems";  break;
    case  2:  ret += " LLC";      break;
    case  3:  ret += ", Inc";     break;
    case  4:  ret += " Ltd";      break;
    default: /* Do Nothing */     break;
  }

  return ret;
}

std::string generate_company_food_name()
{
  std::string ret;

  switch (rng(1, 10)) {
    case  1:  ret = "Sweet";    break;
    case  2:  ret = "Bak";      break;
    case  3:  ret = "Tast";     break;
    case  4:  ret = "Nutr";     break;
    case  5:  ret = "Food";     break;
    case  6:  ret = "Drink";    break;
    case  7:  ret = "Yumm";     break;
    case  8:  ret = "Pept";     break;
    case  9:  ret = "Past";     break;
    case 10:  ret = "Nibbl";    break;
  }

  switch (rng(1, 10)) {
    case  1:  ret += "ums";     break;
    case  2:  ret += "ola";     break;
    case  3:  ret += "astic";   break;
    case  4:  ret += "amatic";  break;
    case  5:  ret += "asmic";   break;
    case  6:  ret += "a-King";  break;
    case  7:  ret += "ies";     break;
    case  8:  ret += "ize";     break;
    case  9:  ret += "atron";   break;
    case 10:  ret += "ico";     break;
  }

  switch (rng(1, 10)) {
    case  1:  ret += " Foods";  break;
    case  2:  ret = "General " + ret; break;
    case  3:  ret += " Quality"; break;
    case  4:  ret += " Nutrition"; break;
    default: /* Do Nothing */   break;
  }

  return ret;
}
    
std::string generate_acronym()
{
  std::string ret;
  ret = 'A' + rng(0, 25); // Start with any random letter

  int length = rng(3, 6);

  while (ret.length() < length) {
    char last = ret[ret.size() - 1];
// We skip U cause we want U to have a vowel after it.
    if (last == 'A' || last == 'E' || last == 'I' || last == 'O') {
      switch (rng(1, 28)) {
        case  1:  ret += 'B'; break;
        case  2:  ret += 'C'; break;
        case  3:  ret += 'D'; break;
        case  4:  ret += 'F'; break;
        case  5:  ret += 'G'; break;
        case  6:  ret += 'K'; break;
        case  7:  ret += 'L'; break;
        case  8:  ret += 'M'; break;
        case  9:  ret += 'N'; break;
        case 10:  ret += 'P'; break;
        case 11:  ret += 'Q'; break;
        case 12:  ret += 'R'; break;
        case 13:  ret += 'S'; break;
        case 14:  ret += 'T'; break;
        case 15:  ret += 'V'; break;
        case 16:  ret += 'X'; break;
        case 17:  ret += 'Z'; break;
        case 18:  ret += "BB";  break;
        case 19:  ret += "DD";  break;
        case 20:  ret += "GG";  break;
        case 21:  ret += "LL";  break;
        case 22:  ret += "MM";  break;
        case 23:  ret += "NN";  break;
        case 24:  ret += "PP";  break;
        case 25:  ret += "RR";  break;
        case 26:  ret += "SS";  break;
        case 27:  ret += "TT";  break;
        case 28:  ret += 'W'; break;
      }
    } else { // Not a vowel
      if (last == 'Q') {
        ret += 'U';
      } else if ((last == 'B' || last == 'C' || last == 'D' || last == 'F' ||
                  last == 'G' || last == 'P' || last == 'T' || last == 'W'  ) &&
                 one_in(4)) {
        ret += 'R';
      } else if ((last == 'C' || last == 'P' || last == 'S' || last == 'T') &&
                 one_in(4)) {
        ret += 'H';
      } else if ((last == 'C' || last == 'F' || last == 'G' || last == 'K' ||
                  last == 'P' || last == 'S') && one_in(4)) {
        ret += 'L';
      } else if (last == 'S' && one_in(3)) {
        switch (rng(1, 9)) {
          case  1:  ret += 'C'; break;
          case  2:  ret += 'K'; break;
          case  3:  ret += 'L'; break;
          case  4:  ret += 'M'; break;
          case  5:  ret += 'N'; break;
          case  6:  ret += 'P'; break;
          case  7:  ret += 'Q'; break;
          case  8:  ret += 'T'; break;
          case  9:  ret += 'W'; break;
        }
      } else {
        switch (rng(1, 5)) {
          case  1:  ret += 'A'; break;
          case  2:  ret += 'E'; break;
          case  3:  ret += 'I'; break;
          case  4:  ret += 'O'; break;
          case  5:  ret += 'U'; break;
        }
      }
    }
  }

  return ret;
}

std::string generate_product_consumer()
{
  std::string ret;

  switch (rng(1, 10)) {
    case  1:  ret = "Laser";      break;
    case  2:  ret = "Mini";       break;
    case  3:  ret = "Intelli";    break;
    case  4:  ret = "Quanta";     break;
    case  5:  ret = "Posi";       break;
    case  6:  ret = "Mega";       break;
    case  7:  ret = "Neo";        break;
    case  8:  ret = "Power";      break;
    case  9:  ret = "Hyper";      break;
    case 10:  ret = "Trini";      break;
  }

  switch (rng(1, 10)) {
    case  1:  ret += "book";      break;
    case  2:  ret += "pad";       break;
    case  3:  ret += "cube";      break;
    case  4:  ret += "prism";     break;
    case  5:  ret += "zapper";    break;
    case  6:  ret += "matic";     break;
    case  7:  ret += "sphere";    break;
    case  8:  ret += "pet";       break;
    case  9:  ret += "vac";       break;
    case 10:  ret += "drone";     break;
  }

  switch (rng(1, 10)) {
    case  1:  ret += " Pro";      break;
    case  2:  ret += " II";       break;
    case  3:  ret += " Plus";     break;
    case  4:
      ret += 'X';
      switch (rng(1, 8)) {
        case  1:  ret += 'D';   break;
        case  2:  ret += 'L';   break;
        case  3:  ret += 'M';   break;
        case  4:  ret += 'P';   break;
        case  5:  ret += 'R';   break;
        case  6:  ret += 'T';   break;
        case  7:  ret += 'Y';   break;
        case  8:
          ret += 'X';
          while (one_in(10)) {
            ret += 'X';
          }
          break;
      }
      break;
    default: /* Do Nothing */     break;
  }

  return ret;
}

std::string generate_product_edibles()
{
  std::string ret, noun;

  switch (rng(1, 10)) {
    case  1:  noun = "Snack";     break;
    case  2:  noun = "Lunch";     break;
    case  3:  noun = "Sweet";     break;
    case  4:  noun = "Chomp";     break;
    case  5:  noun = "Bite";      break;
    case  6:  noun = "Slurp";     break;
    case  7:  noun = "Gulp";      break;
    case  8:  noun = "Taste";     break;
    case  9:  noun = "Flavor";    break;
    case 10:  noun = "Yum";       break;
  }

  switch (rng(1, 10)) {
    case  1:
      ret = noun + "-o-Matic";
      break;
    case  2:
      noun[0] = noun[0] + 'a' - 'A';
      ret = "Mega" + noun;
      break;
    case  3:
      noun[0] = noun[0] + 'a' - 'A';
      ret = "Infini" + noun;
      break;
    case  4:
      ret = "Mr. " + noun;
      break;
    case  5:
      ret = noun + "tastic";
      break;
    case  6:
      ret = noun + "sters";
      break;
    case  7:
      ret = noun + " Shake";
      break;
    case  8:
      noun[0] = noun[0] + 'a' - 'A';
      ret = "Proto" + noun;
      break;
    case  9:
      noun[0] = noun[0] + 'a' - 'A';
      ret = "Econo" + noun;
      break;
    case 10:
      noun[0] = noun[0] + 'a' - 'A';
      ret = "Nutri" + noun;
      break;
  }

  return ret;
}
