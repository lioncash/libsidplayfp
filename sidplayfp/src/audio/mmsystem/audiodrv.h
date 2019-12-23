/*
 * This file is part of sidplayfp, a console SID player.
 *
 * Copyright 2000 Jarno Paananen
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

#ifndef AUDIO_MMSYSTEM_H
#define AUDIO_MMSYSTEM_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_MMSYSTEM_H

#define HAVE_MMSYSTEM

#ifndef AudioDriver
#  define AudioDriver Audio_MMSystem
#endif

#include <array>
#include <cstdint>
#include <windows.h>
#include <mmsystem.h>

#include "../AudioBase.h"

class Audio_MMSystem final : public AudioBase
{
public:
    Audio_MMSystem();
    ~Audio_MMSystem() override;

    bool open(AudioConfig& cfg) override;
    void close() override;
    void reset() override;
    bool write() override;
    void pause() override {}

private:
    static const char* getErrorMessage(MMRESULT err);
    static void checkResult(MMRESULT err);

    HWAVEOUT  waveHandle{};

    // Rev 1.3 (saw) - Buffer sizes adjusted to get a
    // correct playtimes
    static constexpr std::uint32_t MAXBUFBLOCKS = 3;
    std::array<short*, MAXBUFBLOCKS> blocks{};
    std::array<HGLOBAL, MAXBUFBLOCKS> blockHandles{};
    std::array<WAVEHDR*, MAXBUFBLOCKS> blockHeaders{};
    std::array<HGLOBAL, MAXBUFBLOCKS>  blockHeaderHandles{};
    std::uint32_t blockNum = 0;
    std::uint32_t bufSize = 0;
    bool          isOpen = false;
};

#endif // HAVE_MMSYSTEM_H
#endif // AUDIO_MMSYSTEM_H
