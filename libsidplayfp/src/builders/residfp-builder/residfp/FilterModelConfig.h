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

#ifndef FILTERMODELCONFIG_H
#define FILTERMODELCONFIG_H

#include <array>
#include <cstdint>
#include <memory>

#include "Dac.h"

namespace reSIDfp
{

class Integrator;

/**
 * Calculate parameters for 6581 filter emulation.
 */
class FilterModelConfig
{
public:
    using DACPtr = std::unique_ptr<const std::uint16_t[]>;
    using GainTable = std::array<std::unique_ptr<std::uint16_t[]>, 16>;
    using MixerTable = std::array<std::unique_ptr<std::uint16_t[]>, 8>;
    using SummerTable = std::array<std::unique_ptr<std::uint16_t[]>, 5>;

    static FilterModelConfig* getInstance();

    /**
     * The digital range of one voice is 20 bits; create a scaling term
     * for multiplication which fits in 11 bits.
     */
    int getVoiceScaleS14() const
    {
        constexpr double voice_voltage_range = 1.5;
        return static_cast<int>((norm * ((1 << 14) - 1)) * voice_voltage_range);
    }

    /**
     * The "zero" output level of the voices.
     */
    int getVoiceDC() const
    {
        constexpr double voice_DC_voltage = 5.0;
        return static_cast<int>(N16 * (voice_DC_voltage - vmin));
    }

    const GainTable& getGain() const { return gain; }

    const SummerTable& getSummer() const { return summer; }

    const MixerTable& getMixer() const { return mixer; }

    /**
     * Construct an 11 bit cutoff frequency DAC output voltage table.
     *
     * @param adjustment
     *
     * @return A smart pointer that points to the DAC output voltage table.
     */
    DACPtr getDAC(double adjustment) const;

    /**
     * Construct an integrator solver.
     *
     * @return the integrator
     */
    std::unique_ptr<Integrator> buildIntegrator();

private:
    FilterModelConfig();
    ~FilterModelConfig();

    double getDacZero(double adjustment) const { return dac_zero - (adjustment - 0.5) * 2.0; }

    static constexpr unsigned int DAC_BITS = 11;

    static std::unique_ptr<FilterModelConfig> instance;
    // This allows access to the private constructor
    friend std::unique_ptr<FilterModelConfig>::deleter_type;

    /// Capacitor value.
    static constexpr double C = 470e-12;

    /// Transistor parameters.
    //@{
    static constexpr double Vdd = 12.18;
    static constexpr double Vth = 1.31;              ///< Threshold voltage
    static constexpr double Ut = 26.0e-3;            ///< Thermal voltage: Ut = k*T/q = 8.61734315e-5*T ~ 26mV
    static constexpr double k = 1.0;                 ///< Gate coupling coefficient: K = Cox/(Cox+Cdep) ~ 0.7
    static constexpr double uCox = 20e-6;            ///< u*Cox
    static constexpr double WL_vcr = 9.0 / 1.0;      ///< W/L for VCR
    static constexpr double WL_snake = 1.0 / 115.0;  ///< W/L for "snake"
    static constexpr double kVddt = k * (Vdd - Vth); ///< k * (Vdd - Vth)
    //@}

    /// DAC parameters.
    //@{
    static constexpr double dac_zero = 6.65;
    static constexpr double dac_scale = 2.63;
    //@}

    // Derived stuff
    static constexpr double vmin = 0.81;                          // First X value within the OpAmp voltage table.
    static constexpr double vmax = kVddt < 10.31 ? 10.31 : kVddt; // 10.31 -> First Y value within the OpAmp voltage table.
    static constexpr double denorm = vmax - vmin;
    static constexpr double norm = 1.0 / denorm;

    /// Fixed point scaling for 16 bit op-amp output.
    static constexpr double N16 = norm * ((1 << 16) - 1);

    /// Lookup tables for gain and summer op-amps in output stage / filter.
    //@{
    MixerTable mixer;
    SummerTable summer;
    GainTable gain;
    //@}

    /// DAC lookup table
    Dac dac;

    /// VCR - 6581 only.
    //@{
    std::array<unsigned short, 1 << 16> vcr_kVg;
    std::array<unsigned short, 1 << 16> vcr_n_Ids_term;
    //@}

    /// Reverse op-amp transfer function.
    std::array<unsigned short, 1 << 16> opamp_rev;
};

} // namespace reSIDfp

#endif
