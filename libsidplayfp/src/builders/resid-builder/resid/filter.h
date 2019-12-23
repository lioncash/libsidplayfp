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

#ifndef RESID_FILTER_H
#define RESID_FILTER_H

#include "resid-config.h"

namespace reSID
{

// ----------------------------------------------------------------------------
// The SID filter is modeled with a two-integrator-loop biquadratic filter,
// which has been confirmed by Bob Yannes to be the actual circuit used in
// the SID chip.
//
// Measurements show that excellent emulation of the SID filter is achieved,
// except when high resonance is combined with high sustain levels.
// In this case the SID op-amps are performing less than ideally and are
// causing some peculiar behavior of the SID filter. This however seems to
// have more effect on the overall amplitude than on the color of the sound.
//
// The theory for the filter circuit can be found in "Microelectric Circuits"
// by Adel S. Sedra and Kenneth C. Smith.
// The circuit is modeled based on the explanation found there except that
// an additional inverter is used in the feedback from the bandpass output,
// allowing the summer op-amp to operate in single-ended mode. This yields
// filter outputs with levels independent of Q, which corresponds with the
// results obtained from a real SID.
//
// We have been able to model the summer and the two integrators of the circuit
// to form components of an IIR filter.
// Vhp is the output of the summer, Vbp is the output of the first integrator,
// and Vlp is the output of the second integrator in the filter circuit.
//
// According to Bob Yannes, the active stages of the SID filter are not really
// op-amps. Rather, simple NMOS inverters are used. By biasing an inverter
// into its region of quasi-linear operation using a feedback resistor from
// input to output, a MOS inverter can be made to act like an op-amp for
// small signals centered around the switching threshold.
//
// In 2008, Michael Huth facilitated closer investigation of the SID 6581
// filter circuit by publishing high quality microscope photographs of the die.
// Tommi Lempinen has done an impressive work on re-vectorizing and annotating
// the die photographs, substantially simplifying further analysis of the
// filter circuit.
// 
// The filter schematics below are reverse engineered from these re-vectorized
// and annotated die photographs. While the filter first depicted in reSID 0.9
// is a correct model of the basic filter, the schematics are now completed
// with the audio mixer and output stage, including details on intended
// relative resistor values. Also included are schematics for the NMOS FET
// voltage controlled resistors (VCRs) used to control cutoff frequency, the
// DAC which controls the VCRs, the NMOS op-amps, and the output buffer.
//
//
// SID filter / mixer / output
// ---------------------------
// 
//                ---------------------------------------------------
//               |                                                   |
//               |                         --1R1-- \--  D7           |
//               |              ---R1--   |           |              |
//               |             |       |  |--2R1-- \--| D6           |
//               |    ------------<A]-----|           |     $17      |
//               |   |                    |--4R1-- \--| D5  1=open   | (3.5R1)
//               |   |                    |           |              |
//               |   |                     --8R1-- \--| D4           | (7.0R1)
//               |   |                                |              |
// $17           |   |                    (CAP2B)     |  (CAP1B)     |
// 0=to mixer    |    --R8--    ---R8--        ---C---|       ---C---| 
// 1=to filter   |          |  |       |      |       |      |       |
//                ------R8--|-----[A>--|--Rw-----[A>--|--Rw-----[A>--|
//     ve (EXT IN)          |          |              |              |
// D3  \ ---------------R8--|          |              | (CAP2A)      | (CAP1A)
//     |   v3               |          | vhp          | vbp          | vlp
// D2  |   \ -----------R8--|     -----               |              |
//     |   |   v2           |    |                    |              |
// D1  |   |   \ -------R8--|    |    ----------------               |
//     |   |   |   v1       |    |   |                               |
// D0  |   |   |   \ ---R8--     |   |    ---------------------------
//     |   |   |   |             |   |   |
//     R6  R6  R6  R6            R6  R6  R6
//     |   |   |   | $18         |   |   |  $18
//     |    \  |   | D7: 1=open   \   \   \ D6 - D4: 0=open
//     |   |   |   |             |   |   |
//      ---------------------------------                          12V
//                 |
//                 |               D3  --/ --1R2--                  |
//                 |    ---R8--       |           |   ---R2--       |
//                 |   |       |   D2 |--/ --2R2--|  |       |  ||--
//                  ------[A>---------|           |-----[A>-----||
//                                 D1 |--/ --4R2--| (4.25R2)    ||--
//                        $18         |           |                 |
//                        0=open   D0  --/ --8R2--  (8.75R2)        |
//
//                                                                  vo (AUDIO
//                                                                      OUT)
//
//
// v1  - voice 1
// v2  - voice 2
// v3  - voice 3
// ve  - ext in
// vhp - highpass output
// vbp - bandpass output
// vlp - lowpass output
// vo  - audio out
// [A> - single ended inverting op-amp (self-biased NMOS inverter)
// Rn  - "resistors", implemented with custom NMOS FETs
// Rw  - cutoff frequency resistor (VCR)
// C   - capacitor
//
// Notes:
//
// R2  ~  2.0*R1
// R6  ~  6.0*R1
// R8  ~  8.0*R1
// R24 ~ 24.0*R1
//
// The Rn "resistors" in the circuit are implemented with custom NMOS FETs,
// probably because of space constraints on the SID die. The silicon substrate
// is laid out in a narrow strip or "snake", with a strip length proportional
// to the intended resistance. The polysilicon gate electrode covers the entire
// silicon substrate and is fixed at 12V in order for the NMOS FET to operate
// in triode mode (a.k.a. linear mode or ohmic mode).
//
// Even in "linear mode", an NMOS FET is only an approximation of a resistor,
// as the apparant resistance increases with increasing drain-to-source
// voltage. If the drain-to-source voltage should approach the gate voltage
// of 12V, the NMOS FET will enter saturation mode (a.k.a. active mode), and
// the NMOS FET will not operate anywhere like a resistor.
//
// 
// 
// NMOS FET voltage controlled resistor (VCR)
// ------------------------------------------
//
//                Vw
//
//                |
//                |
//                R1
//                |
//          --R1--|
//         |    __|__
//         |    -----
//         |    |   |
// vi ----------     -------- vo
//         |           |
//          ----R24----
//
//
// vi  - input
// vo  - output
// Rn  - "resistors", implemented with custom NMOS FETs
// Vw  - voltage from 11-bit DAC (frequency cutoff control)
// 
// Notes:
//
// An approximate value for R24 can be found by using the formula for the
// filter cutoff frequency:
//
// FCmin = 1/(2*pi*Rmax*C)
//
// Assuming that a the setting for minimum cutoff frequency in combination with
// a low level input signal ensures that only negligible current will flow
// through the transistor in the schematics above, values for FCmin and C can
// be substituted in this formula to find Rmax.
// Using C = 470pF and FCmin = 220Hz (measured value), we get:
//
// FCmin = 1/(2*pi*Rmax*C)
// Rmax = 1/(2*pi*FCmin*C) = 1/(2*pi*220*470e-12) ~ 1.5MOhm
//
// From this it follows that:
// R24 =  Rmax   ~ 1.5MOhm
// R1  ~  R24/24 ~  64kOhm
// R2  ~  2.0*R1 ~ 128kOhm
// R6  ~  6.0*R1 ~ 384kOhm
// R8  ~  8.0*R1 ~ 512kOhm
//
// Note that these are only approximate values for one particular SID chip,
// due to process variations the values can be substantially different in
// other chips.
// 
// 
// 
// Filter frequency cutoff DAC
// ---------------------------
//
//
//        12V  10   9   8   7   6   5   4   3   2   1   0   VGND
//          |   |   |   |   |   |   |   |   |   |   |   |     |   Missing
//         2R  2R  2R  2R  2R  2R  2R  2R  2R  2R  2R  2R    2R   termination
//          |   |   |   |   |   |   |   |   |   |   |   |     |
//     Vw ----R---R---R---R---R---R---R---R---R---R---R--   ---
//
// Bit on:  12V
// Bit off:  5V (VGND)
//
// As is the case with all MOS 6581 DACs, the termination to (virtual) ground
// at bit 0 is missing.
//
// Furthermore, the control of the two VCRs imposes a load on the DAC output
// which varies with the input signals to the VCRs. This can be seen from the
// VCR figure above.
//
// 
// 
// "Op-amp" (self-biased NMOS inverter)
// ------------------------------------
//                  
//                  
//                        12V
//
//                         |
//              -----------|
//             |           |
//             |     ------|
//             |    |      |
//             |    |  ||--
//             |     --||
//             |       ||--
//         ||--            |
// vi -----||              |--------- vo
//         ||--            |   |
//             |       ||--    |
//             |-------||      |
//             |       ||--    |
//         ||--            |   |
//       --||              |   |
//      |  ||--            |   |
//      |      |           |   |
//      |       -----------|   |
//      |                  |   |
//      |                      |
//      |                 GND  |
//      |                      |
//       ----------------------
//
//
// vi  - input
// vo  - output
//
// Notes:
//
// The schematics above are laid out to show that the "op-amp" logically
// consists of two building blocks; a saturated load NMOS inverter (on the
// right hand side of the schematics) with a buffer / bias input stage
// consisting of a variable saturated load NMOS inverter (on the left hand
// side of the schematics).
//
// Provided a reasonably high input impedance and a reasonably low output
// impedance, the "op-amp" can be modeled as a voltage transfer function
// mapping input voltage to output voltage.
//
//
//
// Output buffer (NMOS voltage follower)
// -------------------------------------
//
//
//            12V
//
//             |
//             |
//         ||--
// vi -----||
//         ||--
//             |
//             |------ vo
//             |     (AUDIO
//            Rext    OUT)
//             |
//             |
//
//            GND
//
// vi   - input
// vo   - output
// Rext - external resistor, 1kOhm
//
// Notes:
//
// The external resistor Rext is needed to complete the NMOS voltage follower,
// this resistor has a recommended value of 1kOhm.
//
// Die photographs show that actually, two NMOS transistors are used in the
// voltage follower. However the two transistors are coupled in parallel (all
// terminals are pairwise common), which implies that we can model the two
// transistors as one.
//
// ----------------------------------------------------------------------------

// Compile-time computation of op-amp summer and mixer table offsets.

// The highpass summer has 2 - 6 inputs (bandpass, lowpass, and 0 - 4 voices).
template<int i>
struct summer_offset
{
  enum { value = summer_offset<i - 1>::value + ((2 + i - 1) << 16) };
};

template<>
struct summer_offset<0>
{
  enum { value = 0 };
};

// The mixer has 0 - 7 inputs (0 - 4 voices and 0 - 3 filter outputs).
template<int i>
struct mixer_offset
{
  enum { value = mixer_offset<i - 1>::value + ((i - 1) << 16) };
};

template<>
struct mixer_offset<1>
{
  enum { value = 1 };
};

template<>
struct mixer_offset<0>
{
  enum { value = 0 };
};


class Filter
{
public:
  Filter();

