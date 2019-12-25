 /*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2012-2015 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include <sidplayfp/SidTune.h>

#include <array>
#include "sidtune/SidTuneBase.h"

using namespace libsidplayfp;

namespace
{
constexpr char MSG_NO_ERRORS[] = "No errors";

// Default sidtune file name extensions. This selection can be overriden
// by specifying a custom list in the constructor.
constexpr std::array defaultFileNameExt{
    // Preferred default file extension for single-file sidtunes
    // or sidtune description files in SIDPLAY INFOFILE format.
    ".sid", ".SID",
    // File extensions used (and created) by various C64 emulators and
    // related utilities. These extensions are recommended to be used as
    // a replacement for ".dat" in conjunction with two-file sidtunes.
    ".c64", ".prg", ".p00", ".C64", ".PRG", ".P00",
    // Stereo Sidplayer (.mus/.MUS ought not be included because
    // these must be loaded first; it sometimes contains the first
    // credit lines of a MUS/STR pair).
    ".str", ".STR", ".mus", ".MUS",
    // End.
    static_cast<const char*>(nullptr),
};
} // Anonymous namespace

const char* const* SidTune::fileNameExtensions = defaultFileNameExt.data();

SidTune::SidTune(const char* fileName, const char* const* fileNameExt, bool separatorIsSlash)
    : m_statusString{MSG_NO_ERRORS}
{
    setFileNameExtensions(fileNameExt);
    load(fileName, separatorIsSlash);
}

SidTune::SidTune(const uint_least8_t* oneFileFormatSidtune, uint_least32_t sidtuneLength)
    : m_statusString{MSG_NO_ERRORS}
{
    read(oneFileFormatSidtune, sidtuneLength);
}

SidTune::~SidTune() = default;

void SidTune::setFileNameExtensions(const char* const* fileNameExt)
{
    fileNameExtensions = (fileNameExt != nullptr) ? fileNameExt : defaultFileNameExt.data();
}

void SidTune::load(const char* fileName, bool separatorIsSlash)
{
    try
    {
        tune = SidTuneBase::load(fileName, fileNameExtensions, separatorIsSlash);
        m_status = true;
        m_statusString = MSG_NO_ERRORS;
    }
    catch (loadError const &e)
    {
        m_status = false;
        m_statusString = e.message();
    }
}

void SidTune::read(const uint_least8_t* sourceBuffer, uint_least32_t bufferLen)
{
    try
    {
        tune = SidTuneBase::read(sourceBuffer, bufferLen);
        m_status = true;
        m_statusString = MSG_NO_ERRORS;
    }
    catch (loadError const &e)
    {
        m_status = false;
        m_statusString = e.message();
    }
}

unsigned int SidTune::selectSong(unsigned int songNum)
{
    return tune != nullptr ? tune->selectSong(songNum) : 0;
}

const SidTuneInfo* SidTune::getInfo() const
{
    return tune != nullptr ? tune->getInfo() : nullptr;
}

const SidTuneInfo* SidTune::getInfo(unsigned int songNum)
{
    return tune != nullptr ? tune->getInfo(songNum) : nullptr;
}

bool SidTune::getStatus() const { return m_status; }

const char* SidTune::statusString() const { return m_statusString; }

bool SidTune::placeSidTuneInC64mem(sidmemory& mem)
{
    if (tune == nullptr)
        return false;

    tune->placeSidTuneInC64mem(mem);
    return true;
}

const char* SidTune::createMD5(char *md5)
{
    return tune != nullptr ? tune->createMD5(md5) : nullptr;
}

const char* SidTune::createMD5New(char *md5)
{
    return tune != nullptr ? tune->createMD5New(md5) : nullptr;
}

const uint_least8_t* SidTune::c64Data() const
{
    return tune != nullptr ? tune->c64Data() : nullptr;
}
