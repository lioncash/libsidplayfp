/*
 * This file is part of sidplayfp, a SID player.
 *
 * Copyright 2011-2018 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2000-2004 Simon White
 * Copyright 2000 Michael Schwendt
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

#include "auFile.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <new>
#include <vector>

namespace
{
/// Set the lo byte (8 bit) in a word (16 bit)
void endian_16lo8(uint_least16_t &word, uint8_t byte)
{
    word &= 0xff00;
    word |= byte;
}

/// Get the lo byte (8 bit) in a word (16 bit)
uint8_t endian_16lo8(uint_least16_t word)
{
    return static_cast<uint8_t>(word);
}

/// Set the hi byte (8 bit) in a word (16 bit)
void endian_16hi8(uint_least16_t &word, uint8_t byte)
{
    word &= 0x00ff;
    word |= static_cast<uint_least16_t>(byte) << 8;
}

/// Set the hi byte (8 bit) in a word (16 bit)
uint8_t endian_16hi8(uint_least16_t word)
{
    return static_cast<uint8_t>(word >> 8);
}

/// Convert high-byte and low-byte to 16-bit word.
uint_least16_t endian_16(uint8_t hi, uint8_t lo)
{
    uint_least16_t word = 0;
    endian_16lo8(word, lo);
    endian_16hi8(word, hi);
    return word;
}

/// Convert high-byte and low-byte to 16-bit big endian word.
uint_least16_t endian_big16(const uint8_t ptr[2])
{
    return endian_16(ptr[0], ptr[1]);
}

/// Set the hi word (16bit) in a dword (32 bit)
void endian_32hi16(uint_least32_t &dword, uint_least16_t word)
{
    dword &= static_cast<uint_least32_t>(0x0000ffff);
    dword |= static_cast<uint_least32_t>(word) << 16;
}

/// Get the hi word (16bit) in a dword (32 bit)
uint_least16_t endian_32hi16(uint_least32_t dword)
{
    return static_cast<uint_least16_t>(dword >> 16);
}

/// Set the lo byte (8 bit) in a dword (32 bit)
void endian_32lo8(uint_least32_t &dword, uint8_t byte)
{
    dword &= static_cast<uint_least32_t>(0xffffff00);
    dword |= static_cast<uint_least32_t>(byte);
}

/// Get the lo byte (8 bit) in a dword (32 bit)
uint8_t endian_32lo8(uint_least32_t dword)
{
    return static_cast<uint8_t>(dword);
}

/// Set the hi byte (8 bit) in a dword (32 bit)
void endian_32hi8(uint_least32_t &dword, uint8_t byte)
{
    dword &= static_cast<uint_least32_t>(0xffff00ff);
    dword |= static_cast<uint_least32_t>(byte) << 8;
}

/// Get the hi byte (8 bit) in a dword (32 bit)
uint8_t endian_32hi8(uint_least32_t dword)
{
    return static_cast<uint8_t>(dword >> 8);
}

/// Convert high-byte and low-byte to 32-bit word.
uint_least32_t endian_32(uint8_t hihi, uint8_t hilo, uint8_t hi, uint8_t lo)
{
    uint_least32_t dword = 0;
    uint_least16_t word  = 0;
    endian_32lo8(dword, lo);
    endian_32hi8(dword, hi);
    endian_16lo8(word, hilo);
    endian_16hi8(word, hihi);
    endian_32hi16(dword, word);
    return dword;
}

/// Convert high-byte and low-byte to 32-bit big endian word.
uint_least32_t endian_big32(const uint8_t ptr[4])
{
    return endian_32(ptr[0], ptr[1], ptr[2], ptr[3]);
}

// Write a big-endian 32-bit word to four bytes in memory.
void endian_big32(uint8_t ptr[4], uint_least32_t dword)
{
    const uint_least16_t word = endian_32hi16(dword);
    ptr[0] = endian_16hi8(word);
    ptr[1] = endian_16lo8(word);
    ptr[2] = endian_32hi8(dword);
    ptr[3] = endian_32lo8(dword);
}

constexpr auHeader defaultAuHdr =
{
    // ASCII keywords are hexified.
    {0x2e,0x73,0x6e,0x64}, // '.snd'
    {0,0,0,24},            // data offset
    {0,0,0,0},             // data size
    {0,0,0,0},             // encoding
    {0,0,0,0},             // Samplerate
    {0,0,0,0},             // Channels
};
} // Anonymous namespace

auFile::auFile(std::string name) :
    AudioBase("AUFILE"),
    name(std::move(name)),
    auHdr(defaultAuHdr)
{}

auFile::~auFile()
{
    auFile::close();
}

bool auFile::open(AudioConfig &cfg)
{
    precision = cfg.precision;

    const unsigned short bits       = precision;
    const unsigned long  format     = (precision == 16) ? 3 : 6;
    const unsigned long  channels   = cfg.channels;
    const unsigned long  freq       = cfg.frequency;
    const auto blockAlign           = static_cast<unsigned short>((bits >> 3) * channels);
    const unsigned long  bufSize    = freq * blockAlign;
    cfg.bufSize = bufSize;

    if (name.empty())
        return false;

    if (file && !file->fail())
        close();

    byteCount = 0;

    // We need to make a buffer for the user
    try
    {
        _sampleBuffer = new short[bufSize];
    }
    catch (const std::bad_alloc&)
    {
        setError("Unable to allocate memory for sample buffers.");
        return false;
    }

    // Fill in header with parameters and expected file size.
    endian_big32(auHdr.encoding, format);
    endian_big32(auHdr.sampleRate, freq);
    endian_big32(auHdr.channels, channels);

    if (name.compare("-") == 0)
    {
        file = &std::cout;
    }
    else
    {
        file = new std::ofstream(name.c_str(), std::ios::out|std::ios::binary|std::ios::trunc);
    }

    _settings = cfg;
    return true;
}

bool auFile::fail() const
{
    return file->fail() != 0;
}

bool auFile::bad() const
{
    return file->bad() != 0;
}

bool auFile::write()
{
    if (file && !file->fail())
    {
        unsigned long int bytes = _settings.bufSize;
        if (!headerWritten)
        {
            file->write((char*)&auHdr, sizeof(auHeader));
            headerWritten = true;
        }

        if (precision == 16)
        {
            std::vector<uint_least16_t> buffer(_settings.bufSize);
            bytes *= 2;
            for (unsigned long i=0; i<_settings.bufSize; i++)
            {
                uint_least16_t temp = _sampleBuffer[i];
                buffer[i] = endian_big16((uint8_t*)&temp);
            }
            file->write((char*)&buffer.front(), bytes);
        }
        else
        {
            std::vector<float> buffer(_settings.bufSize);
            bytes *= 4;
            // normalize floats
            for (std::size_t i = 0; i < _settings.bufSize; i++)
            {
                float temp = static_cast<float>(_sampleBuffer[i]) / 32768.f;
                buffer[i] = static_cast<float>(endian_big32((uint8_t*)&temp));
            }
            file->write((char*)&buffer.front(), bytes);
        }
        byteCount += bytes;

    }
    return true;
}

void auFile::close()
{
    if (file && !file->fail())
    {
        // update length field in header
        endian_big32(auHdr.dataSize, byteCount);
        if (file != &std::cout)
        {
            file->seekp(0, std::ios::beg);
            file->write((char*)&auHdr, sizeof(auHeader));
            delete file;
        }
        file = nullptr;
        delete[] _sampleBuffer;
    }
}
