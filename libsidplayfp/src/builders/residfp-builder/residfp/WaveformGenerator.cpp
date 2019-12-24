/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2019 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004 Dag Lem <resid@nimrod.no>
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

#include "WaveformGenerator.h"

#include <cstddef>

#include "Dac.h"

namespace reSIDfp
{

/**
 * Number of cycles after which the waveform output fades to 0 when setting
 * the waveform register to 0.
 *
 * FIXME
 * This value has been adjusted aleatorily to ~1 sec
 * from the original reSID value (0x4000)
 * to fix /MUSICIANS/H/Hatlelid_Kris/Grand_Prix_Circuit.sid#2
 * and /MUSICIANS/P/PVCF/Thomkat_with_Strange_End.sid;
 * see [VICE Bug #290](http://sourceforge.net/p/vice-emu/bugs/290/)
 */
constexpr int FLOATING_OUTPUT_TTL_6581 = 200000;  // ~200ms
constexpr int FLOATING_OUTPUT_TTL_8580 = 5000000; // ~5s;

/**
 * Number of cycles after which the shift register is reset
 * when the test bit is set.
 * Values measured on warm chips (6581R3 and 8580R5).
 * Times vary wildly with temperature and may differ
 * from chip to chip so the numbers here represents
 * only the big difference between the old and new models.
 */
constexpr int SHIFT_REGISTER_RESET_6581 = 200000;  // ~200ms
constexpr int SHIFT_REGISTER_RESET_8580 = 5000000; // ~5s

constexpr std::size_t DAC_BITS = 12;

/*
 * This is what happens when the lfsr is clocked:
 *
 * cycle 0: bit 19 of the accumulator goes from low to high, the noise register acts normally,
 *          the output may overwrite a bit;
 *
 * cycle 1: first phase of the shift, the bits are interconnected and the output of each bit
 *          is latched into the following. The output may overwrite the latched value.
 *
 * cycle 2: second phase of the shift, the latched value becomes active in the first
 *          half of the clock and from the second half the register returns to normal operation.
 *
 * When the test or reset lines are active the first phase is executed at every cyle
 * until the signal is released triggering the second phase.
 */
void WaveformGenerator::clock_shift_register(unsigned int bit0)
{
    shift_register = (shift_register >> 1) | bit0;

    // New noise waveform output.
    set_noise_output();
}

unsigned int WaveformGenerator::get_noise_writeback() const
{
  return
    ~(
        (1 <<  2) |  // Bit 20
        (1 <<  4) |  // Bit 18
        (1 <<  8) |  // Bit 14
        (1 << 11) |  // Bit 11
        (1 << 13) |  // Bit  9
        (1 << 17) |  // Bit  5
        (1 << 20) |  // Bit  2
        (1 << 22)    // Bit  0
    ) |
    ((waveform_output & (1 << 11)) >>  9) |  // Bit 11 -> bit 20
    ((waveform_output & (1 << 10)) >>  6) |  // Bit 10 -> bit 18
    ((waveform_output & (1 <<  9)) >>  1) |  // Bit  9 -> bit 14
    ((waveform_output & (1 <<  8)) <<  3) |  // Bit  8 -> bit 11
    ((waveform_output & (1 <<  7)) <<  6) |  // Bit  7 -> bit  9
    ((waveform_output & (1 <<  6)) << 11) |  // Bit  6 -> bit  5
    ((waveform_output & (1 <<  5)) << 15) |  // Bit  5 -> bit  2
    ((waveform_output & (1 <<  4)) << 18);   // Bit  4 -> bit  0
}

void WaveformGenerator::write_shift_register()
{
    if (unlikely(waveform > 0x8) && likely(!test) && likely(shift_pipeline != 1))
    {
        // Write changes to the shift register output caused by combined waveforms
        // back into the shift register. This happens only when the register is clocked
        // (see $D1+$81_wave_test [1]) or when the test bit is set.
        // A bit once set to zero cannot be changed, hence the and'ing.
        //
        // [1] ftp://ftp.untergrund.net/users/nata/sid_test/$D1+$81_wave_test.7z
        //
        // FIXME: Write test program to check the effect of 1 bits and whether
        // neighboring bits are affected.

        shift_register &= get_noise_writeback();

        noise_output &= waveform_output;
        no_noise_or_noise_output = no_noise | noise_output;
    }
}

void WaveformGenerator::reset_shift_register()
{
    shift_register = 0x7fffff;
    shift_register_reset = 0;
}

void WaveformGenerator::set_noise_output()
{
    noise_output =
        ((shift_register & (1 <<  2)) <<  9) |  // Bit 20 -> bit 11
        ((shift_register & (1 <<  4)) <<  6) |  // Bit 18 -> bit 10
        ((shift_register & (1 <<  8)) <<  1) |  // Bit 14 -> bit  9
        ((shift_register & (1 << 11)) >>  3) |  // Bit 11 -> bit  8
        ((shift_register & (1 << 13)) >>  6) |  // Bit  9 -> bit  7
        ((shift_register & (1 << 17)) >> 11) |  // Bit  5 -> bit  6
        ((shift_register & (1 << 20)) >> 15) |  // Bit  2 -> bit  5
        ((shift_register & (1 << 22)) >> 18);   // Bit  0 -> bit  4

    no_noise_or_noise_output = no_noise | noise_output;
}

void WaveformGenerator::setWaveformModels(matrix_t* models)
{
    model_wave = models;
}

void WaveformGenerator::setChipModel(ChipModel chipModel)
{
    is6581 = chipModel == MOS6581;

    Dac dacBuilder(DAC_BITS);
    dacBuilder.kinkedDac(chipModel);

    const double offset = dacBuilder.getOutput(is6581 ? 0x380 : 0x9c0);

    for (unsigned int i = 0; i < dac.size(); i++)
    {
        const double dacValue = dacBuilder.getOutput(i);
        dac[i] = static_cast<float>(dacValue - offset);
    }

    model_shift_register_reset = is6581 ? SHIFT_REGISTER_RESET_6581 : SHIFT_REGISTER_RESET_8580;
}

void WaveformGenerator::clock()
{
    if (unlikely(test))
    {
        if (unlikely(shift_register_reset != 0) && unlikely(--shift_register_reset == 0))
        {
            reset_shift_register();

            // New noise waveform output.
            set_noise_output();
        }

        // The test bit sets pulse high.
        pulse_output = 0xfff;
    }
    else
    {
        // Calculate new accumulator value;
        const unsigned int accumulator_old = accumulator;
        accumulator = (accumulator + freq) & 0xffffff;

        // Check which bit have changed
        const unsigned int accumulator_bits_set = ~accumulator_old & accumulator;

        // Check whether the MSB is set high. This is used for synchronization.
        msb_rising = (accumulator_bits_set & 0x800000) != 0;

        // Shift noise register once for each time accumulator bit 19 is set high.
        // The shift is delayed 2 cycles.
        if (unlikely((accumulator_bits_set & 0x080000) != 0))
        {
            // Pipeline: Detect rising bit, shift phase 1, shift phase 2.
            shift_pipeline = 2;
        }
        else if (unlikely(shift_pipeline != 0) && --shift_pipeline == 0)
        {
            // bit0 = (bit22 | test) ^ bit17
            clock_shift_register(((shift_register << 22) ^ (shift_register << 17)) & (1 << 22));
        }
    }
}

void WaveformGenerator::synchronize(WaveformGenerator* syncDest, const WaveformGenerator* syncSource) const
{
    // A special case occurs when a sync source is synced itself on the same
    // cycle as when its MSB is set high. In this case the destination will
    // not be synced. This has been verified by sampling OSC3.
    if (unlikely(msb_rising) && syncDest->sync && !(sync && syncSource->msb_rising))
    {
        syncDest->accumulator = 0;
    }
}

bool do_pre_writeback(unsigned int waveform_prev, unsigned int waveform, bool is6581)
{
    // no writeback without combined waveforms
    if (likely(waveform_prev <= 0x8))
        return false;
    // This need more investigation
    if (waveform == 8)
        return false;
    // What's happening here?
    if (is6581 &&
            ((((waveform_prev & 0x3) == 0x1) && ((waveform & 0x3) == 0x2))
            || (((waveform_prev & 0x3) == 0x2) && ((waveform & 0x3) == 0x1))))
        return false;
    // ok do the writeback
    return true;
}

void WaveformGenerator::writeCONTROL_REG(unsigned char control)
{
    const unsigned int waveform_prev = waveform;
    const bool test_prev = test;

    waveform = (control >> 4) & 0x0f;
    test = (control & 0x08) != 0;
    sync = (control & 0x02) != 0;

    // Substitution of accumulator MSB when sawtooth = 0, ring_mod = 1.
    ring_msb_mask = ((~control >> 5) & (control >> 2) & 0x1) << 23;

    if (waveform != waveform_prev)
    {
        // Set up waveform table.
        wave = (*model_wave)[waveform & 0x7];

        // no_noise and no_pulse are used in set_waveform_output() as bitmasks to
        // only let the noise or pulse influence the output when the noise or pulse
        // waveforms are selected.
        no_noise = (waveform & 0x8) != 0 ? 0x000 : 0xfff;
        no_noise_or_noise_output = no_noise | noise_output;
        no_pulse = (waveform & 0x4) != 0 ? 0x000 : 0xfff;

        if (waveform == 0)
        {
            // Change to floating DAC input.
            // Reset fading time for floating DAC input.
            floating_output_ttl = is6581 ? FLOATING_OUTPUT_TTL_6581 : FLOATING_OUTPUT_TTL_8580;
        }
    }

    if (test != test_prev)
    {
        if (test)
        {
            // Reset accumulator.
            accumulator = 0;

            // Flush shift pipeline.
            shift_pipeline = 0;

            // Set reset time for shift register.
            shift_register_reset = model_shift_register_reset;

            // New noise waveform output.
            set_noise_output();
        }
        else
        {
            // When the test bit is falling, the second phase of the shift is
            // completed by enabling SRAM write.

            // During first phase of the shift the bits are interconnected
            // and the output of each bit is latched into the following.
            // The output may overwrite the latched value.
            if (do_pre_writeback(waveform_prev, waveform, is6581)) {
                shift_register &= get_noise_writeback();
            }

            // bit0 = (bit22 | test) ^ bit17 = 1 ^ bit17 = ~bit17
            clock_shift_register((~shift_register << 17) & (1 << 22));
        }
    }
}

void WaveformGenerator::reset()
{
    // accumulator is not changed on reset
    freq = 0;
    pw = 0;

    msb_rising = false;

    waveform = 0;
    osc3 = 0;

    test = false;
    sync = false;

    wave = model_wave ? (*model_wave)[0] : nullptr;

    ring_msb_mask = 0;
    no_noise = 0xfff;
    no_pulse = 0xfff;
    pulse_output = 0xfff;

    reset_shift_register();
    // when reset is released the shift register is clocked once
    clock_shift_register((~shift_register << 17) & (1 << 22));

    shift_pipeline = 0;

    waveform_output = 0;
    floating_output_ttl = 0;
}

float WaveformGenerator::output(const WaveformGenerator* ringModulator)
{
    // Set output value.
    if (likely(waveform != 0))
    {
        const unsigned int ix = (accumulator ^ (~ringModulator->accumulator & ring_msb_mask)) >> 12;

        // The bit masks no_pulse and no_noise are used to achieve branch-free
        // calculation of the output value.
        waveform_output = wave[ix] & (no_pulse | pulse_output) & no_noise_or_noise_output;

        // Triangle/Sawtooth output is delayed half cycle on 8580.
        // This will appear as a one cycle delay on OSC3 as it is latched first phase of the clock.
        if ((waveform & 3) && !is6581)
        {
            osc3 = tri_saw_pipeline & (no_pulse | pulse_output) & no_noise_or_noise_output;
            tri_saw_pipeline = wave[ix];
        }
        else
        {
            osc3 = waveform_output;
        }

        // In the 6581 the top bit of the accumulator may be driven low by combined waveforms
        // when the sawtooth is selected
        // FIXME doesn't seem to always happen
        if ((waveform & 2) && unlikely(waveform & 0xd) && is6581)
            accumulator &= (waveform_output << 12) | 0x7fffff;

        write_shift_register();
    }
    else
    {
        // Age floating DAC input.
        if (likely(floating_output_ttl != 0) && unlikely(--floating_output_ttl == 0))
        {
            waveform_output = 0;
        }
    }

    // The pulse level is defined as (accumulator >> 12) >= pw ? 0xfff : 0x000.
    // The expression -((accumulator >> 12) >= pw) & 0xfff yields the same
    // results without any branching (and thus without any pipeline stalls).
    // NB! This expression relies on that the result of a boolean expression
    // is either 0 or 1, and furthermore requires two's complement integer.
    // A few more cycles may be saved by storing the pulse width left shifted
    // 12 bits, and dropping the and with 0xfff (this is valid since pulse is
    // used as a bit mask on 12 bit values), yielding the expression
    // -(accumulator >= pw24). However this only results in negligible savings.

    // The result of the pulse width compare is delayed one cycle.
    // Push next pulse level into pulse level pipeline.
    pulse_output = ((accumulator >> 12) >= pw) ? 0xfff : 0x000;

    // DAC imperfections are emulated by using waveform_output as an index
    // into a DAC lookup table. readOSC() uses waveform_output directly.
    return dac[waveform_output];
}

} // namespace reSIDfp
