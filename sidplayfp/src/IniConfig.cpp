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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "IniConfig.h"

#include <string>

#ifndef _WIN32
#  include <sys/types.h>
#  include <sys/stat.h>  /* mkdir */
#  include <dirent.h>    /* opendir */
#else
#  include <windows.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "utils.h"
#include "ini/dataParser.h"

inline void debug(const TCHAR *msg, const TCHAR *val)
{
#ifndef NDEBUG
    SID_COUT << msg << val << std::endl;
#endif
}

inline void error(const TCHAR *msg)
{
    SID_CERR << msg << std::endl;
}

inline void error(const TCHAR *msg, const TCHAR *val)
{
    SID_CERR << msg << val << std::endl;
}

const TCHAR *DIR_NAME = TEXT("sidplayfp");
const TCHAR *FILE_NAME = TEXT("sidplayfp.ini");


IniConfig::IniConfig()
{   // Initialise everything else
    clear();
}


IniConfig::~IniConfig()
{
    clear();
}


void IniConfig::clear()
{
    sidplay2_s.version      = 1;           // INI File Version
    sidplay2_s.database.clear();
    sidplay2_s.playLength   = 0;           // INFINITE
    sidplay2_s.recordLength = 3 * 60 + 30; // 3.5 minutes
    sidplay2_s.kernalRom.clear();
    sidplay2_s.basicRom.clear();
    sidplay2_s.chargenRom.clear();

    console_s.ansi          = false;
    console_s.topLeft       = '+';
    console_s.topRight      = '+';
    console_s.bottomLeft    = '+';
    console_s.bottomRight   = '+';
    console_s.vertical      = '|';
    console_s.horizontal    = '-';
    console_s.junctionLeft  = '+';
    console_s.junctionRight = '+';

    audio_s.frequency = SidConfig::DEFAULT_SAMPLING_FREQ;
    audio_s.playback  = SidConfig::PlaybackMode::Mono;
    audio_s.precision = 16;

    emulation_s.modelDefault  = SidConfig::C64Model::PAL;
    emulation_s.modelForced   = false;
    emulation_s.sidModel      = SidConfig::SIDModel::MOS6581;
    emulation_s.forceModel    = false;
    emulation_s.ciaModel      = SidConfig::CIAModel::MOS6526;
    emulation_s.filter        = true;
    emulation_s.engine.clear();

    emulation_s.bias            = 0.0;
    emulation_s.filterCurve6581 = 0.0;
    emulation_s.filterCurve8580 = 0.0;
}


// static helpers

const TCHAR* readKey(iniHandler &ini, const TCHAR *key)
{
    const TCHAR* value = ini.getValue(key);
    if (value == nullptr)
    {   // Doesn't exist, add it
        ini.addValue(key, TEXT(""));
        debug(TEXT("Key doesn't exist: "), key);
    }
    else if (!value[0])
    {
        // Ignore empty values
        return nullptr;
    }

    return value;
}

void readDouble(iniHandler &ini, const TCHAR *key, double &result)
{
    const TCHAR* value = readKey(ini, key);
    if (value == nullptr)
        return;

    try
    {
        result = dataParser::parseDouble(value);
    }
    catch (const dataParser::parseError&)
    {
        error(TEXT("Error parsing double at "), key);
    }
}


void readInt(iniHandler &ini, const TCHAR *key, int &result)
{
    const TCHAR* value = readKey(ini, key);
    if (value == nullptr)
        return;

    try
    {
        result = dataParser::parseInt(value);
    }
    catch (const dataParser::parseError&)
    {
        error(TEXT("Error parsing int at "), key);
    }
}


void readBool(iniHandler &ini, const TCHAR *key, bool &result)
{
    const TCHAR* value = readKey(ini, key);
    if (value == nullptr)
        return;

    try
    {
        result = dataParser::parseBool(value);
    }
    catch (const dataParser::parseError&)
    {
        error(TEXT("Error parsing bool at "), key);
    }
}


SID_STRING readString(iniHandler &ini, const TCHAR *key)
{
    const TCHAR* value = ini.getValue(key);
    if (value == nullptr)
    {
        // Doesn't exist, add it
        ini.addValue(key, TEXT(""));
        debug(TEXT("Key doesn't exist: "), key);
        return SID_STRING();
    }

    return SID_STRING(value);
}


