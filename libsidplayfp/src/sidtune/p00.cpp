/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2015 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2000-2001 Simon White
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "sidtune/p00.h"

#include <cctype>
#include <cstdint>
#include <cstring>
#include <memory>

#include <sidplayfp/SidTuneInfo.h>

#include "sidtune/SidTuneTools.h"
#include "sidtune/SmartPtr.h"

namespace libsidplayfp
{
namespace
{
constexpr std::uint32_t X00_ID_LEN = 8;
constexpr std::uint32_t X00_NAME_LEN = 17;

enum class X00Format
{
    X00_DEL,
    X00_SEQ,
    X00_PRG,
    X00_USR,
    X00_REL
};

// Format strings
constexpr char TXT_FORMAT_DEL[] = "Unsupported tape image file (DEL)";
constexpr char TXT_FORMAT_SEQ[] = "Unsupported tape image file (SEQ)";
constexpr char TXT_FORMAT_PRG[] = "Tape image file (PRG)";
constexpr char TXT_FORMAT_USR[] = "Unsupported USR file (USR)";
constexpr char TXT_FORMAT_REL[] = "Unsupported tape image file (REL)";

// Magic field
constexpr char P00_ID[] = "C64File";
} // Anonymous namespace

// File format from PC64. PC64 automatically generates
// the filename from the cbm name (16 to 8 conversion)
// but we only need to worry about that when writing files
// should we want pc64 compatibility.  The extension numbers
// are just an index to try to avoid repeats.  Name conversion
// works by creating an initial filename from alphanumeric
// and ' ', '-' characters only with the later two being
// converted to '_'.  Then it parses the filename
// from end to start removing characters stopping as soon
// as the filename becomes <= 8.  The removal of characters
// occurs in three passes, the first removes all '_', then
// vowels and finally numerics.  If the filename is still
// greater than 8 it is truncated.

struct X00Header
{
    char    id[X00_ID_LEN];     // 'C64File' (ASCII)
    uint8_t name[X00_NAME_LEN]; // C64 name (PETSCII)
    uint8_t length;             // Rel files only (Bytes/Record),
                                // should be 0 for all other types
};

std::unique_ptr<SidTuneBase> p00::load(const char *fileName, const buffer_t& dataBuf)
{
    const std::string_view ext = SidTuneTools::fileExtOfPath(fileName);

    // Combined extension & magic field identification
    if (ext.size() != 4)
    {
        return nullptr;
    }

    if (!std::isdigit(ext[2]) || !std::isdigit(ext[3]))
    {
        return nullptr;
    }

    const char *format = nullptr;
    X00Format type;

    switch (std::toupper(ext[1]))
    {
    case 'D':
        type   = X00Format::X00_DEL;
        format = TXT_FORMAT_DEL;
        break;
    case 'S':
        type   = X00Format::X00_SEQ;
        format = TXT_FORMAT_SEQ;
        break;
    case 'P':
        type   = X00Format::X00_PRG;
        format = TXT_FORMAT_PRG;
        break;
    case 'U':
        type   = X00Format::X00_USR;
        format = TXT_FORMAT_USR;
        break;
    case 'R':
        type   = X00Format::X00_REL;
        format = TXT_FORMAT_REL;
        break;
    default:
        return nullptr;
    }

    // Verify the file is what we think it is
    const auto bufLen = dataBuf.size();
    if (bufLen < X00_ID_LEN)
        return nullptr;

    X00Header pHeader;
    memcpy(pHeader.id, &dataBuf[0], X00_ID_LEN);
    memcpy(pHeader.name, &dataBuf[X00_ID_LEN], X00_NAME_LEN);
    pHeader.length = dataBuf[X00_ID_LEN + X00_NAME_LEN];

    if (strcmp(pHeader.id, P00_ID))
        return nullptr;

    // File types current supported
    if (type != X00Format::X00_PRG)
        throw loadError("Not a PRG inside X00");

    if (bufLen < sizeof(X00Header) + 2)
        throw loadError(ERR_TRUNCATED);

    std::unique_ptr<p00> tune(new p00());
    tune->load(format, &pHeader);

    return tune;
}

void p00::load(const char* format, const X00Header* pHeader)
{
    info->m_formatString = format;

    {   // Decode file name
        SmartPtr_sidtt<const uint8_t> spPet(pHeader->name, X00_NAME_LEN);
        info->m_infoString.push_back(petsciiToAscii(spPet));
    }

    // Automatic settings
    fileOffset            = X00_ID_LEN + X00_NAME_LEN + 1;
    info->m_songs         = 1;
    info->m_startSong     = 1;
    info->m_compatibility = SidTuneInfo::Compatibility::BASIC;

    // Create the speed/clock setting table.
    convertOldStyleSpeedToTables(~0, info->m_clockSpeed);
}

}
