/*
 * This file is part of sidplayfp, a console SID player.
 *
 * Copyright 2011-2019 Leandro Nini
 * Copyright 2000-2001 Simon White
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

#include "player.h"

#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include <sidplayfp/SidInfo.h>
#include <sidplayfp/SidTuneInfo.h>

const char SID6581[] = "MOS6581";
const char SID8580[] = "CSG8580";

const char* getModel(SidTuneInfo::Model model)
{
    switch (model)
    {
    default:
    case SidTuneInfo::Model::Unknown:
        return "UNKNOWN";
    case SidTuneInfo::Model::SID6581:
        return SID6581;
    case SidTuneInfo::Model::SID8580:
        return SID8580;
    case SidTuneInfo::Model::Any:
        return "ANY";
    }
}

const char* getModel(SidConfig::SIDModel model)
{
    switch (model)
    {
    default:
    case SidConfig::SIDModel::MOS6581:
        return SID6581;
    case SidConfig::SIDModel::MOS8580:
        return SID8580;
    }
}

const char* getClock(SidTuneInfo::Clock clock)
{
    switch (clock)
    {
    default:
    case SidTuneInfo::Clock::Unknown:
        return "UNKNOWN";
    case SidTuneInfo::Clock::PAL:
        return "PAL";
    case SidTuneInfo::Clock::NTSC:
        return "NTSC";
    case SidTuneInfo::Clock::Any:
        return "ANY";
    }
}


// Display console menu
void ConsolePlayer::menu()
{
    if (m_quietLevel > 1)
        return;

    const SidInfo &info         = m_engine.info();
    const SidTuneInfo *tuneInfo = m_tune.getInfo();

    // cerr << (char) 12 << '\f'; // New Page
    if ((m_iniCfg.console ()).ansi)
    {
        std::cerr << '\x1b' << "[40m";  // Background black
        std::cerr << '\x1b' << "[2J";   // Clear screen
        std::cerr << '\x1b' << "[0;0H"; // Move cursor to 0,0
    }

    consoleTable(tableStart);
    consoleTable(tableMiddle);
    consoleColour(PlayerColor::Red, true);
    std::cerr << "  SID";
    consoleColour(PlayerColor::Blue, true);
    std::cerr << "PLAYFP";
    consoleColour(PlayerColor::White, true);
    std::cerr << " - Music Player and C64 SID Chip Emulator" << std::endl;
    consoleTable(tableMiddle);
    consoleColour(PlayerColor::White, false);
    {
        std::string version;
        version.reserve(54);
        version.append("Sidplayfp V" VERSION ", ").append(1, std::toupper(*info.name())).append(info.name() + 1).append(" V").append(info.version());
        std::cerr << std::setw(54/2 + version.length()/2) << version << std::endl;
    }

    const std::size_t n = tuneInfo->numberOfInfoStrings();
    if (n != 0)
    {
        consoleTable(tableSeparator);

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Cyan, true);
        std::cerr << " Title        : ";
        consoleColour(PlayerColor::Magenta, true);
        std::cerr << tuneInfo->infoString(0) << std::endl;
        if (n > 1)
        {
            consoleTable(tableMiddle);
            consoleColour(PlayerColor::Cyan, true);
            std::cerr << " Author       : ";
            consoleColour(PlayerColor::Magenta, true);
            std::cerr << tuneInfo->infoString(1) << std::endl;
            consoleTable(tableMiddle);
            consoleColour(PlayerColor::Cyan, true);
            std::cerr << " Released     : ";
            consoleColour(PlayerColor::Magenta, true);
            std::cerr << tuneInfo->infoString(2) << std::endl;
        }
    }

    for (std::size_t i = 0; i < tuneInfo->numberOfCommentStrings(); i++)
    {
        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Cyan, true);
        std::cerr << " Comment      : ";
        consoleColour(PlayerColor::Magenta, true);
        std::cerr << tuneInfo->commentString(i) << std::endl;
    }

    consoleTable(tableSeparator);

    if (m_verboseLevel)
    {
        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Green, true);
        std::cerr << " File format  : ";
        consoleColour(PlayerColor::White, true);
        std::cerr << tuneInfo->formatString() << std::endl;
        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Green, true);
        std::cerr << " Filename(s)  : ";
        consoleColour(PlayerColor::White, true);
        std::cerr << tuneInfo->dataFileName() << std::endl;
        // Second file is only sometimes present
        if (tuneInfo->infoFileName())
        {
            consoleTable(tableMiddle);
            consoleColour(PlayerColor::Green, true);
            std::cerr << "              : ";
            consoleColour(PlayerColor::White, true);
            std::cerr << tuneInfo->infoFileName() << std::endl;
        }
        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Green, true);
        std::cerr << " Condition    : ";
        consoleColour(PlayerColor::White, true);
        std::cerr << m_tune.statusString() << std::endl;

#if HAVE_TSID == 1
        if (!m_tsid)
        {
            consoleTable(tableMiddle);
            consoleColour(PlayerColor::Green, true);
            std::cerr << " TSID Error   : ";
            consoleColour(PlayerColor::White, true);
            std::cerr << m_tsid.getError () << std::endl;
        }
#endif // HAVE_TSID
    }

    consoleTable(tableMiddle);
    consoleColour(PlayerColor::Green, true);
    std::cerr << " Playlist     : ";
    consoleColour(PlayerColor::White, true);

    {   // This will be the format used for playlists
        int i = 1;
        if (!m_track.single)
        {
            i  = m_track.selected;
            i -= (m_track.first - 1);
            if (i < 1)
                i += m_track.songs;
        }
        std::cerr << i << '/' << m_track.songs;
        std::cerr << " (tune " << tuneInfo->currentSong() << '/'
                  << tuneInfo->songs() << '['
                  << tuneInfo->startSong() << "])";
    }

    if (m_track.loop)
        std::cerr << " [LOOPING]";
    std::cerr << std::endl;

    if (m_verboseLevel)
    {
        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Green, true);
        std::cerr << " Song Speed   : ";
        consoleColour(PlayerColor::White, true);
        std::cerr << getClock(tuneInfo->clockSpeed()) << std::endl;
    }

    consoleTable(tableMiddle);
    consoleColour(PlayerColor::Green, true);
    std::cerr << " Song Length  : ";
    consoleColour(PlayerColor::White, true);
    if (m_timer.stop)
    {
        const uint_least32_t seconds = m_timer.stop / 1000;
        std::cerr << std::setw(2) << std::setfill('0') << ((seconds / 60) % 100)
                  << ':' << std::setw(2) << std::setfill('0') << (seconds % 60);
    }
    else if (m_timer.valid)
    {
        std::cerr << "FOREVER";
    }
    else
    {
        std::cerr << "UNKNOWN";
    }
    if (m_timer.start)
    {   // Show offset
        const uint_least32_t seconds = m_timer.start / 1000;
        std::cerr << " (+" << std::setw(2) << std::setfill('0') << ((seconds / 60) % 100)
                  << ':' << std::setw(2) << std::setfill('0') << (seconds % 60) << ")";
    }
    std::cerr << std::endl;

    if (m_verboseLevel)
    {
        consoleTable(tableSeparator);

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << " Addresses    : " << std::hex;
        std::cerr.setf(std::ios::uppercase);
        consoleColour(PlayerColor::White, false);
        // Display PSID Driver location
        std::cerr << "DRIVER = ";
        if (info.driverAddr() == 0)
        {
            std::cerr << "NOT PRESENT";
        }
        else
        {
            std::cerr << "$"  << std::setw(4) << std::setfill('0') << info.driverAddr();
            std::cerr << "-$" << std::setw(4) << std::setfill('0') << info.driverAddr() +
                (info.driverLength() - 1);
        }
        if (tuneInfo->playAddr() == 0xffff)
            std::cerr << ", SYS = $" << std::setw(4) << std::setfill('0') << tuneInfo->initAddr();
        else
            std::cerr << ", INIT = $" << std::setw(4) << std::setfill('0') << tuneInfo->initAddr();
        std::cerr << std::endl;
        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << "              : ";
        consoleColour(PlayerColor::White, false);
        std::cerr << "LOAD   = $" << std::setw(4) << std::setfill('0') << tuneInfo->loadAddr();
        std::cerr << "-$"         << std::setw(4) << std::setfill('0') << tuneInfo->loadAddr() +
            (tuneInfo->c64dataLen() - 1);
        if (tuneInfo->playAddr() != 0xffff)
            std::cerr << ", PLAY = $" << std::setw(4) << std::setfill('0') << tuneInfo->playAddr();
        std::cerr << std::dec << std::endl;
        std::cerr.unsetf(std::ios::uppercase);

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << " SID Details  : ";
        consoleColour(PlayerColor::White, false);
        std::cerr << "Model = ";
#if LIBSIDPLAYFP_VERSION_MAJ > 1 || LIBSIDPLAYFP_VERSION_MIN >= 8
        std::cerr << getModel(tuneInfo->sidModel(0));
#else
        std::cerr << getModel(tuneInfo->sidModel1());
#endif
        std::cerr << std::endl;
#if LIBSIDPLAYFP_VERSION_MAJ > 1 || LIBSIDPLAYFP_VERSION_MIN >= 8
        if (tuneInfo->sidChips() > 1)
#else
        if (tuneInfo->isStereo())
#endif
        {
            consoleTable(tableMiddle);
            consoleColour(PlayerColor::Yellow, true);
            std::cerr << "              : ";
            consoleColour(PlayerColor::White, false);
#if LIBSIDPLAYFP_VERSION_MAJ > 1 || LIBSIDPLAYFP_VERSION_MIN >= 8
            std::cerr << "2nd SID = $" << std::hex << tuneInfo->sidChipBase(1) << std::dec;
            std::cerr << ", Model = " << getModel(tuneInfo->sidModel(1));
#else
            std::cerr << "2nd SID = $" << std::hex << tuneInfo->sidChipBase2() << std::dec;
            std::cerr << ", Model = " << getModel(tuneInfo->sidModel2());
#endif
            std::cerr << std::endl;
#if LIBSIDPLAYFP_VERSION_MAJ > 1 || LIBSIDPLAYFP_VERSION_MIN >= 8
            if (tuneInfo->sidChips() > 2)
            {
                consoleTable(tableMiddle);
                consoleColour(PlayerColor::Yellow, true);
                std::cerr << "              : ";
                consoleColour(PlayerColor::White, false);
                std::cerr << "3rd SID = $" << std::hex << tuneInfo->sidChipBase(2) << std::dec;
                std::cerr << ", Model = " << getModel(tuneInfo->sidModel(2));
                std::cerr << std::endl;
            }
#endif
        }

        consoleTable(tableSeparator);

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << " Play speed   : ";
        consoleColour(PlayerColor::White, false);
        std::cerr << info.speedString() << std::endl;

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << " Play mode    : ";
        consoleColour(PlayerColor::White, false);
        std::cerr << (info.channels() == 1 ? "Mono" : "Stereo") << std::endl;

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << " SID Filter   : ";
        consoleColour(PlayerColor::White, false);
        std::cerr << (m_filter.enabled ? "Yes" : "No") << std::endl;

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << " DigiBoost    : ";
        consoleColour(PlayerColor::White, false);
        std::cerr << (m_engCfg.digiBoost ? "Yes" : "No") << std::endl;

        consoleTable(tableMiddle);
        consoleColour(PlayerColor::Yellow, true);
        std::cerr << " SID Model    : ";
        consoleColour(PlayerColor::White, false);
        if (m_engCfg.forceSidModel)
            std::cerr << "Forced ";
        else
            std::cerr << "from tune, default = ";
        std::cerr << getModel(m_engCfg.defaultSidModel) << std::endl;

        if (m_verboseLevel > 1)
        {
            consoleTable(tableMiddle);
            consoleColour(PlayerColor::Yellow, true);
            std::cerr << " Delay        : ";
            consoleColour(PlayerColor::White, false);
            std::cerr << info.powerOnDelay() << " (cycles at poweron)" << std::endl;
        }
    }

    const char* romDesc = info.kernalDesc();

    consoleTable(tableSeparator);

    consoleTable(tableMiddle);
    consoleColour(PlayerColor::Magenta, true);
    std::cerr << " Kernal ROM   : ";
    if (std::strlen(romDesc) == 0)
    {
        consoleColour(PlayerColor::Red, false);
        std::cerr << "None - Some tunes may not play!";
    }
    else
    {
        consoleColour(PlayerColor::White, false);
        std::cerr << romDesc;
    }
    std::cerr << std::endl;

    romDesc = info.basicDesc();

    consoleTable(tableMiddle);
    consoleColour(PlayerColor::Magenta, true);
    std::cerr << " BASIC ROM    : ";
    if (std::strlen(romDesc) == 0)
    {
        consoleColour(PlayerColor::Red, false);
        std::cerr << "None - Basic tunes will not play!";
    }
    else
    {
        consoleColour(PlayerColor::White, false);
        std::cerr << romDesc;
    }
    std::cerr << std::endl;

    romDesc = info.chargenDesc();

    consoleTable(tableMiddle);
    consoleColour(PlayerColor::Magenta, true);
    std::cerr << " Chargen ROM  : ";
    if (std::strlen(romDesc) == 0)
    {
        consoleColour(PlayerColor::Red, false);
        std::cerr << "None";
    }
    else
    {
        consoleColour(PlayerColor::White, false);
        std::cerr << romDesc;
    }
    std::cerr << std::endl;

    consoleTable(tableEnd);

    if (m_driver.file)
        std::cerr << "Creating audio file, please wait...";
    else
        std::cerr << "Playing, press ESC to stop...";

    // Get all the text to the screen so music playback
    // is not disturbed.
    if (!m_quietLevel)
        std::cerr << "00:00";
    std::cerr << std::flush;
}

// Set colour of text on console
void ConsolePlayer::consoleColour(PlayerColor colour, bool bold)
{
    if (m_iniCfg.console().ansi)
    {
        const char *mode = "";

        switch (colour)
        {
        case PlayerColor::Black:   mode = "30"; break;
        case PlayerColor::Red:     mode = "31"; break;
        case PlayerColor::Green:   mode = "32"; break;
        case PlayerColor::Yellow:  mode = "33"; break;
        case PlayerColor::Blue:    mode = "34"; break;
        case PlayerColor::Magenta: mode = "35"; break;
        case PlayerColor::Cyan:    mode = "36"; break;
        case PlayerColor::White:   mode = "37"; break;
        }

        if (bold)
            std::cerr << '\x1b' << "[1;40;" << mode << 'm';
        else
            std::cerr << '\x1b' << "[0;40;" << mode << 'm';
    }
}

// Display menu outline
void ConsolePlayer::consoleTable (player_table_t table)
{
    const unsigned int tableWidth = 54;

    consoleColour(PlayerColor::White, true);
    switch (table)
    {
    case tableStart:
        std::cerr << m_iniCfg.console().topLeft << std::setw(tableWidth)
                  << std::setfill(m_iniCfg.console().horizontal) << ""
                  << m_iniCfg.console().topRight;
        break;

    case tableMiddle:
        std::cerr << std::setw(tableWidth + 1) << std::setfill(' ') << ""
                  << m_iniCfg.console().vertical << '\r'
                  << m_iniCfg.console ().vertical;
        return;

    case tableSeparator:
        std::cerr << m_iniCfg.console().junctionRight << std::setw(tableWidth)
                  << std::setfill(m_iniCfg.console().horizontal) << ""
                  << m_iniCfg.console().junctionLeft;
        break;

    case tableEnd:
        std::cerr << m_iniCfg.console().bottomLeft << std::setw(tableWidth)
                  << std::setfill(m_iniCfg.console().horizontal) << ""
                  << m_iniCfg.console().bottomRight;
        break;
    }

    // Move back to beginning of row and skip first char
    std::cerr << '\n';
}


// Restore Ansi Console to defaults
void ConsolePlayer::consoleRestore ()
{
    if (m_iniCfg.console().ansi)
        std::cerr << '\x1b' << "[0m";
}
