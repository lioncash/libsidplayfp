//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581 SID emulator engine.
//  Copyright (C) 2010  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------

#include "extfilt.h"

namespace reSID
{

// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
ExternalFilter::ExternalFilter()
{
  reset();
  enable_filter(true);

  // Low-pass:  R = 10 kOhm, C = 1000 pF; w0l = 1/RC = 1/(1e4*1e-9) = 100 000
  // High-pass: R =  1 kOhm, C =   10 uF; w0h = 1/RC = 1/(1e3*1e-5) =     100

  // Assume a 1MHz clock.
  // Cutoff frequency accuracy (4 bits) is traded off for filter signal
  // accuracy (27 bits). This is crucial since w0lp and w0hp are so far apart.
  w0lp_1_s7 = int(100000*1.0e-6*(1 << 7) + 0.5);
  w0hp_1_s17 = int(100*1.0e-6*(1 << 17) + 0.5);
}


// ----------------------------------------------------------------------------
// Enable filter.
// ----------------------------------------------------------------------------
void ExternalFilter::enable_filter(bool enable)
{
  enabled = enable;
}

// ----------------------------------------------------------------------------
// SID clocking - 1 cycle.
// ----------------------------------------------------------------------------
void ExternalFilter::clock(short Vi)
{
    // This is handy for testing.
    if (unlikely(!enabled)) {
        // Vo  = Vlp - Vhp;
        Vlp = Vi << 11;
        Vhp = 0;
        return;
    }

    // Calculate filter outputs.
    // Vlp = Vlp + w0lp*(Vi - Vlp)*delta_t;
    // Vhp = Vhp + w0hp*(Vlp - Vhp)*delta_t;
    // Vo  = Vlp - Vhp;

    int dVlp = w0lp_1_s7 * int((unsigned(Vi) << 11) - unsigned(Vlp)) >> 7;
    int dVhp = w0hp_1_s17 * (Vlp - Vhp) >> 17;
    Vlp += dVlp;
    Vhp += dVhp;
}

// ----------------------------------------------------------------------------
// SID clocking - delta_t cycles.
// ----------------------------------------------------------------------------
void ExternalFilter::clock(cycle_count delta_t, short Vi)
{
    // This is handy for testing.
    if (unlikely(!enabled)) {
        // Vo  = Vlp - Vhp;
        Vlp = Vi << 11;
        Vhp = 0;
        return;
    }

    // Maximum delta cycles for the external filter to work satisfactorily
    // is approximately 8.
    cycle_count delta_t_flt = 8;

    while (delta_t) {
        if (unlikely(delta_t < delta_t_flt)) {
            delta_t_flt = delta_t;
        }

        // Calculate filter outputs.
        // Vlp = Vlp + w0lp*(Vi - Vlp)*delta_t;
        // Vhp = Vhp + w0hp*(Vlp - Vhp)*delta_t;
        // Vo  = Vlp - Vhp;

        int dVlp = (w0lp_1_s7 * delta_t_flt >> 3)* ((Vi << 11) - Vlp) >> 4;
        int dVhp = (w0hp_1_s17 * delta_t_flt >> 3)* (Vlp - Vhp) >> 14;
        Vlp += dVlp;
        Vhp += dVhp;

        delta_t -= delta_t_flt;
    }
}

// ----------------------------------------------------------------------------
// SID reset.
// ----------------------------------------------------------------------------
void ExternalFilter::reset()
{
  // State of filter.
  Vlp = 0;
  Vhp = 0;
}

// ----------------------------------------------------------------------------
// Audio output (16 bits).
// ----------------------------------------------------------------------------
short ExternalFilter::output()
{
    // Saturated arithmetics to guard against 16 bit sample overflow.
    const int half = 1 << 15;
    int Vo = (Vlp - Vhp) >> 11;
    if (Vo >= half) {
        Vo = half - 1;
    }
    else if (Vo < -half) {
        Vo = -half;
    }
    return Vo;
}

} // namespace reSID
