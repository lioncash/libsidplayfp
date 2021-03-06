add_library(libresid
    src/builders/resid-builder/resid/dac.cc
    src/builders/resid-builder/resid/dac.h
    src/builders/resid-builder/resid/envelope.cc
    src/builders/resid-builder/resid/envelope.h
    src/builders/resid-builder/resid/extfilt.cc
    src/builders/resid-builder/resid/extfilt.h
    src/builders/resid-builder/resid/pot.cc
    src/builders/resid-builder/resid/pot.h
    src/builders/resid-builder/resid/resid-config.h
    src/builders/resid-builder/resid/sid.cc
    src/builders/resid-builder/resid/sid.h
    src/builders/resid-builder/resid/spline.h
    src/builders/resid-builder/resid/version.cc
    src/builders/resid-builder/resid/voice.cc
    src/builders/resid-builder/resid/voice.h
    src/builders/resid-builder/resid/wave.cc
    src/builders/resid-builder/resid/wave.h
    src/builders/resid-builder/resid/wave6581__ST.h
    src/builders/resid-builder/resid/wave6581_P_T.h
    src/builders/resid-builder/resid/wave6581_PS_.h
    src/builders/resid-builder/resid/wave6581_PST.h
    src/builders/resid-builder/resid/wave8580__ST.h
    src/builders/resid-builder/resid/wave8580_P_T.h
    src/builders/resid-builder/resid/wave8580_PS_.h
    src/builders/resid-builder/resid/wave8580_PST.h
)
if (LIBRESID_USE_NEW_8580_FILTER)
    target_compile_definitions(libresid PRIVATE -DNEW_8580_FILTER=1)
    target_sources(libresid PRIVATE
        src/builders/resid-builder/resid/filter8580new.cc
        src/builders/resid-builder/resid/filter8580new.h
    )
else()
    target_sources(libresid PRIVATE
        src/builders/resid-builder/resid/filter.cc
        src/builders/resid-builder/resid/filter.h
    )
endif()

if (MSVC)
    target_compile_options(libresid PRIVATE
        /W4

        /permissive-        # Enable standards-conformant behavior.
        /volatile:iso       # Use standards-conformant behavior with regards to volatile.
        /Zc:__cplusplus     # Make the __cplusplus processor macro report the correct standards version.
        /Zc:externConstexpr # Enforce standard-conformant behavior with extern constexpr variables.
        /Zc:throwingNew     # Assume that operator new throws upon failure.
    )
endif()

add_library(libresidfp
    src/builders/residfp-builder/residfp/array.h
    src/builders/residfp-builder/residfp/Dac.cpp
    src/builders/residfp-builder/residfp/Dac.h
    src/builders/residfp-builder/residfp/EnvelopeGenerator.cpp
    src/builders/residfp-builder/residfp/EnvelopeGenerator.h
    src/builders/residfp-builder/residfp/ExternalFilter.cpp
    src/builders/residfp-builder/residfp/ExternalFilter.h
    src/builders/residfp-builder/residfp/Filter.cpp
    src/builders/residfp-builder/residfp/Filter.h
    src/builders/residfp-builder/residfp/Filter6581.cpp
    src/builders/residfp-builder/residfp/Filter6581.h
    src/builders/residfp-builder/residfp/Filter8580.cpp
    src/builders/residfp-builder/residfp/Filter8580.h
    src/builders/residfp-builder/residfp/FilterModelConfig.cpp
    src/builders/residfp-builder/residfp/FilterModelConfig.h
    src/builders/residfp-builder/residfp/FilterModelConfig8580.cpp
    src/builders/residfp-builder/residfp/FilterModelConfig8580.h
    src/builders/residfp-builder/residfp/Integrator.cpp
    src/builders/residfp-builder/residfp/Integrator.h
    src/builders/residfp-builder/residfp/Integrator8580.cpp
    src/builders/residfp-builder/residfp/Integrator8580.h
    src/builders/residfp-builder/residfp/OpAmp.cpp
    src/builders/residfp-builder/residfp/OpAmp.h
    src/builders/residfp-builder/residfp/Potentiometer.h
    src/builders/residfp-builder/residfp/SID.cpp
    src/builders/residfp-builder/residfp/SID.h
    src/builders/residfp-builder/residfp/Spline.cpp
    src/builders/residfp-builder/residfp/Spline.h
    src/builders/residfp-builder/residfp/version.cc
    src/builders/residfp-builder/residfp/Voice.h
    src/builders/residfp-builder/residfp/WaveformCalculator.cpp
    src/builders/residfp-builder/residfp/WaveformCalculator.h
    src/builders/residfp-builder/residfp/WaveformGenerator.cpp
    src/builders/residfp-builder/residfp/WaveformGenerator.h
    src/builders/residfp-builder/residfp/resample/Resampler.h
    src/builders/residfp-builder/residfp/resample/SincResampler.cpp
    src/builders/residfp-builder/residfp/resample/SincResampler.h
    src/builders/residfp-builder/residfp/resample/TwoPassSincResampler.h
    src/builders/residfp-builder/residfp/resample/ZeroOrderResampler.h
)
target_include_directories(libresidfp PRIVATE
    src/builders/residfp-builder/residfp/
)

