# TODO: Handle DirectX, OSS, Out123

add_executable(sidplayfp
    src/args.cpp
    src/IniConfig.cpp
    src/IniConfig.h
    src/keyboard.cpp
    src/keyboard.h
    src/main.cpp
    src/menu.cpp
    src/player.cpp
    src/player.h
    src/utils.cpp
    src/utils.h

    src/audio/AudioBase.h
    src/audio/AudioConfig.h
    src/audio/AudioDrv.cpp
    src/audio/AudioDrv.h
    src/audio/IAudio.h
    src/audio/au/auFile.cpp
    src/audio/au/auFile.h
    src/audio/null/null.cpp
    src/audio/null/null.h
    src/audio/wav/WavFile.cpp
    src/audio/wav/WavFile.h

    src/ini/dataParser.h
    src/ini/iniHandler.cpp
    src/ini/iniHandler.h
    src/ini/sidfstream.h
    src/ini/types.h
)

# TODO: Handle exSID and hardSID
target_compile_definitions(sidplayfp
PRIVATE
    # Assume we always have regular ReSID instances.
    HAVE_SIDPLAYFP_BUILDERS_RESID_H=1
    HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H=1

    PACKAGE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}"
    VERSION="${PROJECT_VERSION}"
)

target_link_libraries(sidplayfp
PRIVATE
    libsidplayfp
    resid-builder
    residfp-builder
)

if (UNIX)
    find_package(ALSA)
    if (ALSA_FOUND)
        target_link_libraries(sidplayfp PRIVATE ALSA::ALSA)
        target_sources(sidplayfp PRIVATE
            src/audio/alsa/audiodrv.cpp
            src/audio/alsa/audiodrv.h
        )
    endif()
endif()

if (WIN32)
    target_compile_definitions(sidplayfp PRIVATE HAVE_MMSYSTEM_H=1)
    target_link_libraries(sidplayfp PRIVATE
        shlwapi.lib
        winmm.lib
    )
    target_sources(sidplayfp PRIVATE
        src/audio/mmsystem/audiodrv.cpp
        src/audio/mmsystem/audiodrv.h
    )
endif()

add_executable(stilview
    src/stilview.cpp
)
target_link_libraries(stilview
PRIVATE
    libstilview
)