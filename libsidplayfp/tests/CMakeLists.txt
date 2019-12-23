add_library(catch INTERFACE)

target_include_directories(catch
INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/catch>
)

add_executable(tests
    Main.cpp
    TestDac.cpp
    TestEnvelopeGenerator.cpp
    TestMUS.cpp
    TestPSID.cpp
    TestSpline.cpp
    TestWaveformGenerator.cpp
)
target_link_libraries(tests
PRIVATE
    catch
    libresidfp
    libsidplayfp
)
