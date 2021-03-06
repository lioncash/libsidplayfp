/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright 2011-2015 Leandro Nini
 *  Copyright 2007-2010 Antti Lankila
 *  Copyright 2000 Simon White
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SIDINFOIMPL_H
#define SIDINFOIMPL_H

#include <cstdint>
#include <string>
#include <vector>

#include <sidplayfp/SidInfo.h>

#include "mixer.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifndef PACKAGE_NAME
#  define PACKAGE_NAME PACKAGE
#endif

#ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION VERSION
#endif

/**
 * The implementation of the SidInfo interface.
 */
class SidInfoImpl final : public SidInfo
{
public:
    SidInfoImpl()
    {
        m_credits.emplace_back(PACKAGE_NAME " V" PACKAGE_VERSION " Engine:\n"
            "\tCopyright (C) 2000 Simon White\n"
            "\tCopyright (C) 2007-2010 Antti Lankila\n"
            "\tCopyright (C) 2010-2015 Leandro Nini\n"
            "\t" PACKAGE_URL "\n");
    }

    SidInfoImpl(const SidInfoImpl&) = delete;
    SidInfoImpl& operator=(const SidInfoImpl&) = delete;

    const char* getName() const override { return m_name.c_str(); }
    const char* getVersion() const override { return m_version.c_str(); }

    std::size_t getNumberOfCredits() const override { return m_credits.size(); }
    const char* getCredits(std::size_t i) const override { return i < m_credits.size() ? m_credits[i].c_str() : ""; }

    unsigned int getMaxsids() const override { return libsidplayfp::Mixer::MAX_SIDS; }

    unsigned int getChannels() const override { return m_channels; }

    uint_least16_t getDriverAddr() const override { return m_driverAddr; }
    uint_least16_t getDriverLength() const override { return m_driverLength; }

    uint_least16_t getPowerOnDelay() const override { return m_powerOnDelay; }

    const char* getSpeedString() const override { return m_speedString.c_str(); }

    const char* getKernalDesc() const override { return m_kernalDesc.c_str(); }
    const char* getBasicDesc() const override { return m_basicDesc.c_str(); }
    const char* getChargenDesc() const override { return m_chargenDesc.c_str(); }

    const std::string m_name{PACKAGE_NAME};
    const std::string m_version{PACKAGE_VERSION};
    std::vector<std::string> m_credits;

    std::string m_speedString;

    std::string m_kernalDesc;
    std::string m_basicDesc;
    std::string m_chargenDesc;

    unsigned int m_channels = 1;

    uint_least16_t m_driverAddr = 0;
    uint_least16_t m_driverLength = 0;

    uint_least16_t m_powerOnDelay = 0;
};

#endif  /* SIDTUNEINFOIMPL_H */
