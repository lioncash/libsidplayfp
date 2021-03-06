/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2016 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004,2010 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Dac.h"

#include <numeric>

namespace reSIDfp
{

Dac::Dac(std::size_t bits) : dac(bits)
{
}

Dac::~Dac() = default;

double Dac::getOutput(unsigned int input) const
{
    double dacValue = 0.0;

    for (std::size_t i = 0; i < dac.size(); i++)
    {
        if ((input & (std::size_t{1} << i)) != 0)
        {
            dacValue += dac[i];
        }
    }

    return dacValue;
}

void Dac::kinkedDac(ChipModel chipModel)
{
    const double R_INFINITY = 1e6;

    // Non-linearity parameter, 8580 DACs are perfectly linear
    const double _2R_div_R = chipModel == MOS6581 ? 2.20 : 2.00;

    // 6581 DACs are not terminated by a 2R resistor
    const bool term = chipModel == MOS8580;

    // Calculate voltage contribution by each individual bit in the R-2R ladder.
    for (std::size_t set_bit = 0; set_bit < dac.size(); set_bit++)
    {
        double Vn = 1.;                   // Normalized bit voltage.
        double R = 1.;                    // Normalized R
        const double _2R = _2R_div_R * R; // 2R
        double Rn = term ?                // Rn = 2R for correct termination,
                    _2R : R_INFINITY;     // INFINITY for missing termination.

        std::size_t bit;

        // Calculate DAC "tail" resistance by repeated parallel substitution.
        for (bit = 0; bit < set_bit; bit++)
        {
            Rn = (Rn == R_INFINITY) ?
                 R + _2R :
                 R + _2R * Rn / (_2R + Rn); // R + 2R || Rn
        }

        // Source transformation for bit voltage.
        if (Rn == R_INFINITY)
        {
            Rn = _2R;
        }
        else
        {
            Rn = _2R * Rn / (_2R + Rn); // 2R || Rn
            Vn = Vn * Rn / _2R;
        }

        // Calculate DAC output voltage by repeated source transformation from
        // the "tail".

        for (++bit; bit < dac.size(); bit++)
        {
            Rn += R;
            const double I = Vn / Rn;
            Rn = _2R * Rn / (_2R + Rn); // 2R || Rn
            Vn = Rn * I;
        }

        dac[set_bit] = Vn;
    }

    // Normalize to integerish behavior
    const auto divisor = std::size_t{1} << dac.size();
    const double Vsum = std::accumulate(dac.begin(), dac.end(), 0.0) / divisor;
    for (auto& element : dac)
    {
        element /= Vsum;
    }
}

} // namespace reSIDfp
