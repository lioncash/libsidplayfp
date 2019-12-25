/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2019 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
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

#include <sidplayfp/SidConfig.h>

#include "mixer.h"

SidConfig::SidConfig() :
    leftVolume(libsidplayfp::Mixer::VOLUME_MAX),
    rightVolume(libsidplayfp::Mixer::VOLUME_MAX)
{}

bool SidConfig::compare(const SidConfig &config) const
{
    return defaultC64Model != config.defaultC64Model
        || forceC64Model != config.forceC64Model
        || defaultSidModel != config.defaultSidModel
        || forceSidModel != config.forceSidModel
        || digiBoost != config.digiBoost
        || ciaModel != config.ciaModel
        || playback != config.playback
        || frequency != config.frequency
        || secondSidAddress != config.secondSidAddress
        || thirdSidAddress != config.thirdSidAddress
        || sidEmulation != config.sidEmulation
        || leftVolume != config.leftVolume
        || rightVolume != config.rightVolume
        || powerOnDelay != config.powerOnDelay
        || samplingMethod != config.samplingMethod
        || fastSampling != config.fastSampling;
}
