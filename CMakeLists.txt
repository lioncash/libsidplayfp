cmake_minimum_required(VERSION 3.10)

project(libsidplayfp
    DESCRIPTION "C64 cycle exact stereo sidplayer library."
    VERSION     2.0.1
)

option(LIBRESID_USE_NEW_8580_FILTER "Use new 8580 filter for ReSID" ON)

add_subdirectory(libsidplayfp)
add_subdirectory(STILview)
add_subdirectory(sidplayfp)
