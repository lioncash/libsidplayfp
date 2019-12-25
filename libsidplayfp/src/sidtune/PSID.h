/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2012-2014 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef PSID_H
#define PSID_H

#include <memory>

#include <sidplayfp/SidTune.h>
#include "sidtune/SidTuneBase.h"

namespace libsidplayfp
{

struct psidHeader;

class PSID final : public SidTuneBase
{
public:
    ~PSID() override = default;

    PSID(const PSID&) = delete;
    PSID& operator=(const PSID&) = delete;

    /**
     * @return pointer to a SidTune or 0 if not a PSID file
     * @throw loadError if PSID file is corrupt
     */
    static std::unique_ptr<SidTuneBase> load(const buffer_t& dataBuf);

    const char* createMD5(char* md5) override;

    const char* createMD5New(char* md5) override;

protected:
    PSID() {}

private:
    /**
     * Load PSID file.
     *
     * @throw loadError
     */
    void tryLoad(const psidHeader& pHeader);

    /**
     * Read PSID file header.
     *
     * @throw loadError
     */
    static void readHeader(const buffer_t& dataBuf, psidHeader& hdr);

    char m_md5[SidTune::MD5_LENGTH+1];
};

}

#endif // PSID_H
