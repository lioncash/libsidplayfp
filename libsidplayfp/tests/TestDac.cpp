/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright (C) 2014-2016 Leandro Nini
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

#include <catch.hpp>

#include <array>
#include <cstddef>

#include "../src/builders/residfp-builder/residfp/Dac.h"

namespace
{
constexpr unsigned int DAC_BITS = 8;
using DACArray = std::array<float, 1U << DAC_BITS>;

DACArray buildDac(reSIDfp::ChipModel chipModel)
{
    DACArray dac;
    reSIDfp::Dac dacBuilder(DAC_BITS);
    dacBuilder.kinkedDac(chipModel);

    for (unsigned int i = 0; i < dac.size(); i++)
    {
        dac[i] = dacBuilder.getOutput(i);
    }

    return dac;
}

bool isDacLinear(reSIDfp::ChipModel chipModel)
{
    const auto dac = buildDac(chipModel);

    for (std::size_t i = 1; i < dac.size(); i++)
    {
        if (dac[i] <= dac[i - 1])
            return false;
    }

    return true;
}
} // Anonymous namespace

TEST_CASE("Test DAC 6581", "[dac]")
{
    // Test the non-linearity of the 6581 DACs

    CHECK(!isDacLinear(reSIDfp::MOS6581));
}

TEST_CASE("Test DAC 8580", "[dac]")
{
    // Test the linearity of the 8580 DACs

    CHECK(isDacLinear(reSIDfp::MOS8580));
}
