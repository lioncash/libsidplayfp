/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2016 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2000 Simon White
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

#include "mixer.h"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "sidemu.h"

namespace libsidplayfp
{
namespace
{
class BufferPos
{
public:
    explicit BufferPos(int position) : pos(position) {}
    void operator()(sidemu *s) const { s->bufferpos(pos); }

private:
    int pos;
};

class BufferMove
{
public:
    explicit BufferMove(int position, int numSamples) : pos(position), samples(numSamples) {}
    void operator()(short *dest) const
    {
        const short* const src = dest + pos;
        std::copy(src, src + samples, dest);
    }

private:
    int pos;
    int samples;
};
} // Anonymous namespace

void Mixer::clockChips()
{
    for (sidemu* const chip : m_chips)
    {
        chip->clock();
    }
}

void Mixer::resetBufs()
{
    std::for_each(m_chips.begin(), m_chips.end(), BufferPos(0));
}

void Mixer::doMix()
{
    short *buf = m_sampleBuffer + m_sampleIndex;

    // extract buffer info now that the SID is updated.
    // clock() may update bufferpos.
    // NB: if more than one chip exists, their bufferpos is identical to first chip's.
    const int sampleCount = m_chips.front()->bufferpos();

    int i = 0;
    while (i < sampleCount)
    {
        // Handle whatever output the sid has generated so far
        if (m_sampleIndex >= m_sampleCount)
        {
            break;
        }
        // Are there enough samples to generate the next one?
        if (i + m_fastForwardFactor >= sampleCount)
        {
            break;
        }

        // This is a crude boxcar low-pass filter to
        // reduce aliasing during fast forward.
        for (size_t k = 0; k < m_buffers.size(); k++)
        {
            const short* const buffer = m_buffers[k] + i;
            const std::int32_t sample = std::accumulate(buffer, buffer + m_fastForwardFactor, 0);

            m_iSamples[k] = sample / m_fastForwardFactor;
        }

        // increment i to mark we ate some samples, finish the boxcar thing.
        i += m_fastForwardFactor;

        const int dither = triangularDithering();

        const unsigned int channels = m_stereo ? 2 : 1;
        for (unsigned int ch = 0; ch < channels; ch++)
        {
            const int_least32_t tmp = ((this->*(m_mix[ch]))() * m_volume[ch] + dither) / VOLUME_MAX;
            assert(tmp >= -32768 && tmp <= 32767);
            *buf++ = static_cast<short>(tmp);
            m_sampleIndex++;
        }
    }

    // move the unhandled data to start of buffer, if any.
    const int samplesLeft = sampleCount - i;
    std::for_each(m_buffers.begin(), m_buffers.end(), BufferMove(i, samplesLeft));
    std::for_each(m_chips.begin(), m_chips.end(), BufferPos(samplesLeft));
}

void Mixer::begin(short *buffer, std::size_t count)
{
    m_sampleIndex  = 0;
    m_sampleCount  = count;
    m_sampleBuffer = buffer;
}

void Mixer::updateParams()
{
    switch (m_buffers.size())
    {
    case 1:
        m_mix[0] = m_stereo ? &Mixer::stereo_OneChip : &Mixer::mono<1>;
        if (m_stereo) m_mix[1] = &Mixer::stereo_OneChip;
        break;
    case 2:
        m_mix[0] = m_stereo ? &Mixer::stereo_ch1_TwoChips : &Mixer::mono<2>;
        if (m_stereo) m_mix[1] = &Mixer::stereo_ch2_TwoChips;
        break;
    case 3:
        m_mix[0] = m_stereo ? &Mixer::stereo_ch1_ThreeChips : &Mixer::mono<3>;
        if (m_stereo) m_mix[1] = &Mixer::stereo_ch2_ThreeChips;
        break;
     }
}

void Mixer::clearSids()
{
    m_chips.clear();
    m_buffers.clear();
}

void Mixer::addSid(sidemu *chip)
{
    if (chip == nullptr)
    {
        return;
    }

    m_chips.push_back(chip);
    m_buffers.push_back(chip->buffer());
    m_iSamples.resize(m_buffers.size());

    if (m_mix.empty())
    {
        return;
    }

    updateParams();
}

void Mixer::setStereo(bool stereo)
{
    if (m_stereo == stereo)
    {
        return;
    }

    m_stereo = stereo;
    m_mix.resize(m_stereo ? 2 : 1);
    updateParams();
}

bool Mixer::setFastForward(int ff)
{
    if (ff < 1 || ff > 32)
        return false;

    m_fastForwardFactor = ff;
    return true;
}

void Mixer::setVolume(int_least32_t left, int_least32_t right)
{
    m_volume.clear();
    m_volume.push_back(left);
    m_volume.push_back(right);
}

}
