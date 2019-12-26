add_library(libstilview
    stil.cpp
    stringutils.h
    include/stilview/stil.h
    include/stilview/stildefs.h
)

target_include_directories(libstilview
PUBLIC
    include/
)

if (MSVC)
    target_compile_options(libstilview PRIVATE
        /W4

        /permissive-        # Enable standards-conformant behavior.
        /volatile:iso       # Use standards-conformant behavior with regards to volatile.
        /Zc:__cplusplus     # Make the __cplusplus processor macro report the correct standards version.
        /Zc:externConstexpr # Enforce standard-conformant behavior with extern constexpr variables.
        /Zc:throwingNew     # Assume that operator new throws upon failure.
    )
endif()
