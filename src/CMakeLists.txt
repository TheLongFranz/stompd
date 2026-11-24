cmake_minimum_required(VERSION 3.28.1)

project(hello-stompd LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(${PROJECT_NAME} main.cpp)

qt_add_qml_module(hello-stompd
    URI HelloQt
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Quick)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})