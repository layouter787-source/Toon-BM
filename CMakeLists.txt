cmake_minimum_required(VERSION 3.21)
project(ToonBM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(ToonBM
    src/main.cpp
    src/core/Drawing.h
    src/core/Project.h
    src/core/Project.cpp
    src/ui/CanvasItem.h
    src/ui/CanvasItem.cpp
    src/script/ScriptRunner.h
    src/script/ScriptRunner.cpp
)

qt_add_qml_module(ToonBM
    URI ToonBM
    VERSION 1.0
    QML_FILES qml/Main.qml
)

target_include_directories(ToonBM PRIVATE src)
target_link_libraries(ToonBM PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)

set_target_properties(ToonBM PROPERTIES
    QT_ANDROID_PACKAGE_NAME "com.layouter787.toonbm"
    QT_ANDROID_VERSION_NAME "0.1.0"
    QT_ANDROID_VERSION_CODE 1
)
