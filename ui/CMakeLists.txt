find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(stompd-ui main.cpp)

qt_add_qml_module(stompd-ui
    URI HelloQt
    QML_FILES Main.qml
)

target_link_libraries(stompd-ui PRIVATE Qt6::Quick)

target_compile_features(stompd-ui PUBLIC cxx_std_26)

install(
    TARGETS stompd-ui
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
