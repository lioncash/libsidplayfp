/*
 *  Copyright (C) 2010-2015 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef INIPARSER_H
#define INIPARSER_H

#include <map>
#include <string>
#include <string_view>
#include <utility>

namespace libsidplayfp
{

class iniParser
{
public:
    bool open(const char* fName);
    void close();

    bool setSection(std::string_view section);
    const char* getValue(std::string_view key);

private:
    using keys_t = std::map<std::string, std::string, std::less<>>;
    using sections_t = std::map<std::string, keys_t, std::less<>>;

    std::string parseSection(const std::string& buffer);

    keys_t::value_type parseKey(const std::string& buffer);

    sections_t sections;
    sections_t::const_iterator curSection;
};

}

#endif // INIPARSER_H