void readChar(iniHandler &ini, const TCHAR *key, char &ch)
{
    SID_STRING str = readString(ini, key);
    if (str.empty())
        return;

    TCHAR c = 0;

    // Check if we have an actual chanracter
    if (str[0] == '\'')
    {
        if (str[2] != '\'')
            return;
        else
            c = str[1];
    } // Nope is number
    else
    {
        try
        {
            c = dataParser::parseInt(str.c_str());
        }
        catch (const dataParser::parseError&)
        {
            error(TEXT("Error parsing int at "), key);
        }
    }

    // Clip off special characters
    if ((unsigned) c >= 32)
        ch = c;
}


bool readTime(iniHandler &ini, const TCHAR *key, int &value)
{
    SID_STRING str = readString(ini, key);
    if (str.empty())
        return false;

    int time;
    const size_t sep = str.find_first_of(':');
    try
    {
        if (sep == SID_STRING::npos)
        {   // User gave seconds
            time = dataParser::parseInt(str.c_str());
        }
        else
        {   // Read in MM:SS format
            const int min = dataParser::parseInt(str.substr(0, sep).c_str());
            if (min < 0 || min > 99)
                goto IniCofig_readTime_error;
            time = min * 60;

            const int sec  = dataParser::parseInt(str.substr(sep + 1).c_str());
            if (sec < 0 || sec > 59)
                goto IniCofig_readTime_error;
            time += sec;
        }
    }
    catch (const dataParser::parseError&)
    {
        error(TEXT("Error parsing time at "), key);
        return false;
    }

    value = time;
    return true;

IniCofig_readTime_error:
    error(TEXT("Invalid time at "), key);
    return false;
}


void IniConfig::readSidplay2(iniHandler &ini)
{
    if (!ini.setSection(TEXT("SIDPlayfp")))
        ini.addSection(TEXT("SIDPlayfp"));

    int version = sidplay2_s.version;
    readInt(ini, TEXT("Version"), version);
    if (version > 0)
        sidplay2_s.version = version;

    sidplay2_s.database = readString(ini, TEXT("Songlength Database"));

#if !defined _WIN32 && defined HAVE_UNISTD_H
    if (sidplay2_s.database.empty())
    {
        char buffer[PATH_MAX];
        snprintf(buffer, PATH_MAX, "%sSonglengths.txt", PKGDATADIR);
        if (::access(buffer, R_OK) == 0)
            sidplay2_s.database.assign(buffer);
    }
#endif

    int time;
    if (readTime(ini, TEXT("Default Play Length"), time))
        sidplay2_s.playLength = time;
    if (readTime(ini, TEXT("Default Record Length"), time))
        sidplay2_s.recordLength = time;

    sidplay2_s.kernalRom = readString(ini, TEXT("Kernal Rom"));
    sidplay2_s.basicRom = readString(ini, TEXT("Basic Rom"));
    sidplay2_s.chargenRom = readString(ini, TEXT("Chargen Rom"));
}


void IniConfig::readConsole(iniHandler &ini)
{
    if (!ini.setSection (TEXT("Console")))
        ini.addSection(TEXT("Console"));

    readBool(ini, TEXT("Ansi"),                console_s.ansi);
    readChar(ini, TEXT("Char Top Left"),       console_s.topLeft);
    readChar(ini, TEXT("Char Top Right"),      console_s.topRight);
    readChar(ini, TEXT("Char Bottom Left"),    console_s.bottomLeft);
    readChar(ini, TEXT("Char Bottom Right"),   console_s.bottomRight);
    readChar(ini, TEXT("Char Vertical"),       console_s.vertical);
    readChar(ini, TEXT("Char Horizontal"),     console_s.horizontal);
    readChar(ini, TEXT("Char Junction Left"),  console_s.junctionLeft);
    readChar(ini, TEXT("Char Junction Right"), console_s.junctionRight);
}


void IniConfig::readAudio(iniHandler &ini)
{
    if (!ini.setSection (TEXT("Audio")))
        ini.addSection(TEXT("Audio"));

    {
        int frequency = (int) audio_s.frequency;
        readInt(ini, TEXT("Frequency"), frequency);
        audio_s.frequency = (unsigned long) frequency;
    }

    {
        int channels = 0;
        readInt(ini, TEXT("Channels"),  channels);
        if (channels)
        {
            audio_s.playback = (channels == 1) ? SidConfig::PlaybackMode::Mono : SidConfig::PlaybackMode::Stereo;
        }
    }

    readInt(ini, TEXT("BitsPerSample"), audio_s.precision);
}


