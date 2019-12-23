/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2014 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include "Integrator.h"

namespace reSIDfp
{

int Integrator::solve(int vi)
{
    // Check that transistor is actually in triode mode
    // VDS < VGS - Vth
    assert(vi < kVddt);

    // "Snake" voltages for triode mode calculation.
    const unsigned int Vgst = kVddt - vx;
    const unsigned int Vgdt = kVddt - vi;

    const unsigned int Vgst_2 = Vgst * Vgst;
    const unsigned int Vgdt_2 = Vgdt * Vgdt;

    // "Snake" current, scaled by (1/m)*2^13*m*2^16*m*2^16*2^-15 = m*2^30
    const int n_I_snake = n_snake * (static_cast<int>(Vgst_2 - Vgdt_2) >> 15);

    // VCR gate voltage.       // Scaled by m*2^16
    // Vg = Vddt - sqrt(((Vddt - Vw)^2 + Vgdt^2)/2)
    const int kVg = static_cast<int>(vcr_kVg[(Vddt_Vw_2 + (Vgdt_2 >> 1)) >> 16]);

    // VCR voltages for EKV model table lookup.
    int Vgs = kVg - vx;
    if (Vgs < 0) Vgs = 0;
    assert(Vgs < (1 << 16));
    int Vgd = kVg - vi;
    if (Vgd < 0) Vgd = 0;
    assert(Vgd < (1 << 16));

    // VCR current, scaled by m*2^15*2^15 = m*2^30
    const int n_I_vcr = static_cast<int>(vcr_n_Ids_term[Vgs] - vcr_n_Ids_term[Vgd]) << 15;

    // Change in capacitor charge.
    vc += n_I_snake + n_I_vcr;

    // vx = g(vc)
    const int tmp = (vc >> 15) + (1 << 15);
    assert(tmp < (1 << 16));
    vx = opamp_rev[tmp];

    // Return vo.
    return vx - (vc >> 14);
}

} // namespace reSIDfp
