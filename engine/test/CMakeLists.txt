add_executable(tests src/test.cpp)

find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    include(FetchContent)

    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2
        GIT_TAG v3.9.1
        SYSTEM
    )
    FetchContent_MakeAvailable(Catch2)
endif()
include(Catch)

target_compile_features(tests PRIVATE cxx_std_26)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PRIVATE alsa_min::alsa_min)

set(ALSA_MIN_TEST_DEVICE "hw:Loopback,0" CACHE STRING "ALSA device the hardware tests open")
catch_discover_tests(tests DISCOVERY_MODE PRE_TEST
    PROPERTIES ENVIRONMENT "ALSA_MIN_DEVICE=${ALSA_MIN_TEST_DEVICE}")
