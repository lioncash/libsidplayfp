/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2012-2015 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef MUS_H
#define MUS_H

#include <cstdint>
#include <memory>

#include "sidtune/SidTuneBase.h"

namespace libsidplayfp
{

class MUS final : public SidTuneBase
{
public:
    ~MUS() override = default;

    MUS(const MUS&) = delete;
    MUS& operator=(const MUS&) = delete;

    static std::unique_ptr<SidTuneBase> load(buffer_t& dataBuf, bool init = false);
    static std::unique_ptr<SidTuneBase> load(buffer_t& musBuf,
        buffer_t& strBuf,
        uint_least32_t fileOffset,
        bool init = false);

    void placeSidTuneInC64mem(sidmemory& mem) override;

protected:
    MUS() = default;

    void installPlayer(sidmemory& mem);

    void setPlayerAddress();

    void acceptSidTune(const char* dataFileName, const char* infoFileName,
                       buffer_t& buf, bool isSlashedFileName) override;

private:
    bool mergeParts(buffer_t& musBuf, buffer_t& strBuf);

    void tryLoad(buffer_t& musBuf,
        buffer_t& strBuf,
        uint_least32_t fileOffset,
        uint_least32_t voice3Index,
        bool init);

    /// Needed for MUS/STR player installation.
    uint_least16_t musDataLen = 0;
};

}

#endif // MUS_H
