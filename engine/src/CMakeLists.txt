add_executable(stompd-engine main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE alsa_min::alsa_min)
