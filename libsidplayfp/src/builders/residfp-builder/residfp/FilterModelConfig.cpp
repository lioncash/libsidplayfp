/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2019 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2010 Dag Lem
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

#include "FilterModelConfig.h"

#include <cassert>
#include <cmath>
#include <cstddef>

#include "Integrator.h"
#include "OpAmp.h"
#include "Spline.h"

namespace reSIDfp
{
namespace
{
constexpr unsigned int OPAMP_SIZE = 33;

/**
 * This is the SID 6581 op-amp voltage transfer function, measured on
 * CAP1B/CAP1A on a chip marked MOS 6581R4AR 0687 14.
 * All measured chips have op-amps with output voltages (and thus input
 * voltages) within the range of 0.81V - 10.31V.
 */
constexpr std::array<Spline::Point, OPAMP_SIZE> opamp_voltage{{
  {  0.81, 10.31 },  // Approximate start of actual range
  {  2.40, 10.31 },
  {  2.60, 10.30 },
  {  2.70, 10.29 },
  {  2.80, 10.26 },
  {  2.90, 10.17 },
  {  3.00, 10.04 },
  {  3.10,  9.83 },
  {  3.20,  9.58 },
  {  3.30,  9.32 },
  {  3.50,  8.69 },
  {  3.70,  8.00 },
  {  4.00,  6.89 },
  {  4.40,  5.21 },
  {  4.54,  4.54 },  // Working point (vi = vo)
  {  4.60,  4.19 },
  {  4.80,  3.00 },
  {  4.90,  2.30 },  // Change of curvature
  {  4.95,  2.03 },
  {  5.00,  1.88 },
  {  5.05,  1.77 },
  {  5.10,  1.69 },
  {  5.20,  1.58 },
  {  5.40,  1.44 },
  {  5.60,  1.33 },
  {  5.80,  1.26 },
  {  6.00,  1.21 },
  {  6.40,  1.12 },
  {  7.00,  1.02 },
  {  7.50,  0.97 },
  {  8.50,  0.89 },
  { 10.00,  0.81 },
  { 10.31,  0.81 },  // Approximate end of actual range
}};
} // Anonymous namespace

std::unique_ptr<FilterModelConfig> FilterModelConfig::instance(nullptr);

FilterModelConfig* FilterModelConfig::getInstance()
{
    if (!instance)
    {
        instance.reset(new FilterModelConfig());
    }

    return instance.get();
}

FilterModelConfig::FilterModelConfig() : dac(DAC_BITS)
{
    dac.kinkedDac(MOS6581);

    // Convert op-amp voltage transfer to 16 bit values.

    std::array<Spline::Point, OPAMP_SIZE> scaled_voltage;
    for (std::size_t i = 0; i < scaled_voltage.size(); i++)
    {
        scaled_voltage[i].x = N16 * (opamp_voltage[i].x - opamp_voltage[i].y + denorm) / 2.;
        scaled_voltage[i].y = N16 * (opamp_voltage[i].x - vmin);
    }

    // Create lookup table mapping capacitor voltage to op-amp input voltage:

    const Spline s(scaled_voltage.data(), OPAMP_SIZE);

    for (std::size_t x = 0; x < opamp_rev.size(); x++)
    {
        const Spline::Point out = s.evaluate(double(x));
        double tmp = out.x;
        if (tmp < 0.0)
        {
            tmp = 0.0;
        }
        assert(tmp < 65535.5);
        opamp_rev[x] = static_cast<unsigned short>(tmp + 0.5);
    }

    // Create lookup tables for gains / summers.

    OpAmp opampModel(opamp_voltage.data(), OPAMP_SIZE, kVddt);

    // The filter summer operates at n ~ 1, and has 5 fundamentally different
    // input configurations (2 - 6 input "resistors").
    //
    // Note that all "on" transistors are modeled as one. This is not
    // entirely accurate, since the input for each transistor is different,
    // and transistors are not linear components. However modeling all
    // transistors separately would be extremely costly.
    for (std::size_t i = 0; i < summer.size(); i++)
    {
        const std::size_t idiv = 2 + i;        // 2 - 6 input "resistors".
        const std::size_t size = idiv << 16;
        const auto n = static_cast<double>(idiv);
        opampModel.reset();
        summer[i] = std::make_unique<std::uint16_t[]>(size);

        for (std::size_t vi = 0; vi < size; vi++)
        {
            const double vin = vmin + vi / N16 / idiv; /* vmin .. vmax */
            const double tmp = (opampModel.solve(n, vin) - vmin) * N16;
            assert(tmp > -0.5 && tmp < 65535.5);
            summer[i][vi] = static_cast<unsigned short>(tmp + 0.5);
        }
    }

    // The audio mixer operates at n ~ 8/6, and has 8 fundamentally different
    // input configurations (0 - 7 input "resistors").
    //
    // All "on", transistors are modeled as one - see comments above for
    // the filter summer.
    for (std::size_t i = 0; i < mixer.size(); i++)
    {
        const std::size_t idiv = (i == 0) ? 1 : i;
        const std::size_t size = (i == 0) ? 1 : i << 16;
        const double n = i * 8.0 / 6.0;
        opampModel.reset();
        mixer[i] = std::make_unique<std::uint16_t[]>(size);

        for (std::size_t vi = 0; vi < size; vi++)
        {
            const double vin = vmin + vi / N16 / idiv; /* vmin .. vmax */
            const double tmp = (opampModel.solve(n, vin) - vmin) * N16;
            assert(tmp > -0.5 && tmp < 65535.5);
            mixer[i][vi] = static_cast<unsigned short>(tmp + 0.5);
        }
    }

    // 4 bit "resistor" ladders in the bandpass resonance gain and the audio
    // output gain necessitate 16 gain tables.
    // From die photographs of the bandpass and volume "resistor" ladders
    // it follows that gain ~ vol/8 and 1/Q ~ ~res/8 (assuming ideal
    // op-amps and ideal "resistors").
    for (std::size_t n8 = 0; n8 < gain.size(); n8++)
    {
        constexpr std::size_t size = 1 << 16;
        const double n = n8 / 8.0;
        opampModel.reset();
        gain[n8] = std::make_unique<std::uint16_t[]>(size);

        for (std::size_t vi = 0; vi < size; vi++)
        {
            const double vin = vmin + vi / N16; /* vmin .. vmax */
            const double tmp = (opampModel.solve(n, vin) - vmin) * N16;
            assert(tmp > -0.5 && tmp < 65535.5);
            gain[n8][vi] = static_cast<unsigned short>(tmp + 0.5);
        }
    }

    constexpr double nkVddt = N16 * kVddt;
    constexpr double nVmin = N16 * vmin;

    for (std::size_t i = 0; i < vcr_kVg.size(); i++)
    {
        // The table index is right-shifted 16 times in order to fit in
        // 16 bits; the argument to sqrt is thus multiplied by (1 << 16).
        //
        // The returned value must be corrected for translation. Vg always
        // takes part in a subtraction as follows:
        //
        //   k*Vg - Vx = (k*Vg - t) - (Vx - t)
        //
        // I.e. k*Vg - t must be returned.
        const double Vg = nkVddt - std::sqrt(static_cast<double>(i << 16));
        const double tmp = k * Vg - nVmin;
        assert(tmp > -0.5 && tmp < 65535.5);
        vcr_kVg[i] = static_cast<unsigned short>(tmp + 0.5);
    }

    //  EKV model:
    //
    //  Ids = Is*(if - ir)
    //  Is = 2*u*Cox*Ut^2/k*W/L
    //  if = ln^2(1 + e^((k*(Vg - Vt) - Vs)/(2*Ut))
    //  ir = ln^2(1 + e^((k*(Vg - Vt) - Vd)/(2*Ut))

    constexpr double kVt = k * Vth;
    constexpr double Is = 2.0 * uCox * Ut * Ut / k * WL_vcr;

    // Normalized current factor for 1 cycle at 1MHz.
    constexpr double N15 = norm * ((1 << 15) - 1);
    constexpr double n_Is = N15 * 1.0e-6 / C * Is;

    // kVg_Vx = k*Vg - Vx
    // I.e. if k != 1.0, Vg must be scaled accordingly.
    for (std::size_t kVg_Vx = 0; kVg_Vx < vcr_n_Ids_term.size(); kVg_Vx++)
    {
        const double log_term = std::log1p(exp((kVg_Vx / N16 - kVt) / (2. * Ut)));
        // Scaled by m*2^15
        const double tmp = n_Is * log_term * log_term;
        assert(tmp > -0.5 && tmp < 65535.5);
        vcr_n_Ids_term[kVg_Vx] = static_cast<unsigned short>(tmp + 0.5);
    }
}

FilterModelConfig::~FilterModelConfig() = default;

unsigned short* FilterModelConfig::getDAC(double adjustment) const
{
    const double dac_zero = getDacZero(adjustment);

    constexpr std::size_t dac_size = 1U << DAC_BITS;
    auto* f0_dac = new unsigned short[dac_size];

    for (std::size_t i = 0; i < dac_size; i++)
    {
        const double fcd = dac.getOutput(static_cast<unsigned int>(i));
        const double tmp = N16 * (dac_zero + fcd * dac_scale / dac_size - vmin);
        assert(tmp > -0.5 && tmp < 65535.5);
        f0_dac[i] = static_cast<unsigned short>(tmp + 0.5);
    }

    return f0_dac;
}

std::unique_ptr<Integrator> FilterModelConfig::buildIntegrator()
{
    // Vdd - Vth, normalized so that translated values can be subtracted:
    // k*Vddt - x = (k*Vddt - t) - (x - t)
    double tmp = N16 * (kVddt - vmin);
    assert(tmp > -0.5 && tmp < 65535.5);
    const auto nkVddt = static_cast<unsigned short>(tmp + 0.5);

    // Normalized snake current factor, 1 cycle at 1MHz.
    // Fit in 5 bits.
    tmp = denorm * (1 << 13) * (uCox / (2. * k) * WL_snake * 1.0e-6 / C);
    assert(tmp > -0.5 && tmp < 65535.5);
    const auto n_snake = static_cast<unsigned short>(tmp + 0.5);

    return std::make_unique<Integrator>(vcr_kVg.data(), vcr_n_Ids_term.data(),
                                        opamp_rev.data(), nkVddt, n_snake);
}

} // namespace reSIDfp