void IniConfig::readEmulation(iniHandler &ini)
{
    if (!ini.setSection (TEXT("Emulation")))
        ini.addSection(TEXT("Emulation"));

    emulation_s.engine = readString (ini, TEXT("Engine"));

    {
        SID_STRING str = readString (ini, TEXT("C64Model"));
        if (!str.empty())
        {
            if (str.compare(TEXT("PAL")) == 0)
                emulation_s.modelDefault = SidConfig::C64Model::PAL;
            else if (str.compare(TEXT("NTSC")) == 0)
                emulation_s.modelDefault = SidConfig::C64Model::NTSC;
            else if (str.compare(TEXT("OLD_NTSC")) == 0)
                emulation_s.modelDefault = SidConfig::C64Model::OldNTSC;
            else if (str.compare(TEXT("DREAN")) == 0)
                emulation_s.modelDefault = SidConfig::C64Model::DREAN;
        }
    }

    readBool(ini, TEXT("ForceC64Model"), emulation_s.modelForced);
    readBool(ini, TEXT("DigiBoost"), emulation_s.digiboost);

    {
        SID_STRING str = readString(ini, TEXT("CiaModel"));
        if (!str.empty())
        {
            if (str.compare(TEXT("MOS6526")) == 0)
                emulation_s.ciaModel = SidConfig::CIAModel::MOS6526;
            else if (str.compare(TEXT("MOS8521")) == 0)
                emulation_s.ciaModel = SidConfig::CIAModel::MOS8521;
        }
    }

    {
        SID_STRING str = readString(ini, TEXT("SidModel"));
        if (!str.empty())
        {
            if (str.compare(TEXT("MOS6581")) == 0)
                emulation_s.sidModel = SidConfig::SIDModel::MOS6581;
            else if (str.compare(TEXT("MOS8580")) == 0)
                emulation_s.sidModel = SidConfig::SIDModel::MOS8580;
        }
    }

    readBool(ini, TEXT("ForceSidModel"), emulation_s.forceModel);

    readBool(ini, TEXT("UseFilter"), emulation_s.filter);

    readDouble(ini, TEXT("FilterBias"), emulation_s.bias);
    readDouble(ini, TEXT("FilterCurve6581"), emulation_s.filterCurve6581);
    readDouble(ini, TEXT("FilterCurve8580"), emulation_s.filterCurve8580);
}

class iniError
{
private:
    const SID_STRING msg;

public:
    iniError(const TCHAR* msg) : msg(msg) {}
    const SID_STRING message() const { return msg; }
};

SID_STRING getConfigPath()
{
    SID_STRING configPath;

    try
    {
        configPath = utils::getConfigPath();
    }
    catch (const utils::error&)
    {
        throw iniError(TEXT("Cannot get config path!"));
    }

    debug(TEXT("Config path: "), configPath.c_str());

    configPath.append(SEPARATOR).append(DIR_NAME);

    // Make sure the config path exists
#ifndef _WIN32
    if (!opendir(configPath.c_str()))
    {
        if (mkdir(configPath.c_str(), 0755) < 0)
        {
            throw iniError(strerror(errno));
        }
    }
#else
    if (GetFileAttributes(configPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        if (CreateDirectory(configPath.c_str(), nullptr) == 0)
        {
            LPTSTR pBuffer;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pBuffer, 0, nullptr);
            iniError err(pBuffer);
            LocalFree(pBuffer);
            throw err;
        }
    }
#endif

    configPath.append(SEPARATOR).append(FILE_NAME);

    debug(TEXT("Config file: "), configPath.c_str());

    return configPath;
}

bool tryOpen(iniHandler &ini)
{
#ifdef _WIN32
    {
        // Try exec dir first
        SID_STRING execPath(utils::getExecPath());
        execPath.append(SEPARATOR).append(FILE_NAME);
        if (ini.open(execPath.c_str()))
            return true;
    }
#endif
    return false;
}

void IniConfig::read()
{
    clear();

    iniHandler ini;

    if (!tryOpen(ini))
    {
        try
        {
            SID_STRING configPath = getConfigPath();

            // Opens an existing file or creates a new one
            if (!ini.open(configPath.c_str()))
            {
                error(TEXT("Error reading config file!"));
                return;
            }
        } catch (iniError const &e) {
            error(e.message().c_str());
            return;
        }
    }

    readSidplay2  (ini);
    readConsole   (ini);
    readAudio     (ini);
    readEmulation (ini);
    ini.close();
}
