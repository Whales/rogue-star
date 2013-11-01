#ifndef _STRING_TAGS_H_
#define _STRING_TAGS_H_

#include <string>
#include "color.h"

std::string process_string_tags(const std::string &text,
                                const std::string &planet_name,
                                nc_color color);

std::string get_tagcolor(const std::string &tag);
std::string get_tagstr(const std::string &tag, const std::string &planet_name);

std::string generate_company_name();
std::string generate_company_food_name();
std::string generate_acronym();
std::string generate_product_consumer();
std::string generate_product_edibles();

#endif