if (MSVC)
    target_compile_options(libresidfp PRIVATE
        /W4

        /permissive-        # Enable standards-conformant behavior.
        /volatile:iso       # Use standards-conformant behavior with regards to volatile.
        /Zc:__cplusplus     # Make the __cplusplus processor macro report the correct standards version.
        /Zc:externConstexpr # Enforce standard-conformant behavior with extern constexpr variables.
        /Zc:throwingNew     # Assume that operator new throws upon failure.
    )
endif()

add_library(libsidplayfp
    include/sidplayfp/sidbuilder.h
    include/sidplayfp/sidplayfp.h
    include/sidplayfp/SidConfig.h
    include/sidplayfp/SidDatabase.h
    include/sidplayfp/SidInfo.h
    include/sidplayfp/SidTune.h
    include/sidplayfp/SidTuneInfo.h

    src/Event.h
    src/EventCallback.h
    src/EventScheduler.cpp
    src/EventScheduler.h
    src/mixer.cpp
    src/mixer.h
    src/player.cpp
    src/player.h
    src/psiddrv.cpp
    src/psiddrv.h
    src/reloc65.cpp
    src/reloc65.h
    src/romCheck.h
    src/sidemu.cpp
    src/sidemu.h
    src/sidendian.h
    src/SidInfoImpl.h
    src/sidmd5.h
    src/sidmemory.h
    src/sidrandom.h
    src/stringutils.h

    src/c64/c64.cpp
    src/c64/c64.h
    src/c64/c64cia.h
    src/c64/c64cpu.h
    src/c64/c64env.h
    src/c64/c64sid.h
    src/c64/c64vic.h
    src/c64/mmu.cpp
    src/c64/mmu.h
    src/c64/Banks/Bank.h
    src/c64/Banks/ColorRAMBank.h
    src/c64/Banks/DisconnectedBusBank.h
    src/c64/Banks/ExtraSidBank.h
    src/c64/Banks/IOBank.h
    src/c64/Banks/NullSid.h
    src/c64/Banks/SidBank.h
    src/c64/Banks/SystemRAMBank.h
    src/c64/Banks/SystemROMBanks.h
    src/c64/Banks/ZeroRAMBank.h
    src/c64/CIA/interrupt.h
    src/c64/CIA/mos6526.cpp
    src/c64/CIA/mos6526.h
    src/c64/CIA/SerialPort.cpp
    src/c64/CIA/SerialPort.h
    src/c64/CIA/timer.cpp
    src/c64/CIA/timer.h
    src/c64/CIA/tod.cpp
    src/c64/CIA/tod.h
    src/c64/CPU/flags.h
    src/c64/CPU/mos6510.cpp
    src/c64/CPU/mos6510.h
    src/c64/CPU/mos6510debug.cpp
    src/c64/CPU/mos6510debug.h
    src/c64/CPU/opcodes.h
    src/c64/VIC_II/lightpen.h
    src/c64/VIC_II/mos656x.cpp
    src/c64/VIC_II/mos656x.h
    src/c64/VIC_II/sprites.h

    src/sidplayfp/sidbuilder.cpp
    src/sidplayfp/sidplayfp.cpp
    src/sidplayfp/SidConfig.cpp
    src/sidplayfp/SidInfo.cpp
    src/sidplayfp/SidTune.cpp
    src/sidplayfp/SidTuneInfo.cpp

    src/sidtune/MUS.cpp
    src/sidtune/MUS.h
    src/sidtune/p00.cpp
    src/sidtune/p00.h
    src/sidtune/prg.cpp
    src/sidtune/prg.h
    src/sidtune/PSID.cpp
    src/sidtune/PSID.h
    src/sidtune/SidTuneBase.cpp
    src/sidtune/SidTuneBase.h
    src/sidtune/SidTuneCfg.h
    src/sidtune/SidTuneInfoImpl.h
    src/sidtune/SidTuneTools.cpp
    src/sidtune/SidTuneTools.h
    src/sidtune/SmartPtr.h

    src/utils/iMd5.h
    src/utils/iniParser.cpp
    src/utils/iniParser.h
    src/utils/md5Factory.cpp
    src/utils/md5Factory.h
    src/utils/md5Internal.h
    src/utils/SidDatabase.cpp
    src/utils/MD5/MD5.cpp
    src/utils/MD5/MD5.h
    src/utils/MD5/MD5_Defs.h
)
target_compile_definitions(libsidplayfp
PUBLIC
    -DLIBSIDPLAYFP_VERSION_MAJ=${PROJECT_VERSION_MAJOR}
    -DLIBSIDPLAYFP_VERSION_MIN=${PROJECT_VERSION_MINOR}
    -DLIBSIDPLAYFP_VERSION_LEV=${PROJECT_VERSION_PATCH}

PRIVATE
    -DPACKAGE_NAME="${PROJECT_NAME}"
    -DPACKAGE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
    -DPACKAGE_VERSION="${PROJECT_VERSION}"
)
target_include_directories(libsidplayfp
PUBLIC
    include/

PRIVATE
    src/
    src/c64/
    src/sidplayfp/
    src/sidtune/
    src/utils/
)