  void enable_filter(bool enable);
  void adjust_filter_bias(double dac_bias);
  void set_chip_model(chip_model model);
  void set_voice_mask(reg4 mask);

  void clock(int voice1, int voice2, int voice3);
  void clock(cycle_count delta_t, int voice1, int voice2, int voice3);
  void reset();

  // Write registers.
  void writeFC_LO(reg8);
  void writeFC_HI(reg8);
  void writeRES_FILT(reg8);
  void writeMODE_VOL(reg8);

  // SID audio input (16 bits).
  void input(short sample);

  // SID audio output (16 bits).
  short output();

protected:
  void set_sum_mix();
  void set_w0();
  void set_Q();

  // Filter enabled.
  bool enabled;

  // Filter cutoff frequency.
  reg12 fc;

  // Filter resonance.
  reg8 res;

  // Selects which voices to route through the filter.
  reg8 filt;

  // Selects which filter outputs to route into the mixer.
  reg4 mode;

  // Output master volume.
  reg4 vol;

  // Used to mask out EXT IN if not connected, and for test purposes
  // (voice muting).
  reg8 voice_mask;

  // Select which inputs to route into the summer / mixer.
  // These are derived from filt, mode, and voice_mask.
  reg8 sum;
  reg8 mix;

  // State of filter.
  int Vhp; // highpass
  int Vbp; // bandpass
  int Vbp_x, Vbp_vc;
  int Vlp; // lowpass
  int Vlp_x, Vlp_vc;
  // Filter / mixer inputs.
  int ve;
  int v3;
  int v2;
  int v1;

