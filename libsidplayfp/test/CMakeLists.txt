add_executable(test-demo
    demo.cpp
)
target_link_libraries(test-demo
PRIVATE
    libsidplayfp
)

add_executable(test-test
    test.cpp
)
target_link_libraries(test-test
PRIVATE
    libsidplayfp
)