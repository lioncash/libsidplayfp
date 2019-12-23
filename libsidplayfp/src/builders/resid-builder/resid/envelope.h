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

#ifndef RESID_ENVELOPE_H
#define RESID_ENVELOPE_H

#include "resid-config.h"

namespace reSID
{

// ----------------------------------------------------------------------------
// A 15 bit counter is used to implement the envelope rates, in effect
// dividing the clock to the envelope counter by the currently selected rate
// period.
// In addition, another counter is used to implement the exponential envelope
// decay, in effect further dividing the clock to the envelope counter.
// The period of this counter is set to 1, 2, 4, 8, 16, 30 at the envelope
// counter values 255, 93, 54, 26, 14, 6, respectively.
// ----------------------------------------------------------------------------
class EnvelopeGenerator
{
public:
  EnvelopeGenerator();

  enum State { ATTACK, DECAY_SUSTAIN, RELEASE, FREEZED };

  void set_chip_model(chip_model model);

  void clock();
  void clock(cycle_count delta_t);
  void reset();

  void writeCONTROL_REG(reg8);
  void writeATTACK_DECAY(reg8);
  void writeSUSTAIN_RELEASE(reg8);
  reg8 readENV();

  // 8-bit envelope output.
  short output();

protected:
  void set_exponential_counter();

  void state_change();

  reg16 rate_counter;
  reg16 rate_period;
  reg8 exponential_counter;
  reg8 exponential_counter_period;
  reg8 new_exponential_counter_period;
  reg8 envelope_counter;
  reg8 env3;
  // Emulation of pipeline delay for envelope decrement.
  cycle_count envelope_pipeline;
  cycle_count exponential_pipeline;
  cycle_count state_pipeline;
  bool hold_zero;
  bool reset_rate_counter;

  reg4 attack;
  reg4 decay;
  reg4 sustain;
  reg4 release;

  reg8 gate;

  State state;
  State next_state;

  chip_model sid_model;

  // Lookup table to convert from attack, decay, or release value to rate
  // counter period.
  static reg16 rate_counter_period[];

  // The 16 selectable sustain levels.
  static reg8 sustain_level[];

  // DAC lookup tables.
  static unsigned short model_dac[2][1 << 8];

friend class SID;
};

} // namespace reSID

#endif // not RESID_ENVELOPE_H