add_library(resid-builder
    include/sidplayfp/builders/resid.h
    src/builders/resid-builder/resid-builder.cpp
    src/builders/resid-builder/resid-emu.cpp
    src/builders/resid-builder/resid-emu.h
)
target_compile_definitions(resid-builder
PRIVATE
    -DVERSION="${PROJECT_VERSION}"
)
target_include_directories(resid-builder
PRIVATE
    src/
    src/c64/
    src/sidplayfp/
    src/sidtune/
    src/utils/
)
target_link_libraries(resid-builder
PRIVATE
    libresid
    libsidplayfp
)

add_library(residfp-builder
    include/sidplayfp/builders/residfp.h
    src/builders/residfp-builder/residfp-builder.cpp
    src/builders/residfp-builder/residfp-emu.cpp
    src/builders/residfp-builder/residfp-emu.h
)
target_compile_definitions(residfp-builder
PRIVATE
    -DVERSION="${PROJECT_VERSION}"
)
target_include_directories(residfp-builder
PRIVATE
    src/
    src/c64/
    src/sidplayfp/
    src/sidtune/
    src/utils/
)
target_link_libraries(residfp-builder
PRIVATE
    libresidfp
    libsidplayfp
)

#
# Tests
#
add_subdirectory(test)
add_subdirectory(tests)
