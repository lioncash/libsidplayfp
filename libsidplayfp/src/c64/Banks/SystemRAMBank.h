/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2012-2013 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2010 Antti Lankila
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

#ifndef SYSTEMRAMBANK_H
#define SYSTEMRAMBANK_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "Banks/Bank.h"

namespace libsidplayfp
{

/**
 * Area backed by RAM.
 *
 * @author Antti Lankila
 */
class SystemRAMBank final : public Bank
{
    friend class MMU;
public:
    /**
     * Initialize RAM with powerup pattern.
     */
    void reset()
    {
        ram.fill(0);
        for (std::size_t i = 0x40; i < ram.size(); i += 0x80)
        {
            std::memset(ram.data() + i, 0xff, 0x40);
        }
    }

    uint8_t peek(uint_least16_t address) override
    {
        return ram[address];
    }

    void poke(uint_least16_t address, uint8_t value) override
    {
        ram[address] = value;
    }

private:
    /// C64 RAM area
    std::array<uint8_t, 0x10000> ram;
};

}

#endif
