/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright (C) 2015-2019 Leandro Nini
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

#include <catch.hpp>

#include <sidplayfp/SidTune.h>
#include <sidplayfp/SidTuneInfo.h>
#include "../src/sidtune/MUS.h"

#include <array>
#include <cstdint>
#include <cstring>

namespace
{
#define BUFFERSIZE 26

#define LOADADDRESS_HI  1
#define LOADADDRESS_LO  0

#define VOICE1_LEN_HI   3
#define VOICE1_LEN_LO   2

constexpr std::array<std::uint8_t, BUFFERSIZE> bufferMUS{
    0x52, 0x53,             // load address
    0x04, 0x00,             // length of the data for Voice 1
    0x04, 0x00,             // length of the data for Voice 2
    0x04, 0x00,             // length of the data for Voice 3
    0x00, 0x00, 0x01, 0x4F, // data for Voice 1
    0x00, 0x00, 0x01, 0x4F, // data for Voice 2
    0x00, 0x01, 0x01, 0x4F, // data for Voice 3
    0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x00, // text description
};

struct TestFixture
{
    // Test setup
    TestFixture() = default;

    std::array<std::uint8_t, BUFFERSIZE> data{bufferMUS};
};
} // Anonymous namespace

TEST_CASE_METHOD(TestFixture, "TestPlayerAddress", "[mus]")
{
    SidTune tune(data.data(), BUFFERSIZE);

    REQUIRE(tune.getInfo()->initAddr() == 0xec60);
    REQUIRE(tune.getInfo()->playAddr() == 0xec80);
}

TEST_CASE_METHOD(TestFixture, "TestWrongVoiceLength", "[mus]")
{
    data[VOICE1_LEN_LO] = 0x76;

    SidTune tune(data.data(), BUFFERSIZE);
    CHECK(!tune.getStatus());

    REQUIRE(std::strcmp(tune.statusString(), "SIDTUNE ERROR: Could not determine file format") == 0);
}
