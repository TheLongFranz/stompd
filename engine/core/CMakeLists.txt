add_library(alsa_min_alsa_min)
add_library(alsa_min::alsa_min ALIAS alsa_min_alsa_min)

target_sources(alsa_min_alsa_min
    PUBLIC FILE_SET CXX_MODULES
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include/
    FILES
    alsa_min/audio_device.cppm
    alsa_min/audio_processor.cppm)

set_property(TARGET alsa_min_alsa_min PROPERTY EXPORT_NAME ${PROJECT_NAME})

target_compile_features(alsa_min_alsa_min PUBLIC cxx_std_26)

find_package(ALSA REQUIRED)
target_link_libraries(alsa_min_alsa_min PUBLIC ALSA::ALSA)
