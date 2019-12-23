/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 *  Copyright (C) 2014-2017 Leandro Nini
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
#include "../src/builders/residfp-builder/residfp/WaveformCalculator.h"

#define private public

#include "../src/builders/residfp-builder/residfp/WaveformGenerator.h"

TEST_CASE("Test Clock Shift Register", "[waveform-generator]")
{
    reSIDfp::WaveformGenerator generator;
    generator.reset();

    generator.shift_register = 0x35555e;
    generator.clock_shift_register(0);

    REQUIRE(generator.noise_output == 2528);
}

TEST_CASE("Test Noise Output", "[waveform-generator]")
{
    reSIDfp::WaveformGenerator generator;
    generator.reset();

    generator.shift_register = 0x35555f;
    generator.set_noise_output();

    REQUIRE(generator.noise_output == 3616);
}

TEST_CASE("Test Write Shift Register", "[waveform-generator]")
{
    reSIDfp::WaveformGenerator generator;
    generator.reset();

    generator.waveform_output = 0x5a7;
    generator.write_shift_register();

    REQUIRE(generator.noise_output == 0xfe0);
}

TEST_CASE("Test Set Test Bit", "[waveform-generator]")
{
    matrix_t* tables = reSIDfp::WaveformCalculator::getInstance()->buildTable(reSIDfp::MOS6581);

    reSIDfp::WaveformGenerator generator;
    generator.reset();
    generator.shift_register = 0x35555e;
    generator.setWaveformModels(tables);

    generator.writeCONTROL_REG(0x08); // set test bit
    generator.writeCONTROL_REG(0x00); // unset test bit

    REQUIRE(generator.noise_output == 2544);
}

TEST_CASE("Test Noise Write Back1", "[waveform-generator]")
{
    matrix_t* tables = reSIDfp::WaveformCalculator::getInstance()->buildTable(reSIDfp::MOS6581);

    reSIDfp::WaveformGenerator modulator;

    reSIDfp::WaveformGenerator generator;
    generator.setWaveformModels(tables);
    generator.reset();

    generator.writeCONTROL_REG(0x88);
    generator.clock();
    generator.output(&modulator);
    generator.writeCONTROL_REG(0x90);
    generator.clock();
    generator.output(&modulator);


    generator.writeCONTROL_REG(0x88);
    generator.clock();
    generator.output(&modulator);
    generator.writeCONTROL_REG(0x80);
    generator.clock();
    generator.output(&modulator);
    REQUIRE(int(generator.readOSC()) == 0xfc);
    generator.writeCONTROL_REG(0x88);
    generator.clock();
    generator.output(&modulator);
    generator.writeCONTROL_REG(0x80);
    generator.clock();
    generator.output(&modulator);
    REQUIRE(int(generator.readOSC()) == 0x6c);
    generator.writeCONTROL_REG(0x88);
    generator.clock();
    generator.output(&modulator);
    generator.writeCONTROL_REG(0x80);
    generator.clock();
    generator.output(&modulator);
    REQUIRE(int(generator.readOSC()) == 0xd8);
    generator.writeCONTROL_REG(0x88);
    generator.clock();
    generator.output(&modulator);
    generator.writeCONTROL_REG(0x80);
    generator.clock();
    generator.output(&modulator);
    REQUIRE(int(generator.readOSC()) == 0xb1);
    generator.writeCONTROL_REG(0x88);
    generator.clock();
    generator.output(&modulator);
    generator.writeCONTROL_REG(0x80);
    generator.clock();
    generator.output(&modulator);
    REQUIRE(int(generator.readOSC()) == 0xd8);
}
