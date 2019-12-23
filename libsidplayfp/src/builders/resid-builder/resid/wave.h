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

#ifndef RESID_WAVE_H
#define RESID_WAVE_H

#include "resid-config.h"

namespace reSID
{

// ----------------------------------------------------------------------------
// A 24 bit accumulator is the basis for waveform generation. FREQ is added to
// the lower 16 bits of the accumulator each cycle.
// The accumulator is set to zero when TEST is set, and starts counting
// when TEST is cleared.
// The noise waveform is taken from intermediate bits of a 23 bit shift
// register. This register is clocked by bit 19 of the accumulator.
// ----------------------------------------------------------------------------
class WaveformGenerator
{
public:
  WaveformGenerator();

  void set_sync_source(WaveformGenerator*);
  void set_chip_model(chip_model model);

  void clock();
  void clock(cycle_count delta_t);
  void synchronize();
  void reset();

  void writeFREQ_LO(reg8);
  void writeFREQ_HI(reg8);
  void writePW_LO(reg8);
  void writePW_HI(reg8);
  void writeCONTROL_REG(reg8);
  reg8 readOSC();

  // 12-bit waveform output.
  short output();

  // Calculate and set waveform output value.
  void set_waveform_output();
  void set_waveform_output(cycle_count delta_t);

protected:
  void clock_shift_register();
  void write_shift_register();
  void reset_shift_register();
  void set_noise_output();

  const WaveformGenerator* sync_source;
  WaveformGenerator* sync_dest;

  reg24 accumulator;

  // Tell whether the accumulator MSB was set high on this cycle.
  bool msb_rising;

  // Fout  = (Fn*Fclk/16777216)Hz
  // reg16 freq;
  reg24 freq;
  // PWout = (PWn/40.95)%
  reg12 pw;

  reg24 shift_register;

  // Remaining time to fully reset shift register.
  cycle_count shift_register_reset;
  // Emulation of pipeline causing bit 19 to clock the shift register.
  cycle_count shift_pipeline;

  // Helper variables for waveform table lookup.
  reg24 ring_msb_mask;
  unsigned short no_noise;
  unsigned short noise_output;
  unsigned short no_noise_or_noise_output;
  unsigned short no_pulse;
  unsigned short pulse_output;

  // The control register right-shifted 4 bits; used for waveform table lookup.
  reg8 waveform;

  // 8580 tri/saw pipeline
  reg12 tri_saw_pipeline;
  reg12 osc3;

  // The remaining control register bits.
  reg8 test;
  reg8 ring_mod;
  reg8 sync;
  // The gate bit is handled by the EnvelopeGenerator.

  // DAC input.
  reg12 waveform_output;
  // Fading time for floating DAC input (waveform 0).
  cycle_count floating_output_ttl;

  chip_model sid_model;

  // Sample data for waveforms, not including noise.
  unsigned short* wave;
  static unsigned short model_wave[2][8][1 << 12];
  // DAC lookup tables.
  static unsigned short model_dac[2][1 << 12];

friend class Voice;
friend class SID;
};

} // namespace reSID

#endif // not RESID_WAVE_H
