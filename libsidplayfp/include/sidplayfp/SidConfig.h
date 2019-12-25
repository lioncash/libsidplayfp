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

#ifndef SIDCONFIG_H
#define SIDCONFIG_H

#include <cstdint>
#include <sidplayfp/siddefs.h>

class sidbuilder;

/**
 * SidConfig
 *
 * An instance of this class is used to transport emulator settings
 * to and from the interface class.
 */
class SID_EXTERN SidConfig
{
public:
    /// Playback mode
    enum class PlaybackMode
    {
        Mono = 1,
        Stereo
    };

    /// SID chip model
    enum class SIDModel
    {
        MOS6581,
        MOS8580
    };

    /// CIA chip model
    enum class CIAModel
    {
        MOS6526,
        MOS8521
    };

    /// C64 model
    enum class C64Model
    {
        PAL,
        NTSC,
        OldNTSC,
        DREAN,
        PAL_M
    };

    /// Sampling method
    enum class SamplingMethod
    {
        Interpolate,
        ResampleInterpolate
    };

    SidConfig();

    /**
     * Compare two config objects.
     *
     * @return true if different
     */
    bool compare(const SidConfig& config) const;

    /**
     * Maximum power on delay.
     * - Delays <= MAX produce constant results
     * - Delays >  MAX produce random results
     */
    static constexpr uint_least16_t MAX_POWER_ON_DELAY = 0x1FFF;
    static constexpr uint_least16_t DEFAULT_POWER_ON_DELAY = MAX_POWER_ON_DELAY + 1;

    static constexpr uint_least32_t DEFAULT_SAMPLING_FREQ  = 44100;

    /**
     * Intended c64 model when unknown or forced.
     * - PAL
     * - NTSC
     * - OldNTSC
     * - DREAN
     * - PAL_M
     */
    C64Model defaultC64Model = C64Model::PAL;

    /**
     * Force the model to #defaultC64Model ignoring tune's clock setting.
     */
    bool forceC64Model = false;

    /**
     * Intended sid model when unknown or forced.
     * - MOS6581
     * - MOS8580
     */
    SIDModel defaultSidModel = SIDModel::MOS6581;

    /**
     * Force the sid model to #defaultSidModel.
     */
    bool forceSidModel = false;

    /**
     * Enable digiboost when 8580 SID model is used.
     */
    bool digiBoost = false;

    /**
     * Intended cia model.
     * - MOS6526
     * - MOS8521
     */
    CIAModel ciaModel = CIAModel::MOS6526;

    /**
     * Playbak mode.
     * - MONO
     * - STEREO
     */
    PlaybackMode playback = PlaybackMode::Mono;

    /**
     * Sampling frequency.
     */
    uint_least32_t frequency = DEFAULT_SAMPLING_FREQ;

    /**
     * Extra SID chips addresses.
     */
    //@{
    uint_least16_t secondSidAddress = 0;
    uint_least16_t thirdSidAddress = 0;
    //@}

    /**
     * Pointer to selected emulation,
     * reSIDfp, reSID or hardSID.
     */
    sidbuilder *sidEmulation = nullptr;

    /**
     * Left channel volume.
     */
    uint_least32_t leftVolume;

    /**
     * Right channel volume.
     */
    uint_least32_t rightVolume;

    /**
     * Power on delay cycles.
     */
    uint_least16_t powerOnDelay = DEFAULT_POWER_ON_DELAY;

    /**
     * Sampling method.
     * - Interpolate
     * - ResampleInterpolate
     */
    SamplingMethod samplingMethod = SamplingMethod::ResampleInterpolate;

    /**
     * Faster low-quality emulation,
     * available only for reSID.
     */
    bool fastSampling = false;
};

#endif // SIDCONFIG_H
