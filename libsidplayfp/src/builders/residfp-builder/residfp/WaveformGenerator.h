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

#ifndef WAVEFORMGENERATOR_H
#define WAVEFORMGENERATOR_H

#include <array>
#include "array.h"
#include "siddefs-fp.h"

namespace reSIDfp
{

/**
 * A 24 bit accumulator is the basis for waveform generation.
 * FREQ is added to the lower 16 bits of the accumulator each cycle.
 * The accumulator is set to zero when TEST is set, and starts counting
 * when TEST is cleared.
 *
 * Waveforms are generated as follows:
 *
 * - No waveform:
 * When no waveform is selected, the DAC input is floating.
 *
 *
 * - Triangle:
 * The upper 12 bits of the accumulator are used.
 * The MSB is used to create the falling edge of the triangle by inverting
 * the lower 11 bits. The MSB is thrown away and the lower 11 bits are
 * left-shifted (half the resolution, full amplitude).
 * Ring modulation substitutes the MSB with MSB EOR NOT sync_source MSB.
 *
 *
 * - Sawtooth:
 * The output is identical to the upper 12 bits of the accumulator.
 *
 *
 * - Pulse:
 * The upper 12 bits of the accumulator are used.
 * These bits are compared to the pulse width register by a 12 bit digital
 * comparator; output is either all one or all zero bits.
 * The pulse setting is delayed one cycle after the compare.
 * The test bit, when set to one, holds the pulse waveform output at 0xfff
 * regardless of the pulse width setting.
 *
 *
 * - Noise:
 * The noise output is taken from intermediate bits of a 23-bit shift register
 * which is clocked by bit 19 of the accumulator.
 * The shift is delayed 2 cycles after bit 19 is set high.
 *
 * Operation: Calculate EOR result, shift register, set bit 0 = result.
 *
 *                    reset    -------------------------------------------
 *                      |     |                                           |
 *               test--OR-->EOR<--                                        |
 *                      |         |                                       |
 *                      2 2 2 1 1 1 1 1 1 1 1 1 1                         |
 *     Register bits:   2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 <---
 *                          |   |       |     |   |       |     |   |
 *     Waveform bits:       1   1       9     8   7       6     5   4
 *                          1   0
 *
 * The low 4 waveform bits are zero (grounded).
 */
class WaveformGenerator
{
public:
    // These five functions are not intended to be used in anything other than unit tests and internally.
    void clock_shift_register(unsigned int bit0);
    void write_shift_register();
    void reset_shift_register();
    void set_noise_output();

    void setWaveformModels(matrix_t* models);

    /**
     * Set the chip model.
     * This determines the type of the analog DAC emulation:
     * 8580 is perfectly linear while 6581 is nonlinear.
     *
     * @param chipModel
     */
    void setChipModel(ChipModel chipModel);

    /**
     * SID clocking - 1 cycle.
     */
    void clock();

    /**
     * Synchronize oscillators.
     * This must be done after all the oscillators have been clock()'ed,
     * so that they are in the same state.
     *
     * @param syncDest The oscillator I am syncing
     * @param syncSource The oscillator syncing me.
     */
    void synchronize(WaveformGenerator* syncDest, const WaveformGenerator* syncSource) const;

    /**
     * Write FREQ LO register.
     *
     * @param freq_lo low 8 bits of frequency
     */
    void writeFREQ_LO(unsigned char freq_lo) { freq = (freq & 0xff00) | (freq_lo & 0xff); }

    /**
     * Write FREQ HI register.
     *
     * @param freq_hi high 8 bits of frequency
     */
    void writeFREQ_HI(unsigned char freq_hi) { freq = (freq_hi << 8 & 0xff00) | (freq & 0xff); }

    /**
     * Write PW LO register.
     *
     * @param pw_lo low 8 bits of pulse width
     */
    void writePW_LO(unsigned char pw_lo) { pw = (pw & 0xf00) | (pw_lo & 0x0ff); }

    /**
     * Write PW HI register.
     *
     * @param pw_hi high 8 bits of pulse width
     */
    void writePW_HI(unsigned char pw_hi) { pw = (pw_hi << 8 & 0xf00) | (pw & 0x0ff); }

    /**
     * Write CONTROL REGISTER register.
     *
     * @param control control register value
     */
    void writeCONTROL_REG(unsigned char control);

    /**
     * SID reset.
     */
    void reset();

    /**
     * 12-bit waveform output as an analogue float value.
     *
     * @param ringModulator The oscillator ring-modulating current one.
     * @return output the waveform generator output
     */
    float output(const WaveformGenerator* ringModulator);

    /**
     * Read OSC3 value.
     */
    unsigned char readOSC() const { return static_cast<unsigned char>(osc3 >> 4); }

    /**
     * Read accumulator value.
     */
    unsigned int readAccumulator() const { return accumulator; }

    /**
     * Read freq value.
     */
    unsigned int readFreq() const { return freq; }

    /**
     * Read test value.
     */
    bool readTest() const { return test; }

    /**
     * Read sync value.
     */
    bool readSync() const { return sync; }

private:
    unsigned int get_noise_writeback() const;

    matrix_t* model_wave = nullptr;

    short* wave = nullptr;

    // PWout = (PWn/40.95)%
    unsigned int pw = 0;

    unsigned int shift_register = 0;

    /// Emulation of pipeline causing bit 19 to clock the shift register.
    int shift_pipeline = 0;

    unsigned int ring_msb_mask = 0;
    unsigned int no_noise = 0;
    unsigned int noise_output = 0;
    unsigned int no_noise_or_noise_output = 0;
    unsigned int no_pulse = 0;
    unsigned int pulse_output = 0;

    /// The control register right-shifted 4 bits; used for output function table lookup.
    unsigned int waveform = 0;

    int floating_output_ttl = 0;

    unsigned int waveform_output = 0;

    /// Current and previous accumulator value.
    unsigned int accumulator = 0x555555; // Accumulator's even bits are high on powerup

    // Fout  = (Fn*Fclk/16777216)Hz
    unsigned int freq = 0;

    // 8580 tri/saw pipeline
    unsigned int tri_saw_pipeline = 0x555;
    unsigned int osc3 = 0;

    /// Remaining time to fully reset shift register.
    int shift_register_reset = 0;

    /// Current chip model's shift register reset time.
    int model_shift_register_reset = 0;

    /// The control register bits. Gate is handled by EnvelopeGenerator.
    //@{
    bool test = false;
    bool sync = false;
    //@}

    /// Tell whether the accumulator MSB was set high on this cycle.
    bool msb_rising = false;

    bool is6581 = true;

    std::array<float, 4096> dac;
};

} // namespace reSIDfp

#endif