  // Cutoff frequency DAC voltage, resonance.
  int Vddt_Vw_2, Vw_bias;
  int _8_div_Q;
  // FIXME: Temporarily used for MOS 8580 emulation.
  int w0;
  int _1024_div_Q;

  chip_model sid_model;

   typedef struct {
    unsigned short vx;
    short dvx;
  } opamp_t;

  typedef struct {
    int vo_N16;  // Fixed point scaling for 16 bit op-amp output.
    int kVddt;   // K*(Vdd - Vth)
    int n_snake;
    int voice_scale_s14;
    int voice_DC;
    int ak;
    int bk;
    int vc_min;
    int vc_max;

    // Reverse op-amp transfer function.
    unsigned short opamp_rev[1 << 16];
    // Lookup tables for gain and summer op-amps in output stage / filter.
    unsigned short summer[summer_offset<5>::value];
    unsigned short gain[16][1 << 16];
    unsigned short mixer[mixer_offset<8>::value];
    // Cutoff frequency DAC output voltage table. FC is an 11 bit register.
    unsigned short f0_dac[1 << 11];
  } model_filter_t;

  int solve_gain(opamp_t* opamp, int n, int vi_t, int& x, model_filter_t& mf);
  int solve_integrate_6581(int dt, int vi_t, int& x, int& vc, model_filter_t& mf);

  // VCR - 6581 only.
  static unsigned short vcr_kVg[1 << 16];
  static unsigned short vcr_n_Ids_term[1 << 16];
  // Common parameters.
  static model_filter_t model_filter[2];

friend class SID;
};

} // namespace reSID

#endif // not RESID_FILTER_H
