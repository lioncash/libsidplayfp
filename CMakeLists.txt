cmake_minimum_required(VERSION 3.10)

project(libsidplayfp
    DESCRIPTION "C64 cycle exact stereo sidplayer library."
    VERSION     2.0.1
)

option(LIBRESID_USE_NEW_8580_FILTER "Use new 8580 filter for ReSID" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_subdirectory(libsidplayfp)
add_subdirectory(STILview)
add_subdirectory(sidplayfp)
