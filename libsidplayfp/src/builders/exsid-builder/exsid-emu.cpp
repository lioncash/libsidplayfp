/***************************************************************************
             exsid.cpp  -  exSID support interface.
                             -------------------
   Based on hardsid.cpp (C) 2001 Jarno Paananen

    copyright            : (C) 2015-2017 Thibaut VARENE
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/

#include "exsid-emu.h"

#include <cstdio>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>

#ifdef HAVE_EXSID
#  include <exSID.h>
#else
#  include "driver/exSID.h"
#endif

namespace libsidplayfp
{

unsigned int exSID::sid = 0;

const char* exSID::getCredits()
{
    static std::string credits;

    if (credits.empty())
    {
        // Setup credits
        std::ostringstream ss;
        ss << "exSID V" << VERSION << " Engine:\n";
        ss << "\t(C) 2015-2017 Thibaut VARENE\n";
        credits = ss.str();
    }

	return credits.c_str();
}

exSID::exSID(sidbuilder *builder) :
    sidemu(builder),
    m_status(false),
    readflag(false),
    busValue(0)
{
    if (exSID_init() < 0)
    {
        //FIXME should get error message from the driver
        m_error = exSID_error_str();
        return;
    }

    m_status = true;
    sid++;
    sidemu::reset();

    muted[0] = muted[1] = muted[2] = false;
}

exSID::~exSID()
{
    sid--;
    if (m_status)
        exSID_audio_op(XS_AU_MUTE);
    exSID_exit();
}

void exSID::reset(uint8_t volume)
{
    exSID_reset(volume);
    m_accessClk = 0;
    readflag = false;
}

unsigned int exSID::delay()
{
    event_clock_t cycles = eventScheduler->getTime(m_accessClk, EventPhase::ClockPHI1);
    m_accessClk += cycles;

    while (cycles > 0xffff)
    {
        exSID_delay(0xffff);
        cycles -= 0xffff;
    }

    return static_cast<unsigned int>(cycles);
}

void exSID::clock()
{
    const unsigned int cycles = delay();

    if (cycles)
        exSID_delay(cycles);
}

uint8_t exSID::read(uint_least8_t addr)
{
    if ((addr < 0x19) || (addr > 0x1C))
    {
        return busValue;
    }

    if (!readflag)
    {
#ifdef DEBUG
        printf("WARNING: Read support is limited. This file may not play correctly!\n");
#endif
        readflag = true;

        // Here we implement the "safe" detection routine return values
        if (0x1b == addr) {	// we could implement a commandline-chosen return byte here
            return (SidConfig::MOS8580 == runmodel) ? 0x02 : 0x03;
        }
    }

    const unsigned int cycles = delay();

    busValue = exSID_clkdread(cycles, addr);	// busValue is updated on valid reads
    return busValue;
}

void exSID::write(uint_least8_t addr, uint8_t data)
{
    busValue = data;

    if (addr > 0x18)
        return;

    const unsigned int cycles = delay();

    if (addr % 7 == 4 && muted[addr / 7])
        data = 0;

    exSID_clkdwrite(cycles, addr, data);
}

void exSID::voice(unsigned int num, bool mute)
{
    muted[num] = mute;
}

void exSID::model(SidConfig::SIDModel model, bool digiboost)
{
    runmodel = model;
    // currently no support for stereo mode: output the selected SID to both L and R channels
    exSID_audio_op(model == SidConfig::SIDModel::MOS8580 ? XS_AU_8580_8580 : XS_AU_6581_6581);	// mutes output
    //exSID_audio_op(XS_AU_UNMUTE);	// sampling is set after model, no need to unmute here and cause pops
    exSID_chipselect(model == SidConfig::SIDModel::MOS8580 ? XS_CS_CHIP1 : XS_CS_CHIP0);
}

void exSID::flush() {}

bool exSID::lock(EventScheduler* env)
{
    return sidemu::lock(env);
}

void exSID::unlock()
{
    sidemu::unlock();
}

void exSID::sampling(float systemclock, float freq,
                     [[maybe_unused]] SidConfig::SamplingMethod method, [[maybe_unused]] bool fast)
{
    exSID_audio_op(XS_AU_MUTE);
    if (systemclock < 1000000.0F)
        exSID_clockselect(XS_CL_PAL);
    else
        exSID_clockselect(XS_CL_NTSC);
    exSID_audio_op(XS_AU_UNMUTE);
}

}
