/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright 2013-2014 Leandro Nini
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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <algorithm>
#include <cstddef>
#include <cctype>
#include <string_view>

namespace stringutils
{
    /**
     * Compare two characters in a case insensitive way.
     */
    inline bool casecompare(char c1, char c2) { return std::tolower(c1) == std::tolower(c2); }

    /**
     * Compare two strings in a case insensitive way. 
     *
     * @return true if strings are equal.
     */
    inline bool equal(std::string_view s1, std::string_view s2)
    {
        return s1.size() == s2.size() && std::equal(s1.cbegin(), s1.cend(), s2.cbegin(), casecompare);
    }

    /**
     * Compare first n characters of two strings in a case insensitive way.
     *
     * @return true if strings are equal.
     */
    inline bool equal(std::string_view s1, std::string_view s2, std::size_t n)
    {
        return std::equal(s1.cbegin(), s1.cbegin() + n, s2.cbegin(), casecompare);
    }
}

#endif
