/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2016 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004,2010 Dag Lem
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

#ifndef FILTERMODELCONFIG8580_H
#define FILTERMODELCONFIG8580_H

#include <array>
#include <memory>

namespace reSIDfp
{

class Integrator8580;

/**
 * Calculate parameters for 8580 filter emulation.
 */
class FilterModelConfig8580
{
public:
    using GainTable = std::array<std::unique_ptr<std::uint16_t[]>, 16>;
    using MixerTable = std::array<std::unique_ptr<std::uint16_t[]>, 8>;
    using OpAmpTable = std::array<std::uint16_t, 1 << 16>;
    using SummerTable = std::array<std::unique_ptr<std::uint16_t[]>, 5>;

    static FilterModelConfig8580* getInstance();

    /**
     * The digital range of one voice is 20 bits; create a scaling term
     * for multiplication which fits in 11 bits.
     */
    int getVoiceScaleS14() const
    {
        constexpr double voice_voltage_range = 0.4; // FIXME: Measure
        return static_cast<int>((norm * ((1 << 14) - 1)) * voice_voltage_range);
    }

    /**
     * The "zero" output level of the voices.
     */
    int getVoiceDC() const
    {
        constexpr double voice_DC_voltage = 4.80;   // FIXME: Was 4.76
        return static_cast<int>(N16 * (voice_DC_voltage - vmin));
    }

    const GainTable& getGainVol() const { return gain_vol; }
    const GainTable& getGainRes() const { return gain_res; }

    const SummerTable& getSummer() const { return summer; }

    const MixerTable& getMixer() const { return mixer; }

    /**
     * Construct an integrator solver.
     *
     * @return the integrator
     */
    std::unique_ptr<Integrator8580> buildIntegrator();

private:
    FilterModelConfig8580();
    ~FilterModelConfig8580();

    static std::unique_ptr<FilterModelConfig8580> instance;

    // This allows access to the private constructor
    friend std::unique_ptr<FilterModelConfig8580>::deleter_type;

    /// Capacitor value.
    static constexpr double C = 22e-9;

    /// Transistor parameters.
    //@{
    static constexpr double Vdd = 9.09;
    static constexpr double Vth = 0.80;              ///< Threshold voltage
    static constexpr double Ut = 26.0e-3;            ///< Thermal voltage: Ut = k*T/q = 8.61734315e-5*T ~ 26mV

    // FIXME: This is just a hack, k must be less than one, likely around 0.7
    static constexpr double k = 1.3;                 ///< Gate coupling coefficient: K = Cox/(Cox+Cdep) ~ 0.7

    // FIXME: Measure
    static constexpr double uCox = 55e-6;            ///< u*Cox
    static constexpr double kVddt = k * (Vdd - Vth); ///< k * (Vdd - Vth)
    //@}

    // Derived stuff
    static constexpr double vmin = 1.30;                       // First X value in the op amp table.
    static constexpr double vmax = kVddt < 8.91 ? 8.91: kVddt; // 8.91 -> First Y value in the op amp table.
    static constexpr double denorm = vmax - vmin;
    static constexpr double norm = 1.0 / denorm;

    /// Fixed point scaling for 16 bit op-amp output.
    static constexpr double N16 = norm * ((1 << 16) - 1);

    /// Lookup tables for gain and summer op-amps in output stage / filter.
    //@{
    MixerTable mixer;
    SummerTable summer;
    GainTable gain_vol;
    GainTable gain_res;
    //@}

    /// Reverse op-amp transfer function.
    OpAmpTable opamp_rev;
};

} // namespace reSIDfp

#endif
