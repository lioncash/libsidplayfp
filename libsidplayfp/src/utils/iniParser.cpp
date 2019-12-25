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

#include "utils/iniParser.h"

#include <fstream>

namespace libsidplayfp
{

class parseError {};

std::string iniParser::parseSection(std::string_view buffer)
{
    const std::size_t pos = buffer.find(']');

    if (pos == std::string::npos)
    {
        throw parseError();
    }

    return std::string{buffer.substr(1, pos-1)};
}

iniParser::keys_t::value_type iniParser::parseKey(std::string_view buffer)
{
    const std::size_t pos = buffer.find('=');

    if (pos == std::string::npos)
    {
        throw parseError();
    }

    std::string key{buffer.substr(0, buffer.find_last_not_of(' ', pos-1) + 1)};
    std::string value{buffer.substr(pos + 1)};
    return std::make_pair(std::move(key), std::move(value));
}

bool iniParser::open(const char *fName)
{
    std::ifstream iniFile(fName);

    if (iniFile.fail())
    {
        return false;
    }

    sections_t::iterator mIt;

    while (iniFile.good())
    {
        std::string buffer;
        std::getline(iniFile, buffer);

        if (buffer.empty())
            continue;

        switch (buffer.front())
        {
        case ';':
        case '#':
            // skip comments
            break;
        case '[':
            try
            {
                std::string section = parseSection(buffer);
                keys_t keys;
                const auto it = sections.emplace(std::move(section), std::move(keys));
                mIt = it.first;
            }
            catch (const parseError&)
            {
            }
            break;
        default:
            try
            {
                (*mIt).second.insert(parseKey(buffer));
            }
            catch (const parseError&)
            {
            }
            break;
        }
    }

    return true;
}

void iniParser::close()
{
    sections.clear();
}

bool iniParser::setSection(std::string_view section)
{
    curSection = sections.find(section);
    return curSection != sections.end();
}

const char *iniParser::getValue(std::string_view key) const
{
    const auto keyIt = (*curSection).second.find(key);
    return (keyIt != (*curSection).second.end()) ? keyIt->second.c_str() : nullptr;
}

}
