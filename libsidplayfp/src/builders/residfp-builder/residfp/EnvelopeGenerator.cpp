/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2018 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include "EnvelopeGenerator.h"

#include "Dac.h"

namespace reSIDfp
{

const unsigned int DAC_BITS = 8;

/**
 * Lookup table to convert from attack, decay, or release value to rate
 * counter period.
 *
 * The rate counter is a 15 bit register which is left shifted each cycle.
 * When the counter reaches a specific comparison value,
 * the envelope counter is incremented (attack) or decremented
 * (decay/release) and the rate counter is resetted.
 *
 * see [kevtris.org](http://blog.kevtris.org/?p=13)
 */
const unsigned int EnvelopeGenerator::adsrtable[16] =
{
    0x007f,
    0x3000,
    0x1e00,
    0x0660,
    0x0182,
    0x5573,
    0x000e,
    0x3805,
    0x2424,
    0x2220,
    0x090c,
    0x0ecd,
    0x010e,
    0x23f7,
    0x5237,
    0x64a8
};

/**
 * This is what happens on chip during state switching,
 * based on die reverse engineering and transistor level
 * emulation.
 *
 * Attack
 *
 *  0 - Gate on
 *  1 - Counting direction changes
 *      During this cycle the decay rate is "accidentally" activated
 *  2 - Counter is being inverted
 *      Now the attack rate is correctly activated
 *      Counter is enabled
 *  3 - Counter will be counting upward from now on
 *
 * Decay
 *
 *  0 - Counter == $ff
 *  1 - Counting direction changes
 *      The attack state is still active
 *  2 - Counter is being inverted
 *      During this cycle the decay state is activated
 *  3 - Counter will be counting downward from now on
 *
 * Release
 *
 *  0 - Gate off
 *  1 - During this cycle the release state is activated if coming from sustain/decay
 * *2 - Counter is being inverted, the release state is activated
 * *3 - Counter will be counting downward from now on
 *
 *  (* only if coming directly from Attack state)
 *
 * Freeze
 *
 *  0 - Counter == $00
 *  1 - Nothing
 *  2 - Counter is disabled
 */
void EnvelopeGenerator::state_change()
{
    state_pipeline--;

    switch (next_state)
    {
    case ATTACK:
        if (state_pipeline == 0)
        {
            state = ATTACK;
            // The attack rate register is correctly enabled during second cycle of attack phase
            rate = adsrtable[attack];
            counter_enabled = true;
        }
        break;
    case DECAY_SUSTAIN:
        break;
    case RELEASE:
        if (((state == ATTACK) && (state_pipeline == 0))
            || ((state == DECAY_SUSTAIN) && (state_pipeline == 1)))
        {
            state = RELEASE;
            rate = adsrtable[release];
        }
        break;
    }
}

void EnvelopeGenerator::set_exponential_counter()
{
    // Check for change of exponential counter period.
    //
    // For a detailed description see:
    // http://ploguechipsounds.blogspot.it/2010/03/sid-6581r3-adsr-tables-up-close.html
    switch (envelope_counter)
    {
    case 0xff:
        exponential_counter_period = 1;
        break;

    case 0x5d:
        exponential_counter_period = 2;
        break;

    case 0x36:
        exponential_counter_period = 4;
        break;

    case 0x1a:
        exponential_counter_period = 8;
        break;

    case 0x0e:
        exponential_counter_period = 16;
        break;

    case 0x06:
        exponential_counter_period = 30;
        break;

    case 0x00:
        exponential_counter_period = 1;
        break;
    }
}

void EnvelopeGenerator::setChipModel(ChipModel chipModel)
{
    Dac dacBuilder(DAC_BITS);
    dacBuilder.kinkedDac(chipModel);

    for (unsigned int i = 0; i < (1 << DAC_BITS); i++)
    {
        dac[i] = static_cast<float>(dacBuilder.getOutput(i));
    }
}

void EnvelopeGenerator::clock()
{
    env3 = envelope_counter;

    if (unlikely(state_pipeline))
    {
        state_change();
    }

    if (unlikely(envelope_pipeline != 0) && (--envelope_pipeline == 0))
    {
        if (likely(counter_enabled))
        {
            if (state == ATTACK)
            {
                if (++envelope_counter == 0xff)
                {
                    state = DECAY_SUSTAIN;
                    rate = adsrtable[decay];
                }
            }
            else if ((state == DECAY_SUSTAIN) || (state == RELEASE))
            {
                if (--envelope_counter == 0x00)
                {
                    counter_enabled = false;
                }
            }

            set_exponential_counter();
        }
    }
    else if (unlikely(exponential_pipeline != 0) && (--exponential_pipeline == 0))
    {
        exponential_counter = 0;

        if (((state == DECAY_SUSTAIN) && (envelope_counter != sustain))
            || (state == RELEASE))
        {
            // The envelope counter can flip from 0x00 to 0xff by changing state to
            // attack, then to release. The envelope counter will then continue
            // counting down in the release state.
            // This has been verified by sampling ENV3.

            envelope_pipeline = 1;
        }
    }
    else if (unlikely(resetLfsr))
    {
        lfsr = 0x7fff;
        resetLfsr = false;

        if (state == ATTACK)
        {
            // The first envelope step in the attack state also resets the exponential
            // counter. This has been verified by sampling ENV3.
            exponential_counter = 0; // NOTE this is actually delayed one cycle, not modeled

            // The envelope counter can flip from 0xff to 0x00 by changing state to
            // release, then to attack. The envelope counter is then frozen at
            // zero; to unlock this situation the state must be changed to release,
            // then to attack. This has been verified by sampling ENV3.

            envelope_pipeline = 2;
        }
        else
        {
            if (counter_enabled && (++exponential_counter == exponential_counter_period))
                exponential_pipeline = exponential_counter_period != 1 ? 2 : 1;
        }
    }

    // ADSR delay bug.
    // If the rate counter comparison value is set below the current value of the
    // rate counter, the counter will continue counting up until it wraps around
    // to zero at 2^15 = 0x8000, and then count rate_period - 1 before the
    // envelope can constly be stepped.
    // This has been verified by sampling ENV3.

    // check to see if LFSR matches table value
    if (likely(lfsr != rate))
    {
        // it wasn't a match, clock the LFSR once
        // by performing XOR on last 2 bits
        const unsigned int feedback = ((lfsr << 14) ^ (lfsr << 13)) & 0x4000;
        lfsr = (lfsr >> 1) | feedback;
    }
    else
    {
        resetLfsr = true;
    }
}

void EnvelopeGenerator::reset()
{
    // counter is not changed on reset
    envelope_pipeline = 0;

    state_pipeline = 0;

    attack = 0;
    decay = 0;
    sustain = 0;
    release = 0;

    gate = false;

    resetLfsr = true;

    exponential_counter = 0;
    exponential_counter_period = 1;

    state = RELEASE;
    counter_enabled = true;
    rate = adsrtable[release];
}

void EnvelopeGenerator::writeCONTROL_REG(unsigned char control)
{
    const bool gate_next = (control & 0x01) != 0;

    if (gate_next != gate)
    {
        gate = gate_next;

        // The rate counter is never reset, thus there will be a delay before the
        // envelope counter starts counting up (attack) or down (release).

        if (gate_next)
        {
            // Gate bit on:  Start attack, decay, sustain.
            next_state = ATTACK;
            state = DECAY_SUSTAIN;
            // The decay rate register is "accidentally" enabled during first cycle of attack phase
            rate = adsrtable[decay];
            state_pipeline = 2;
            if (resetLfsr || (exponential_pipeline == 2))
            {
                envelope_pipeline = (exponential_counter_period == 1) || (exponential_pipeline == 2) ? 2 : 4;
            }
            else if (exponential_pipeline == 1)
            {
                state_pipeline = 3;
            }
        }
        else
        {
            // Gate bit off: Start release.
            next_state = RELEASE;
            if (counter_enabled)
            {
                state_pipeline = envelope_pipeline > 0 ? 3 : 2;
            }
        }
    }
}

void EnvelopeGenerator::writeATTACK_DECAY(unsigned char attack_decay)
{
    attack = (attack_decay >> 4) & 0x0f;
    decay = attack_decay & 0x0f;

    if (state == ATTACK)
    {
        rate = adsrtable[attack];
    }
    else if (state == DECAY_SUSTAIN)
    {
        rate = adsrtable[decay];
    }
}

void EnvelopeGenerator::writeSUSTAIN_RELEASE(unsigned char sustain_release)
{
    // From the sustain levels it follows that both the low and high 4 bits
    // of the envelope counter are compared to the 4-bit sustain value.
    // This has been verified by sampling ENV3.
    //
    // For a detailed description see:
    // http://ploguechipsounds.blogspot.it/2010/11/new-research-on-sid-adsr.html
    sustain = (sustain_release & 0xf0) | ((sustain_release >> 4) & 0x0f);

    release = sustain_release & 0x0f;

    if (state == RELEASE)
    {
        rate = adsrtable[release];
    }
}

} // namespace reSIDfp
