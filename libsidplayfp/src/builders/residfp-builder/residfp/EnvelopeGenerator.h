/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2019 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2018 VICE Project
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

#ifndef ENVELOPEGENERATOR_H
#define ENVELOPEGENERATOR_H

#include "siddefs-fp.h"

namespace reSIDfp
{

/**
 * A 15 bit [LFSR] is used to implement the envelope rates, in effect dividing
 * the clock to the envelope counter by the currently selected rate period.
 *
 * In addition, another 5 bit counter is used to implement the exponential envelope decay,
 * in effect further dividing the clock to the envelope counter.
 * The period of this counter is set to 1, 2, 4, 8, 16, 30 at the envelope counter
 * values 255, 93, 54, 26, 14, 6, respectively.
 *
 * [LFSR]: https://en.wikipedia.org/wiki/Linear_feedback_shift_register
 */
class EnvelopeGenerator
{
private:
    /**
     * The envelope state machine's distinct states. In addition to this,
     * envelope has a hold mode, which freezes envelope counter to zero.
     */
    enum State
    {
        ATTACK, DECAY_SUSTAIN, RELEASE
    };

private:
    /// XOR shift register for ADSR prescaling.
    unsigned int lfsr;

    /// Comparison value (period) of the rate counter before next event.
    unsigned int rate;

    /**
     * During release mode, the SID arpproximates envelope decay via piecewise
     * linear decay rate.
     */
    unsigned int exponential_counter;

    /**
     * Comparison value (period) of the exponential decay counter before next
     * decrement.
     */
    unsigned int exponential_counter_period;

    unsigned int state_pipeline;

    ///
    unsigned int envelope_pipeline;

    unsigned int exponential_pipeline;

    /// Current envelope state
    State state;
    State next_state;

    /// Whether counter is enabled. Only switching to ATTACK can release envelope.
    bool counter_enabled;

    /// Gate bit
    bool gate;

    ///
    bool resetLfsr;

    /// The current digital value of envelope output.
    unsigned char envelope_counter;

    /// Attack register
    unsigned char attack;

    /// Decay register
    unsigned char decay;

    /// Sustain register
    unsigned char sustain;

    /// Release register
    unsigned char release;

    /// The ENV3 value, sampled at the first phase of the clock
    unsigned char env3;

    /**
     * Emulated nonlinearity of the envelope DAC.
     *
     * @See Dac
     */
    float dac[256];

private:
    static const unsigned int adsrtable[16];

private:
    void set_exponential_counter();

    void state_change();

public:
    /**
     * Set chip model.
     * This determines the type of the analog DAC emulation:
     * 8580 is perfectly linear while 6581 is nonlinear.
     *
     * @param chipModel
     */
    void setChipModel(ChipModel chipModel);

    /**
     * SID clocking.
     */
    void clock();

    /**
     * Get the Envelope Generator output.
     * DAC imperfections are emulated by using envelope_counter as an index
     * into a DAC lookup table. readENV() uses envelope_counter directly.
     */
    float output() const { return dac[envelope_counter]; }

    /**
     * Constructor.
     */
    EnvelopeGenerator() :
        lfsr(0x7fff),
        rate(0),
        exponential_counter(0),
        exponential_counter_period(1),
        state_pipeline(0),
        envelope_pipeline(0),
        exponential_pipeline(0),
        state(RELEASE),
        next_state(RELEASE),
        counter_enabled(true),
        gate(false),
        resetLfsr(false),
        envelope_counter(0xaa),
        attack(0),
        decay(0),
        sustain(0),
        release(0),
        env3(0)
    {}

    /**
     * SID reset.
     */
    void reset();

    /**
     * Write control register.
     *
     * @param control
     *            control register
     */
    void writeCONTROL_REG(unsigned char control);

    /**
     * Write Attack/Decay register.
     *
     * @param attack_decay
     *            attack/decay value
     */
    void writeATTACK_DECAY(unsigned char attack_decay);

    /**
     * Write Sustain/Release register.
     *
     * @param sustain_release
     *            sustain/release value
     */
    void writeSUSTAIN_RELEASE(unsigned char sustain_release);

    /**
     * Return the envelope current value.
     *
     * @return envelope counter
     */
    unsigned char readENV() const { return env3; }
};

} // namespace reSIDfp

#endif
